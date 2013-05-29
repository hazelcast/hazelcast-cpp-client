//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "MembershipEvent.h"
#include "../protocol/AddMembershipListenerRequest.h"
#include "../../util/SerializableCollection.h"
#include "ClientConfig.h"
#include "../spi/ClusterService.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ClusterListenerThread::ClusterListenerThread(ConnectionManager& connectionMgr, hazelcast::client::ClientConfig& clientConfig, hazelcast::client::spi::ClusterService& clusterService)
            : connectionManager(connectionMgr)
            , clientConfig(clientConfig)
            , clusterService(clusterService)
            , conn(NULL) {

            };

            void ClusterListenerThread::setInitialConnection(hazelcast::client::connection::Connection *connection) {
                this->conn = connection;
            };

            void *ClusterListenerThread::run(void *input) {
                static_cast<ClusterListenerThread *>(input)->runImpl();
                return NULL;

            };

            void ClusterListenerThread::runImpl() {
                while (true) {
                    try {
                        if (conn == NULL) {
                            conn = pickConnection();
                            std::cout << "Connected: " << (*conn) << std::endl;
                        }
                        loadInitialMemberList();
                        listenMembershipEvents();
                    } catch (hazelcast::client::HazelcastException& e) {
//                        if (client.getLifecycleService().isRunning()) {
//                            e.printStackTrace();
//                        }
                        std::cout << *conn << " FAILED..." << std::endl;
                        conn->close();
                    }
                    try {
                        sleep(1);
                    } catch (void *x) {
                        break;
                    }
                }
            };

            Connection *ClusterListenerThread::pickConnection() {
                std::vector<Address> addresses;
                if (!members.empty()) {
                    vector<Address> clusterAddresses = getClusterAddresses();
                    addresses.insert(addresses.begin(), clusterAddresses.begin(), clusterAddresses.end());
                }
                vector<Address> configAddresses = getConfigAddresses();
                addresses.insert(addresses.end(), configAddresses.begin(), configAddresses.end());
//                std::cout << "Possible addresses: " << addresses << std::endl;
                return clusterService.connectToOne(addresses);
            };

            void ClusterListenerThread::loadInitialMemberList() {
//                hazelcast::client::protocol::AddMembershipListenerRequest request;
//                hazelcast::util::SerializableCollection coll;
//                clusterService.sendAndReceive(*conn, request, coll);
//                serialization::SerializationService & serializationService = clusterService.getSerializationService();
//                std::map<std::string, Member> prevMembers;
//                if (!members.empty()) {
////                    prevMembers = new HashMap<String, MemberImpl>(members.size());
//                    for (Member member : members) {
//                        prevMembers[member.getUuid()] = member;
//                    }
//                    members.clear();
//                }
//                for (hazelcast::client::serialization::Data *data : coll.getCollection()) {
//                    Member member;
//                    serializationService.toObject(*data, member);
//                    members.push_back(member);
//                }
////                System.err.println("members = " + members);
//                updateMembersRef();
//                std::vector<MembershipEvent> events;
//                for (Member member : members) {
////                    Member former = prevMembers.remove(member.getUuid());
//                    Member former;
//                    if (prevMembers.count(member.getUuid()) > 0) {
//                        prevMembers.erase(member.getUuid());
//                    } else {
//                        events.push_back(MembershipEvent(member, MembershipEvent::MEMBER_ADDED));
//                    }
//                }
//                std::map< std::string, Member >::iterator it;
//                for (it = prevMembers.begin(); it != prevMembers.end(); ++it) {
//                    events.push_back(MembershipEvent(it->second, MembershipEvent::MEMBER_REMOVED));
//                }
//                for (MembershipEvent event : events) {
//                    fireMembershipEvent(event);
//                }
            };

            void ClusterListenerThread::listenMembershipEvents() {
//                    final Map<Address, MemberImpl> map = new LinkedHashMap<Address, MemberImpl>(members.size());
//                    for (MemberImpl member : members) {
//                        map.put(member.getAddress(), member);
//                    }
//                    membersRef.set(Collections.unmodifiableMap(map));
            };


            void ClusterListenerThread::fireMembershipEvent(MembershipEvent & event) {
//                    client.getClientExecutionService().execute(new Runnable() {
//                        public void run() {
//                            for (MembershipListener listener : listeners.values()) {
//                                if (event.getEventType() == MembershipEvent.MEMBER_ADDED) {
//                                    listener.memberAdded(event);
//                                } else {
//                                    listener.memberRemoved(event);
//                                }
//                            }
//                        }
//                    });
            };

            void ClusterListenerThread::updateMembersRef() {
//                final Map<Address, MemberImpl> map = new LinkedHashMap<Address, MemberImpl>(members.size());
//                for (MemberImpl member : members) {
//                    map.put(member.getAddress(), member);
//                }
//                membersRef.set(Collections.unmodifiableMap(map));

            };

            std::vector<Address> ClusterListenerThread::getClusterAddresses() const {
                std::vector<Address> socketAddresses;
                for (Member member : members) {
                    socketAddresses.push_back(member.getAddress());
                }
//                Collections.shuffle(socketAddresses);
                return socketAddresses;
            };

            vector<Address>  ClusterListenerThread::getConfigAddresses() const {
                std::vector<Address> socketAddresses;
                vector<Address>  & configAddresses = clientConfig.getAddresses();
                for (Address address : configAddresses) {
                    socketAddresses.push_back(address);
                }
//                Collections.shuffle(socketAddresses);
                return socketAddresses;
            };


        }
    }
}