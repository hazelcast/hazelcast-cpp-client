//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include <ctime>

namespace hazelcast {
    namespace client {
        namespace spi {
            ClusterService::ClusterService(ClientContext &clientContext)
            : clientContext(clientContext)
            , clusterThread(clientContext)
            , active(false) {

            }

            bool ClusterService::start() {

                ClientConfig &config = clientContext.getClientConfig();
                std::set<MembershipListener *> const &membershipListeners = config.getMembershipListeners();
                listeners.insert(membershipListeners.begin(), membershipListeners.end());
                std::set<InitialMembershipListener *> const &initialMembershipListeners = config.getInitialMembershipListeners();
                initialListeners.insert(initialMembershipListeners.begin(), initialMembershipListeners.end());

                util::Thread *t = new util::Thread("hz.clusterListenerThread", connection::ClusterListenerThread::staticRun, &clusterThread);
                clusterThread.setThread(t);
                clusterThread.startLatch.await();
                if (!clusterThread.isStartedSuccessfully) {
                    return false;
                }
                initMembershipListeners();
                active = true;
                return true;
            }

            void ClusterService::initMembershipListeners() {
                util::LockGuard guard(listenerLock);
                std::set< InitialMembershipListener *>::iterator it;
                Cluster &cluster = clientContext.getCluster();
                InitialMembershipEvent event(cluster, cluster.getMembers());
                for (it = initialListeners.begin(); it != initialListeners.end(); ++it) {
                    (*it)->init(event);
                }
            }

            void ClusterService::shutdown() {
                active = false;
                clusterThread.stop();
            }

            std::auto_ptr<Address> ClusterService::getMasterAddress() {
                std::vector<Member> list = getMemberList();
                if (list.empty()) {
                    return std::auto_ptr<Address>(NULL);
                }
                return std::auto_ptr<Address>(new Address(list[0].getAddress()));
            }

            void ClusterService::addMembershipListener(MembershipListener *listener) {
                util::LockGuard guard(listenerLock);
                listeners.insert(listener);
            };


            void ClusterService::addMembershipListener(InitialMembershipListener *listener) {
                util::LockGuard guard(listenerLock);
                Cluster &cluster = clientContext.getCluster();
                InitialMembershipEvent event(cluster, cluster.getMembers());
                listener->init(event);
                initialListeners.insert(listener);
            }

            bool ClusterService::removeMembershipListener(MembershipListener *listener) {
                util::LockGuard guard(listenerLock);
                bool b = listeners.erase(listener) == 1;
                return b;
            };

            bool ClusterService::removeMembershipListener(InitialMembershipListener *listener) {
                util::LockGuard guard(listenerLock);
                bool b = initialListeners.erase(listener) == 1;
                return b;
            }


            bool ClusterService::isMemberExists(Address const &address) {
                util::LockGuard guard(membersLock);
                return members.count(address) > 0;
            };

            Member ClusterService::getMember(Address &address) {
                util::LockGuard guard(membersLock);
                return members[address];
            }

            Member ClusterService::getMember(const std::string &uuid) {
                std::vector<Member> list = getMemberList();
                for (std::vector<Member>::iterator it = list.begin(); it != list.end(); ++it) {
                    if (uuid.compare(it->getUuid())) {
                        return *it;
                    }
                }
                return Member();
            };

            std::vector<Member>  ClusterService::getMemberList() {
                typedef std::map<Address, Member, addressComparator> MemberMap;
                std::vector<Member> v;
                util::LockGuard guard(membersLock);
                MemberMap::const_iterator it;
                for (it = members.begin(); it != members.end(); it++) {
                    v.push_back(it->second);
                }
                return v;
            };


            //--------- Used by CLUSTER LISTENER THREAD ------------

            boost::shared_ptr<connection::Connection> ClusterService::connectToOne() {
                active = false;
                const int connectionAttemptLimit = clientContext.getClientConfig().getConnectionAttemptLimit();
                int attempt = 0;
                exception::IException lastError;
                while (true) {
                    time_t tryStartTime = std::time(NULL);
                    std::vector<Address>::const_iterator it;
                    std::vector<Address> socketAddresses = clusterThread.getSocketAddresses();
                    for (it = socketAddresses.begin(); it != socketAddresses.end(); it++) {
                        try {
                            boost::shared_ptr<connection::Connection> pConnection = clientContext.getConnectionManager().createOwnerConnection(*it);
                            active = true;
                            clientContext.getLifecycleService().fireLifecycleEvent(LifecycleEvent::CLIENT_CONNECTED);
                            return pConnection;
                        } catch (exception::IException &e) {
                            lastError = e;
                            std::ostringstream errorStream;
                            errorStream << "IO error  during initial connection =>" << e.what();
                            util::ILogger::getLogger().warning(errorStream.str());
                        }
                    }
                    if (attempt++ >= connectionAttemptLimit) {
                        break;
                    }
                    const double remainingTime = clientContext.getClientConfig().getAttemptPeriod() - std::difftime(std::time(NULL), tryStartTime);
                    using namespace std;
                    std::ostringstream errorStream;
                    errorStream << "Unable to get alive cluster connection, try in " << max(0.0, remainingTime)
                            << " ms later, attempt " << attempt << " of " << connectionAttemptLimit << ".";
                    util::ILogger::getLogger().warning(errorStream.str());

                    if (remainingTime > 0) {
                        util::sleep((unsigned) remainingTime / 1000);//MTODO
                    }
                }
                throw  exception::IllegalStateException("ClusterService", "Unable to connect to any address in the config! =>" + std::string(lastError.what()));
            };


            void ClusterService::fireMembershipEvent(MembershipEvent &event) {
                util::LockGuard guard(listenerLock);
                for (std::set<MembershipListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                        (*it)->memberAdded(event);
                    } else if (event.getEventType() == MembershipEvent::MEMBER_REMOVED) {
                        (*it)->memberRemoved(event);
                    }
                }

                for (std::set<InitialMembershipListener *>::iterator it = initialListeners.begin(); it != initialListeners.end(); ++it) {
                    if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                        (*it)->memberAdded(event);
                    } else if (event.getEventType() == MembershipEvent::MEMBER_REMOVED) {
                        (*it)->memberRemoved(event);
                    }
                }
            };


            void ClusterService::fireMemberAttributeEvent(MemberAttributeEvent &event) {
                util::LockGuard guard(listenerLock);
                for (std::set<MembershipListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    (*it)->memberAttributeChanged(event);
                }

                for (std::set<InitialMembershipListener *>::iterator it = initialListeners.begin(); it != initialListeners.end(); ++it) {
                    (*it)->memberAttributeChanged(event);
                }
            }

            void ClusterService::setMembers(const std::map<Address, Member, addressComparator > &map) {
                util::LockGuard guard(membersLock);
                members = map;
            }
        }
    }
}

