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

#include <boost/foreach.hpp>

#include "hazelcast/client/internal/cluster/impl/MemberSelectingCollection.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/util/UuidUtil.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/spi/impl/ClientMembershipListener.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                impl::ClientClusterServiceImpl::ClientClusterServiceImpl(hazelcast::client::spi::ClientContext &client)
                        : client(client) {
                    ClientConfig &config = client.getClientConfig();
                    const std::set<boost::shared_ptr<MembershipListener> > &membershipListeners = config.getManagedMembershipListeners();

                    BOOST_FOREACH(const boost::shared_ptr<MembershipListener> &listener, membershipListeners) {
                                    addMembershipListenerWithoutInit(listener);
                                }
                }

                std::string ClientClusterServiceImpl::addMembershipListenerWithoutInit(
                        const boost::shared_ptr<MembershipListener> &listener) {
                    std::string id = util::UuidUtil::newUnsecureUuidString();
                    listeners.put(id, listener);
                    listener->setRegistrationId(id);
                    return id;
                }

                boost::shared_ptr<Member> ClientClusterServiceImpl::getMember(const Address &address) {
                    std::map<Address, boost::shared_ptr<Member> > currentMembers = members.get();
                    const std::map<hazelcast::client::Address, boost::shared_ptr<hazelcast::client::Member> >::iterator &it = currentMembers.find(
                            address);
                    if (it == currentMembers.end()) {
                        return boost::shared_ptr<Member>();
                    }
                    return it->second;
                }

                boost::shared_ptr<Member> ClientClusterServiceImpl::getMember(const std::string &uuid) {
                    std::vector<Member> memberList = getMemberList();
                    BOOST_FOREACH(const Member &member, memberList) {
                                    if (uuid == member.getUuid()) {
                                        return boost::shared_ptr<Member>(new Member(member));
                                    }
                                }
                    return boost::shared_ptr<Member>();
                }

                std::vector<Member> ClientClusterServiceImpl::getMemberList() {
                    typedef std::map<Address, boost::shared_ptr<Member> > MemberMap;
                    MemberMap memberMap = members.get();
                    std::vector<Member> memberList;
                    BOOST_FOREACH(const MemberMap::value_type &entry, memberMap) {
                                    memberList.push_back(*entry.second);
                                }
                    return memberList;
                }

                void ClientClusterServiceImpl::initMembershipListener(MembershipListener &listener) {
                    if (listener.shouldRequestInitialMembers()) {
                        Cluster &cluster = client.getCluster();
                        std::vector<Member> memberCollection = getMemberList();
                        InitialMembershipEvent event(cluster, std::set<Member>(memberCollection.begin(),
                                                                               memberCollection.end()));
                        ((InitialMembershipListener &) listener).init(event);
                    }
                }

                void ClientClusterServiceImpl::start() {
                    clientMembershipListener.reset(new ClientMembershipListener(client));
                }

                void ClientClusterServiceImpl::handleMembershipEvent(const MembershipEvent &event) {
                    util::LockGuard guard(initialMembershipListenerMutex);
                    const Member &member = event.getMember();
                    std::map<Address, boost::shared_ptr<Member> > newMap = members.get();
                    if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                        newMap[member.getAddress()] = boost::shared_ptr<Member>(new Member(member));
                    } else {
                        newMap.erase(member.getAddress());
                    }
                    members = newMap;
                    fireMembershipEvent(event);
                }

                void ClientClusterServiceImpl::fireMembershipEvent(const MembershipEvent &event) {
                    BOOST_FOREACH(const boost::shared_ptr<MembershipListener> &listener, listeners.values()) {
                                    if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                                        listener->memberAdded(event);
                                    } else {
                                        listener->memberRemoved(event);
                                    }
                                }
                }

                void ClientClusterServiceImpl::fireMemberAttributeEvent(const MemberAttributeEvent &event) {
                    BOOST_FOREACH(const boost::shared_ptr<MembershipListener> &listener, listeners.values()) {
                                    listener->memberAttributeChanged(event);
                                }
                }

                void ClientClusterServiceImpl::handleInitialMembershipEvent(const InitialMembershipEvent &event) {
                    util::LockGuard guard(initialMembershipListenerMutex);
                    const std::vector<Member> &initialMembers = event.getMembers();
                    std::map<Address, boost::shared_ptr<Member> > newMap;
                    BOOST_FOREACH (const Member &initialMember, initialMembers) {
                                    newMap[initialMember.getAddress()] = boost::shared_ptr<Member>(
                                            new Member(initialMember));
                                }
                    members.set(newMap);
                    fireInitialMembershipEvent(event);

                }

                void ClientClusterServiceImpl::fireInitialMembershipEvent(const InitialMembershipEvent &event) {
                    BOOST_FOREACH (const boost::shared_ptr<MembershipListener> &listener, listeners.values()) {
                                    if (listener->shouldRequestInitialMembers()) {
                                        ((InitialMembershipListener *) listener.get())->init(event);
                                    }
                                }
                }

                void ClientClusterServiceImpl::shutdown() {
                }

                void ClientClusterServiceImpl::listenMembershipEvents(
                        const boost::shared_ptr<connection::Connection> &ownerConnection) {
                    clientMembershipListener->listenMembershipEvents(clientMembershipListener, ownerConnection);
                }

                std::string
                ClientClusterServiceImpl::addMembershipListener(const boost::shared_ptr<MembershipListener> &listener) {
                    if (listener.get() == NULL) {
                        throw exception::NullPointerException("ClientClusterServiceImpl::addMembershipListener",
                                                              "listener can't be null");
                    }

                    util::LockGuard guard(initialMembershipListenerMutex);
                    std::string id = addMembershipListenerWithoutInit(listener);
                    initMembershipListener(*listener);
                    return id;
                }

                bool ClientClusterServiceImpl::removeMembershipListener(const std::string &registrationId) {
                    return listeners.remove(registrationId).get() != NULL;
                }

                boost::shared_ptr<util::Collection<Member> >
                ClientClusterServiceImpl::getMembers(const cluster::memberselector::MemberSelector &selector) {
                    return boost::shared_ptr<util::Collection<Member> >(
                            new internal::cluster::impl::MemberSelectingCollection(getMemberList(), selector));
                }
            }
        }
    }
}
