//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "MembershipEvent.h"
#include "../protocol/AddMembershipListenerRequest.h"
#include "../../util/SerializableCollection.h"
#include "../ClientConfig.h"
#include "../spi/ClusterService.h"
#include "../MembershipListener.h"

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
                hazelcast::client::protocol::AddMembershipListenerRequest request;
                hazelcast::util::SerializableCollection coll;
                clusterService.sendAndReceive(*conn, request, coll);
                serialization::SerializationService & serializationService = clusterService.getSerializationService();
                std::map<std::string, Member> prevMembers;
                if (!members.empty()) {
                    for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                        prevMembers[(*it).getUuid()] = *it;
                    }
                    members.clear();
                }

                vector<serialization::Data *> collection = coll.getCollection();
                for (vector<serialization::Data *> ::iterator it = collection.begin(); it != collection.end(); ++it) {
                    Member member;
                    serializationService.toObject(*(*it), member);
                    members.push_back(member);
                }
                updateMembersRef();
                std::vector<MembershipEvent> events;

                for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                    if (prevMembers.count((*it).getUuid()) > 0) {
                        prevMembers.erase((*it).getUuid());
                    } else {
                        events.push_back(MembershipEvent((*it), MembershipEvent::MEMBER_ADDED));
                    }
                }

                for (std::map< std::string, Member >::iterator it = prevMembers.begin(); it != prevMembers.end(); ++it) {
                    events.push_back(MembershipEvent(it->second, MembershipEvent::MEMBER_REMOVED));
                }


                for (std::vector<MembershipEvent>::iterator it = events.begin(); it != events.end(); ++it) {
                    fireMembershipEvent((*it));
                }
            };

            void ClusterListenerThread::listenMembershipEvents() {
                serialization::SerializationService & serializationService = clusterService.getSerializationService();
                while (true) {
                    hazelcast::client::serialization::Data data;
                    conn->read(data);
                    MembershipEvent event;
                    serializationService.toObject(data, event);
                    Member member = event.getMember();
                    if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                        members.push_back(member);
                    } else {
                        //TODO O(n) time complexity ???
                        members.erase(std::find(members.begin(), members.end(), member));
                    }
                    updateMembersRef();
                    connectionManager.removeConnectionPool(member.getAddress());
                    fireMembershipEvent(event);
                }
            };


            void ClusterListenerThread::fireMembershipEvent(MembershipEvent & event) {
                //TODO give this job to another thread
                vector<MembershipListener *> values = clusterService.listeners.values();

                for (vector<MembershipListener *>::iterator it = values.begin(); it != values.end(); ++it) {
                    if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                        (*it)->memberAdded(event);
                    } else {
                        (*it)->memberRemoved(event);
                    }
                }
            };

            void ClusterListenerThread::updateMembersRef() {
                std::map<hazelcast::client::Address, Member> *map = new std::map<hazelcast::client::Address, Member>;
                std::cout << "Members" << std::endl;
                for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                    std::cout << "\t" << (*it).getAddress() << std::endl;
                    (*map)[(*it).getAddress()] = (*it);
                }
                delete clusterService.membersRef.set(map);

            };

            std::vector<Address> ClusterListenerThread::getClusterAddresses() const {
                std::vector<Address> socketAddresses;
                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    socketAddresses.push_back((*it).getAddress());
                }
                //                Collections.shuffle(socketAddresses);
                return socketAddresses;
            };

            vector<Address>  ClusterListenerThread::getConfigAddresses() const {
                std::vector<Address> socketAddresses;
                std::vector<Address>  & configAddresses = clientConfig.getAddresses();

                for (std::vector<Address>::iterator it = configAddresses.begin(); it != configAddresses.end(); ++it) {
                    socketAddresses.push_back((*it));
                }
                //                Collections.shuffle(socketAddresses);
                return socketAddresses;
            };


        }
    }
}