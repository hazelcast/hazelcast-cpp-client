//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "hazelcast/client/protocol/AddMembershipListenerRequest.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/MemberShipEvent.h"
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
                        events.push_back(MembershipEvent((*it), MembershipEvent::MEMBER_ADDED));
                    }
                }

                for (std::map< std::string, Member >::iterator it = prevMembers.begin(); it != prevMembers.end(); ++it) {
                    events.push_back(MembershipEvent(it->second, MembershipEvent::MEMBER_REMOVED));
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
                    boost::shared_ptr<MembershipEvent> event = clientContext.getSerializationService().toObject<MembershipEvent>(response->getData());
                    Member member = event->getMember();
                    if (event->getEventType() == MembershipEvent::MEMBER_ADDED) {
                        members.push_back(member);
                    } else if (event->getEventType() == MembershipEvent::MEMBER_REMOVED) {
                        members.erase(std::find(members.begin(), members.end(), member));
//                        connectionManager.removeConnectionPool(member.getAddress()); MTODO
                    } else {
                        std::cerr << "error in ClusterListenerThread::listenMembershipEvents() " << event->getEventType() << std::endl;
                    }
                    updateMembersRef();
                    clientContext.getClusterService().fireMembershipEvent(*event);
                }
            };

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
