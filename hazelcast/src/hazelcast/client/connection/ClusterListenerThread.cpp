/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 5/23/13.

#include <boost/foreach.hpp>
#include "hazelcast/client/aws/AWSClient.h"
#include "hazelcast/client/ClientProperties.h"

#include "hazelcast/util/Util.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/ClientConfig.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ClusterListenerThread::ClusterListenerThread(spi::ClientContext &clientContext)
                    : startLatch(1), clientContext(clientContext), deletingConnection(false) {
                listenerThread.reset(
                        new util::Thread(boost::shared_ptr<util::Runnable>(new util::RunnableDelegator(*this))));
            }

            const std::string ClusterListenerThread::getName() const {
                return "ClusterListenerThread";
            }

            void ClusterListenerThread::run() {
                Address previousConnectionAddr;
                Address *previousConnectionAddrPtr = NULL;
                spi::LifecycleService &lifecycleService = clientContext.getLifecycleService();
                while (lifecycleService.isRunning()) {
                    try {
                        if (conn.get() == NULL || !conn->live) {
                            try {
                                conn = clientContext.getClusterService().connectToOne(previousConnectionAddrPtr);
                                previousConnectionAddr = conn->getRemoteEndpoint();
                                previousConnectionAddrPtr = &previousConnectionAddr;
                            } catch (std::exception &e) {
                                if (lifecycleService.isRunning()) {
                                    util::ILogger::getLogger().severe(
                                            std::string("Error while connecting to cluster! =>") + e.what());
                                    lifecycleService.shutdown();
                                }
                                /**
                                 * Do nothing except returning here. Since client and cluster service may have been
                                 * already been destructed, any access to startLatch or any other client variable
                                 * may cause invalid memory access.
                                 */
                                return;
                            }
                        }

                        isInitialMembersLoaded = false;
                        isRegistrationIdReceived = false;
                        loadInitialMemberList();
                        clientContext.getServerListenerService().triggerFailedListeners();
                        startLatch.countDown();
                        listenMembershipEvents();
                        listenerThread->interruptibleSleep(1);
                    } catch (std::exception &e) {
                        if (lifecycleService.isRunning()) {
                            util::ILogger::getLogger().warning(
                                    std::string("Error while listening cluster events! -> ") + e.what());
                        }

                        clientContext.getConnectionManager().onCloseOwnerConnection();
                        if (deletingConnection.compareAndSet(false, true)) {
                            boost::shared_ptr<Connection> connection = conn;
                            if (connection.get()) {
                                util::IOUtil::closeResource(connection.get(), "Error while listening cluster events");
                                lifecycleService.fireLifecycleEvent(LifecycleEvent::CLIENT_DISCONNECTED);
                            }
                            deletingConnection = false;
                        }
                        if (clientContext.getLifecycleService().isRunning()) {
                            listenerThread->interruptibleSleep(1);
                        }
                    }
                }
            }

            void ClusterListenerThread::stop() {
                startLatch.countDown();
                if (deletingConnection.compareAndSet(false, true)) {
                    if (conn.get()) {
                        util::IOUtil::closeResource(conn.get(), "Cluster listener thread is stopping");
                        clientContext.getLifecycleService().fireLifecycleEvent(LifecycleEvent::CLIENT_DISCONNECTED);
                    }
                    deletingConnection = false;
                }
            }

            std::set<Address, addressComparator> ClusterListenerThread::getSocketAddresses() const {
                std::set<Address, addressComparator> addresses;
                if (!members.empty()) {
                    std::vector<Address> clusterAddresses = getClusterAddresses();
                    addresses.insert(clusterAddresses.begin(), clusterAddresses.end());
                }
                std::vector<Address> configAddresses = getConfigAddresses();
                addresses.insert(configAddresses.begin(), configAddresses.end());
                std::vector<Address> awsAddresses = getAwsAddresses();
                addresses.insert(awsAddresses.begin(), awsAddresses.end());

                if (addresses.empty()) {
                    addresses.insert(Address("127.0.0.1", 5701));
                }

                return addresses;
            }

            void ClusterListenerThread::loadInitialMemberList() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ClientAddMembershipListenerCodec::RequestParameters::encode(false);

                request->setCorrelationId(clientContext.getConnectionManager().getNextCallId());

                conn->writeBlocking(*request);

                std::auto_ptr<protocol::ClientMessage> response;

                do {
                    response = conn->readBlocking();
                    if (protocol::codec::ClientAddMembershipListenerCodec::ResponseParameters::TYPE ==
                        response->getMessageType()) {
                        protocol::codec::ClientAddMembershipListenerCodec::ResponseParameters result = protocol::codec::ClientAddMembershipListenerCodec::ResponseParameters::decode(
                                *response);
                        registrationId = result.response;
                        isRegistrationIdReceived = true;
                    } else {
                        handle(response);
                    }
                } while (!isInitialMembersLoaded || !isRegistrationIdReceived);
            }

            void ClusterListenerThread::listenMembershipEvents() {
                while (clientContext.getLifecycleService().isRunning()) {
                    std::auto_ptr<protocol::ClientMessage> clientMessage = conn->readBlocking();
                    if (!clientContext.getLifecycleService().isRunning()) {
                        break;
                    }

                    handle(clientMessage);
                }
            }

            void ClusterListenerThread::updateMembersRef() {
                std::auto_ptr<std::map<Address, Member, addressComparator> > addrMap(
                        new std::map<Address, Member, addressComparator>());
                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    (*addrMap)[it->getAddress()] = *it;
                }
                clientContext.getClusterService().setMembers(addrMap);
            }

            std::vector<Address> ClusterListenerThread::getClusterAddresses() const {
                std::vector<Address> socketAddresses;
                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    socketAddresses.push_back(it->getAddress());
                }
                return socketAddresses;
            }

            std::vector<Address>  ClusterListenerThread::getConfigAddresses() const {
                std::vector<Address> socketAddresses;
                std::set<Address, addressComparator> &configAddresses = clientContext.getClientConfig().getAddresses();
                std::set<Address, addressComparator>::iterator it;

                for (it = configAddresses.begin(); it != configAddresses.end(); ++it) {
                    socketAddresses.push_back((*it));
                }

                return socketAddresses;
            }

            std::vector<Address> ClusterListenerThread::getAwsAddresses() const {
                std::vector<Address> awsAdresses;
                config::ClientAwsConfig &awsConfig = clientContext.getClientConfig().getNetworkConfig().getAwsConfig();
                if (awsConfig.isEnabled()) {
                    try {
                        aws::AWSClient awsClient(awsConfig);
                        typedef std::map<std::string, std::string> AddressMap;
                        BOOST_FOREACH(const AddressMap::value_type &addressPair , awsClient.getAddresses()) {
                            awsAdresses.push_back(Address(addressPair.first, awsMemberPort));
                        }
                    } catch (exception::IException &e) {
                        util::ILogger::getLogger().warning(std::string("Aws addresses failed to load: ") + e.what());
                    }
                }

                return awsAdresses;
            }

            void ClusterListenerThread::handleMember(const Member &member, const int32_t &eventType) {
                switch (eventType) {
                    case MembershipEvent::MEMBER_ADDED:
                        memberAdded(member);
                        break;
                    case MembershipEvent::MEMBER_REMOVED:
                        memberRemoved(member);
                        break;
                    default:
                        char buf[50];
                        util::hz_snprintf(buf, 50, "Unknown event type :%d", eventType);
                        util::ILogger::getLogger().warning(buf);
                }
                clientContext.getPartitionService().wakeup();
            }

            void ClusterListenerThread::handleMemberList(const std::vector<Member> &initialMembers) {
                std::auto_ptr<std::map<std::string, Member> > prevMembers;
                if (members.size() > 0) {
                    prevMembers = std::auto_ptr<std::map<std::string, Member> >(new std::map<std::string, Member>);
                    for (std::vector<Member>::const_iterator member = members.begin();
                         member != members.end(); ++member) {
                        (*prevMembers)[member->getUuid()] = *member;
                    }
                    members.clear();
                }

                for (std::vector<Member>::const_iterator initialMember = initialMembers.begin();
                     initialMember != initialMembers.end(); ++initialMember) {
                    members.push_back(*initialMember);
                }

                std::vector<MembershipEvent> events = detectMembershipEvents(prevMembers);
                if (events.size() != 0) {
                    applyMemberListChanges();
                }
                fireMembershipEvents(events);

                isInitialMembersLoaded = true;
            }

            void ClusterListenerThread::handleMemberAttributeChange(const std::string &uuid, const std::string &key,
                                                                    const int32_t &operationType,
                                                                    std::auto_ptr<std::string> value) {
                // find and update the member in local list
                std::auto_ptr<std::map<Address, Member, addressComparator> > addrMap(new std::map<Address, Member, addressComparator>());
                std::vector<Member>::const_iterator foundMember = members.end();
                MemberAttributeEvent::MemberAttributeOperationType type = (MemberAttributeEvent::MemberAttributeOperationType) operationType;
                for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                    if (it->getUuid() == uuid) {
                        switch (operationType) {
                            case MemberAttributeEvent::PUT:
                                it->setAttribute(key, *value);
                                break;
                            case MemberAttributeEvent::REMOVE:
                                it->removeAttribute(key);
                                break;
                            default:
                                char buf[50];
                                util::hz_snprintf(buf, 50, "Not a known OperationType: %d", operationType);
                                throw exception::IllegalArgumentException("Member::updateAttribute", buf);
                        }
                        foundMember = it;
                    }
                    (*addrMap)[it->getAddress()] = *it;
                }

                if (members.end() != foundMember) {
                    clientContext.getClusterService().setMembers(addrMap);

                    // fire event
                    MemberAttributeEvent event(clientContext.getCluster(), *foundMember, type, key, *value, members);

                    clientContext.getClusterService().fireMemberAttributeEvent(event);
                }
            }

            void ClusterListenerThread::memberAdded(const Member &member) {
                members.push_back(member);

                applyMemberListChanges();

                MembershipEvent event(clientContext.getCluster(), member, MembershipEvent::MEMBER_ADDED, members);

                clientContext.getClusterService().fireMembershipEvent(event);
            }

            void ClusterListenerThread::memberRemoved(const Member &member) {
                for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                    if (member == *it) {
                        members.erase(it);
                        break;
                    }
                }

                applyMemberListChanges();

                clientContext.getConnectionManager().removeEndpoint(member.getAddress());

                MembershipEvent event(clientContext.getCluster(), member, MembershipEvent::MEMBER_REMOVED,
                                      members);

                clientContext.getClusterService().fireMembershipEvent(event);
            }

            std::vector<MembershipEvent> ClusterListenerThread::detectMembershipEvents(
                    std::auto_ptr<std::map<std::string, Member> > prevMembers) const {
                std::vector<MembershipEvent> events;
                std::vector<Member> eventMembers(members);
                if (NULL != prevMembers.get()) {
                    for (std::vector<Member>::const_iterator member = members.begin();
                         member != members.end(); ++member) {
                        std::map<std::string, Member>::iterator former = prevMembers->find(member->getUuid());
                        if (former == prevMembers->end()) {
                            events.push_back(
                                    MembershipEvent(clientContext.getCluster(), *member, MembershipEvent::MEMBER_ADDED,
                                                    eventMembers));
                        } else {
                            prevMembers->erase(former);
                        }
                    }
                    for (std::map<std::string, Member>::const_iterator it = prevMembers->begin();
                         it != prevMembers->end(); ++it) {
                        MembershipEvent event(clientContext.getCluster(), it->second, MembershipEvent::MEMBER_REMOVED,
                                              eventMembers);
                        events.push_back(event);
                        const Address &address = it->second.getAddress();

                        if (!clientContext.getClusterService().isMemberExists(address)) {
                            clientContext.getConnectionManager().removeEndpoint(address);
                        }
                    }
                } else {
                    for (std::vector<Member>::const_iterator member = members.begin();
                         member != members.end(); ++member) {
                        events.push_back(
                                MembershipEvent(clientContext.getCluster(), *member, MembershipEvent::MEMBER_ADDED,
                                                eventMembers));
                    }
                }
                return events;
            }

            void ClusterListenerThread::applyMemberListChanges() {
                updateMembersRef();

                util::ILogger::getLogger().info(clientContext.getClusterService().membersString());
            }

            void ClusterListenerThread::fireMembershipEvents(const std::vector<MembershipEvent> &events) const {
                for (std::vector<MembershipEvent>::const_iterator it = events.begin(); it != events.end(); ++it) {
                    clientContext.getClusterService().fireMembershipEvent(*it);
                }
            }

            bool ClusterListenerThread::awaitStart() {
                startLatch.await();
                return !clientContext.getClusterService().getMemberList().empty();
            }

            void ClusterListenerThread::setAwsMemberPort(int awsMemberPort) {
                ClusterListenerThread::awsMemberPort = awsMemberPort;
            }

            bool ClusterListenerThread::start() {
                listenerThread->start();
                return awaitStart();
            }

        }
    }
}

