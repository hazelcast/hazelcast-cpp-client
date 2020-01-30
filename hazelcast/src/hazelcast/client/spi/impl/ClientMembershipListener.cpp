/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include <hazelcast/client/MemberAttributeEvent.h>
#include "hazelcast/client/spi/impl/ClientMembershipListener.h"
#include "hazelcast/client/MembershipEvent.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                int ClientMembershipListener::INITIAL_MEMBERS_TIMEOUT_SECONDS = 5;

                ClientMembershipListener::ClientMembershipListener(ClientContext &client)
                        : client(client), logger(client.getLogger()),
                          clusterService(static_cast<ClientClusterServiceImpl &>(client.getClientClusterService())),
                          partitionService((ClientPartitionServiceImpl &) client.getPartitionService()),
                          connectionManager(client.getConnectionManager()) {}

                void ClientMembershipListener::handleMemberEventV10(const Member &member, const int32_t &eventType) {
                    switch (eventType) {
                        case MembershipEvent::MEMBER_ADDED:
                            memberAdded(member);
                            break;
                        case MembershipEvent::MEMBER_REMOVED:
                            memberRemoved(member);
                            break;
                        default:
                            logger.warning() << "Unknown event type: " << eventType;
                    }
                    partitionService.refreshPartitions();
                }

                void ClientMembershipListener::handleMemberListEventV10(const std::vector<Member> &initialMembers) {
                    std::map<std::string, Member> prevMembers;
                    if (!members.empty()) {
                        for (const Member &member : members) {
                                        prevMembers[member.getUuid()] = member;
                                    }
                        members.clear();
                    }

                    for (const Member &initialMember : initialMembers) {
                                    members.insert(initialMember);
                                }

                    if (prevMembers.empty()) {
                        //this means this is the first time client connected to server
                        logger.info(membersString());
                        clusterService.handleInitialMembershipEvent(
                                InitialMembershipEvent(client.getCluster(), members));
                        initialListFetchedLatch.get()->countDown();
                        return;
                    }

                    std::vector<MembershipEvent> events = detectMembershipEvents(prevMembers);
                    logger.info(membersString());
                    fireMembershipEvent(events);
                    initialListFetchedLatch.get()->countDown();
                }

                void
                ClientMembershipListener::handleMemberAttributeChangeEventV10(const std::string &uuid, const std::string &key,
                                                                      const int32_t &operationType,
                                                                      std::unique_ptr<std::string> &value) {
                    std::vector<Member> members = clusterService.getMemberList();
                    for (Member &target : members) {
                                    if (target.getUuid() == uuid) {
                                        Member::MemberAttributeOperationType type = (Member::MemberAttributeOperationType) operationType;
                                        target.updateAttribute(type, key, value);
                                        MemberAttributeEvent memberAttributeEvent(client.getCluster(), target,
                                                                                  (MemberAttributeEvent::MemberAttributeOperationType) type,
                                                                                  key, value.get() ? (*value) : "");
                                        clusterService.fireMemberAttributeEvent(memberAttributeEvent);
                                        break;
                                    }
                                }

                }

                void ClientMembershipListener::memberAdded(const Member &member) {
                    members.insert(member);
                    logger.info() << membersString();
                    MembershipEvent event(client.getCluster(), member, MembershipEvent::MEMBER_ADDED,
                                          std::vector<Member>(members.begin(), members.end()));
                    clusterService.handleMembershipEvent(event);
                }

                std::string ClientMembershipListener::membersString() const {
                    std::stringstream out;
                    out << std::endl << std::endl << "Members [" << members.size() << "]  {";

                    for (const Member &member : members) {
                                    out << std::endl << "\t" << member;
                                }
                    out << std::endl << "}" << std::endl;

                    return out.str();
                }

                void ClientMembershipListener::memberRemoved(const Member &member) {
                    members.erase(member);
                    logger.info() << membersString();
                    std::shared_ptr<connection::Connection> connection = connectionManager.getActiveConnection(
                            member.getAddress());
                    if (connection.get() != NULL) {
                        connection->close("", newTargetDisconnectedExceptionCausedByMemberLeftEvent(connection));
                    }
                    MembershipEvent event(client.getCluster(), member, MembershipEvent::MEMBER_REMOVED,
                                          std::vector<Member>(members.begin(), members.end()));
                    clusterService.handleMembershipEvent(event);
                }

                std::shared_ptr<exception::IException>
                ClientMembershipListener::newTargetDisconnectedExceptionCausedByMemberLeftEvent(
                        const std::shared_ptr<connection::Connection> &connection) {
                    return (exception::ExceptionBuilder<exception::TargetDisconnectedException>(
                            "ClientMembershipListener::newTargetDisconnectedExceptionCausedByMemberLeftEvent")
                            << "The client has closed the connection to this member, after receiving a member left event from the cluster. "
                            << *connection).buildShared();
                }

                std::vector<MembershipEvent>
                ClientMembershipListener::detectMembershipEvents(std::map<std::string, Member> &prevMembers) {
                    std::vector<MembershipEvent> events;

                    const std::set<Member> &eventMembers = members;

                    std::vector<Member> newMembers;
                    for (const Member &member : members) {
                                    std::map<std::string, Member>::iterator formerEntry = prevMembers.find(
                                            member.getUuid());
                                    if (formerEntry != prevMembers.end()) {
                                        prevMembers.erase(formerEntry);
                                    } else {
                                        newMembers.push_back(member);
                                    }
                                }

                    // removal events should be added before added events
                    typedef const std::map<std::string, Member> MemberMap;
                    for (const MemberMap::value_type &member : prevMembers) {
                                    events.push_back(MembershipEvent(client.getCluster(), member.second,
                                                                     MembershipEvent::MEMBER_REMOVED,
                                                                     std::vector<Member>(eventMembers.begin(),
                                                                                         eventMembers.end())));
                                    const Address &address = member.second.getAddress();
                                    if (clusterService.getMember(address).get() == NULL) {
                                        std::shared_ptr<connection::Connection> connection = connectionManager.getActiveConnection(
                                                address);
                                        if (connection.get() != NULL) {
                                            connection->close("",
                                                              newTargetDisconnectedExceptionCausedByMemberLeftEvent(
                                                                      connection));
                                        }
                                    }
                                }
                    for (const Member &member : newMembers) {
                                    events.push_back(
                                            MembershipEvent(client.getCluster(), member, MembershipEvent::MEMBER_ADDED,
                                                            std::vector<Member>(eventMembers.begin(),
                                                                                eventMembers.end())));
                                }

                    return events;
                }

                void ClientMembershipListener::fireMembershipEvent(std::vector<MembershipEvent> &events) {
                    for (const MembershipEvent &event : events) {
                                    clusterService.handleMembershipEvent(event);
                                }
                }

                void
                ClientMembershipListener::listenMembershipEvents(
                        const std::shared_ptr<ClientMembershipListener> &listener,
                        const std::shared_ptr<connection::Connection> &ownerConnection) {
                    listener->initialListFetchedLatch = std::shared_ptr<util::CountDownLatch>(
                            new util::CountDownLatch(1));
                    std::unique_ptr<protocol::ClientMessage> clientMessage = protocol::codec::ClientAddMembershipListenerCodec::encodeRequest(
                            false);
                    std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(listener->client,
                                                                                              clientMessage, "",
                                                                                              ownerConnection);
                    invocation->setEventHandler(listener);
                    invocation->invokeUrgent()->get();
                    listener->waitInitialMemberListFetched();
                }

                void ClientMembershipListener::waitInitialMemberListFetched() {
                    bool success = initialListFetchedLatch.get()->await(INITIAL_MEMBERS_TIMEOUT_SECONDS);
                    if (!success) {
                        logger.warning("Error while getting initial member list from cluster!");
                    }
                }
            }
        }
    }
}
