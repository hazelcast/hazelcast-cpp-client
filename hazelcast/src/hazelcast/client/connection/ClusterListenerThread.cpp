//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "hazelcast/client/protocol/AddMembershipListenerRequest.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/impl/ClientMemberShipEvent.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/impl/SerializableCollection.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/LifecycleEvent.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ClusterListenerThread::ClusterListenerThread(spi::ClientContext &clientContext)
            : clientContext(clientContext)
            , conn(NULL)
            , isReady(false)
            , deletingConnection(false) {

            };

            void ClusterListenerThread::setThread(boost::thread *thread) {
                clusterListenerThread.reset(thread);
            }

            void ClusterListenerThread::run() {
                while (clientContext.getLifecycleService().isRunning()) {
                    try {
                        if (conn.get() == NULL) {
                            try {
                                conn.reset(pickConnection());
                            } catch (std::exception &e) {
                                std::cerr << "Error while connecting to cluster! " << e.what() << std::endl;
                                return;
                            }
                        }
                        loadInitialMemberList();
                        isReady = true;
                        listenMembershipEvents();
                        boost::this_thread::sleep(boost::posix_time::seconds(1));
                    } catch(std::exception &e) {
                        if (clientContext.getLifecycleService().isRunning()) {
                            (std::cerr << "Error while listening cluster events! -> " << e.what() << std::endl);
                        }
                        bool expected = false;
                        if (deletingConnection.compare_exchange_strong(expected, true)) {
                            conn.reset();
                            deletingConnection = false;
                            clientContext.getLifecycleService().fireLifecycleEvent(LifecycleEvent::CLIENT_DISCONNECTED);
                        }
                        boost::this_thread::sleep(boost::posix_time::seconds(1));
                    } catch(boost::thread_interrupted &) {
                        break;
                    } catch(...) {
                        std::cerr << "cluster Listener Thread unknown exception\n";
                    }

                }

            };


            void ClusterListenerThread::stop() {
                bool expected = false;
                if (deletingConnection.compare_exchange_strong(expected, true)) {
                    conn.reset();
                    deletingConnection = false;
                }

                clusterListenerThread->interrupt();
                clusterListenerThread->join();
            }

            Connection *ClusterListenerThread::pickConnection() {
                std::vector<Address> addresses;
                if (!members.empty()) {
                    std::vector<Address> clusterAddresses = getClusterAddresses();
                    addresses.insert(addresses.begin(), clusterAddresses.begin(), clusterAddresses.end());
                }
                std::vector<Address> configAddresses = getConfigAddresses();
                addresses.insert(addresses.end(), configAddresses.begin(), configAddresses.end());
                return clientContext.getClusterService().connectToOne(addresses);
            };

            void ClusterListenerThread::loadInitialMemberList() {
                protocol::AddMembershipListenerRequest requestObject;
                serialization::Data request = clientContext.getSerializationService().toData<protocol::AddMembershipListenerRequest>(&requestObject);
                conn->writeBlocking(request);
                serialization::Data data = conn->readBlocking();
                boost::shared_ptr<ClientResponse> response = clientContext.getSerializationService().toObject<ClientResponse >(data);
                if (response->isException()) {
                    std::cerr << "ClusterListenerThread::loadInitialMemberList" << std::endl;
                    throw exception::IOException("ClusterListenerThread::loadInitialMemberList", "error while waiting initial list");
                }
                boost::shared_ptr<impl::SerializableCollection> coll = clientContext.getSerializationService().toObject<impl::SerializableCollection >(response->getData());


                std::map<std::string, Member> prevMembers;
                if (!members.empty()) {
                    for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                        prevMembers[(*it).getUuid()] = *it;
                    }
                    members.clear();
                }

                const std::vector<serialization::Data *> &collection = coll->getCollection();
                for (std::vector<serialization::Data *>::const_iterator it = collection.begin(); it != collection.end(); ++it) {
                    boost::shared_ptr<Member> member = clientContext.getSerializationService().toObject<Member>(**it);
                    members.push_back(*member);
                }
                updateMembersRef();
                std::vector<MembershipEvent> events;

                for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                    if (prevMembers.count((*it).getUuid()) > 0) {
                        prevMembers.erase((*it).getUuid());
                    } else {
                        events.push_back(MembershipEvent(clientContext.getCluster(), MembershipEvent::MEMBER_ADDED, (*it)));
                    }
                }

                for (std::map< std::string, Member >::iterator it = prevMembers.begin(); it != prevMembers.end(); ++it) {
                    events.push_back(MembershipEvent(clientContext.getCluster(), MembershipEvent::MEMBER_ADDED, it->second));
                }


                for (std::vector<MembershipEvent>::iterator it = events.begin(); it != events.end(); ++it) {
                    clientContext.getClusterService().fireMembershipEvent((*it));
                }
            };

            void ClusterListenerThread::listenMembershipEvents() {
                while (clientContext.getLifecycleService().isRunning()) {
                    serialization::Data data = conn->readBlocking();
                    if (!clientContext.getLifecycleService().isRunning())
                        break;
                    boost::shared_ptr<connection::ClientResponse> response = clientContext.getSerializationService().toObject<connection::ClientResponse>(data);
                    if (response->isException()) {
                        exception::ServerException const &ex = response->getException();
                        std::cerr << ex.what() << std::endl;
                        throw ex;
                    }
                    boost::shared_ptr<impl::ClientMembershipEvent> event = clientContext.getSerializationService().toObject<impl::ClientMembershipEvent>(response->getData());
                    Member member = event->getMember();
                    bool membersUpdated = false;
                    if (event->getEventType() == MembershipEvent::MEMBER_ADDED) {
                        members.push_back(member);
                        membersUpdated = true;
                    } else if (event->getEventType() == MembershipEvent::MEMBER_REMOVED) {
                        members.erase(std::find(members.begin(), members.end(), member));
                        membersUpdated = true;
//                        connectionManager.removeConnectionPool(member.getAddress()); MTODO
                    } else if (event->getEventType() == MembershipEvent::MEMBER_ATTRIBUTE_CHANGED) {
                        impl::MemberAttributeChange const &memberAttributeChange = event->getMemberAttributeChange();
                        for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                            Member &target = *it;
                            if (target.getUuid() == memberAttributeChange.getUuid()) {
                                fireMemberAttributeEvent(memberAttributeChange, target);
                            }
                        }
                    }
                    if (membersUpdated) {
                        updateMembersRef();
                        MembershipEvent membershipEvent(clientContext.getCluster(), event->getEventType(), member);
                        clientContext.getClusterService().fireMembershipEvent(membershipEvent);
                    }
                }
            };


            void ClusterListenerThread::fireMemberAttributeEvent(hazelcast::client::impl::MemberAttributeChange const &memberAttributeChange, Member &target) {
                MemberAttributeEvent::MapOperationType operationType = memberAttributeChange.getOperationType();
                const std::string &value = memberAttributeChange.getValue();
                util::IOUtil::PRIMITIVE_ID primitive_id = memberAttributeChange.getTypeId();
                const std::string &key = memberAttributeChange.getKey();
                if (operationType == MemberAttributeEvent::DELTA_MEMBER_PROPERTIES_OP_PUT) {//PUT
//                    if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_BOOLEAN) {
//                        target.setAttribute(key, value);                MTODO
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_BYTE) {
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_DOUBLE) {
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_FLOAT) {
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_INTEGER) {
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_LONG) {
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_SHORT) {
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_UTF) {
//                    }
                } else if (operationType == MemberAttributeEvent::DELTA_MEMBER_PROPERTIES_OP_REMOVE) {//REMOVE
//                    if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_BOOLEAN) {
//                        target.removeAttribute<bool>(key);               MTODO
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_BYTE) {
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_DOUBLE) {
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_FLOAT) {
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_INTEGER) {
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_LONG) {
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_SHORT) {
//                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_UTF) {
//                    }
                }
                MemberAttributeEvent memberAttributeEvent(clientContext.getCluster(), target, operationType, key, value, primitive_id);
                clientContext.getClusterService().fireMembershipEvent(memberAttributeEvent);
            }

            void ClusterListenerThread::updateMembersRef() {
                std::map<Address, Member, addressComparator > map;
                std::cerr << "Members [" << members.size() << "]  {" << std::endl;
                for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                    (std::cerr << "\t" << (*it) << std::endl);
                    map[(*it).getAddress()] = (*it);
                }
                std::cerr << "}" << std::endl;
                clientContext.getClusterService().setMembers(map);

            };

            std::vector<Address> ClusterListenerThread::getClusterAddresses() const {
                std::vector<Address> socketAddresses;
                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    socketAddresses.push_back((*it).getAddress());
                }
                return socketAddresses;
            };

            std::vector<Address>  ClusterListenerThread::getConfigAddresses() const {
                std::vector<Address> socketAddresses;
                std::vector<Address> &configAddresses = clientContext.getClientConfig().getAddresses();

                for (std::vector<Address>::iterator it = configAddresses.begin(); it != configAddresses.end(); ++it) {
                    socketAddresses.push_back((*it));
                }
                return socketAddresses;
            };
        }
    }
}
