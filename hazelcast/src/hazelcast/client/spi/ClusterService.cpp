/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 5/21/13.

#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/LockGuard.h"
#include <ctime>

namespace hazelcast {
    namespace client {
        namespace spi {
            ClusterService::ClusterService(ClientContext &clientContext)
                    : clientContext(clientContext), clusterThread(clientContext), active(false) {

            }

            bool ClusterService::start() {
                ClientConfig &config = clientContext.getClientConfig();
                std::set<MembershipListener *> const &membershipListeners = config.getMembershipListeners();
                listeners.insert(membershipListeners.begin(), membershipListeners.end());
                std::set<InitialMembershipListener *> const &initialMembershipListeners = config.getInitialMembershipListeners();
                initialListeners.insert(initialMembershipListeners.begin(), initialMembershipListeners.end());

                util::Thread *t = new util::Thread("hz.clusterListenerThread",
                                                   connection::ClusterListenerThread::staticRun, &clusterThread);
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
                std::set<InitialMembershipListener *>::iterator it;
                Cluster &cluster = clientContext.getCluster();
                InitialMembershipEvent event(cluster, cluster.getMembers());
                for (it = initialListeners.begin(); it != initialListeners.end(); ++it) {
                    (*it)->init(event);
                }
            }

            void ClusterService::shutdown() {
                active = false;
                if (NULL != clusterThread.getThread()) {
                    // avoid anyone waiting on the start latch to get stuck
                    clusterThread.startLatch.countDown();

                    clusterThread.stop();
                }
            }

            std::auto_ptr<Address> ClusterService::getMasterAddress() {
                util::LockGuard guard(membersLock);
                typedef std::map<Address, Member, addressComparator> MemberMap;
                if (NULL != members.get() && members->size() > 0) {
                    MemberMap::const_iterator it = members->begin();
                    return std::auto_ptr<Address>(new Address(it->first));
                } else {
                    return std::auto_ptr<Address>(NULL);
                }
            }

            void ClusterService::addMembershipListener(MembershipListener *listener) {
                util::LockGuard guard(listenerLock);
                listeners.insert(listener);
            }


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
            }

            bool ClusterService::removeMembershipListener(InitialMembershipListener *listener) {
                util::LockGuard guard(listenerLock);
                bool b = initialListeners.erase(listener) == 1;
                return b;
            }


            bool ClusterService::isMemberExists(Address const &address) {
                util::LockGuard guard(membersLock);
                return NULL != members.get() && members->count(address) > 0;
            }

            const Member &ClusterService::getMember(Address &address) {
                util::LockGuard guard(membersLock);
                return (*members)[address];
            }

            std::auto_ptr<Member> ClusterService::getMember(const std::string &uuid) {
                std::auto_ptr<Member> result;
                util::LockGuard guard(membersLock);
                if (NULL != members.get()) {
                    for (std::map<Address, Member, addressComparator>::iterator it = members->begin();
                         it != members->end(); ++it) {
                        if (0 == uuid.compare(it->second.getUuid())) {
                            result = std::auto_ptr<Member>(new Member(it->second));
                            break;
                        }
                    }
                }
                return result;
            }

            std::vector<Member> ClusterService::getMemberList() {
                typedef std::map<Address, Member, addressComparator> MemberMap;
                std::vector<Member> v;
                util::LockGuard guard(membersLock);
                if (NULL != members.get()) {
                    for (MemberMap::const_iterator it = members->begin(); it != members->end(); ++it) {
                        v.push_back(it->second);
                    }
                }
                return v;
            }

            std::vector<Address> ClusterService::findServerAddressesToConnect(const Address *previousConnectionAddr) const {
                std::set<Address, addressComparator> socketAddresses = this->clusterThread.getSocketAddresses();
                std::vector<Address> addresses;
                for (std::set<Address, addressComparator>::const_iterator it = socketAddresses.begin();
                     it != socketAddresses.end(); it++) {
                    if ((Address *) NULL != previousConnectionAddr) {
                        if (*previousConnectionAddr != *it) {
                            addresses.push_back(*it);
                        }
                    }
                    addresses.push_back(*it);
                }
                if ((Address *) NULL != previousConnectionAddr) {
                    addresses.push_back(*previousConnectionAddr);
                }
                return addresses;
            }
            //--------- Used by CLUSTER LISTENER THREAD ------------

            boost::shared_ptr<connection::Connection> ClusterService::connectToOne(const Address *previousConnectionAddr) {
                active = false;
                const int connectionAttemptLimit = clientContext.getClientConfig().getConnectionAttemptLimit();
                int attempt = 0;
                exception::IException lastError;
                while (true) {
                    if (util::ILogger::getLogger().isEnabled(FINEST)) {
                        std::stringstream message;
                        message << " Trying to connect to owner node " << attempt << ". attempt ";
                        util::ILogger::getLogger().finest(message.str());
                    }

                    time_t tryStartTime = std::time(NULL);
                    std::vector<Address> addresses = findServerAddressesToConnect(previousConnectionAddr);
                    for (std::vector<Address>::const_iterator it = addresses.begin(); it != addresses.end(); ++it) {
                        try {
                            boost::shared_ptr<connection::Connection> pConnection =
                                    clientContext.getConnectionManager().createOwnerConnection(*it);
                            active = true;
                            clientContext.getLifecycleService().fireLifecycleEvent(LifecycleEvent::CLIENT_CONNECTED);
                            return pConnection;
                        } catch (exception::IException &e) {
                            lastError = e;
                            std::ostringstream errorStream;
                            errorStream << "IO error  during initial connection to " << (*it) << " for owner connection =>" << e.what();
                            util::ILogger::getLogger().warning(errorStream.str());
                        }
                    }

                    if (attempt++ >= connectionAttemptLimit) {
                        break;
                    }
                    const double remainingTime = clientContext.getClientConfig().getAttemptPeriod() -
                                                 std::difftime(std::time(NULL), tryStartTime);
                    using namespace std;
                    std::ostringstream errorStream;
                    errorStream << "Unable to get alive cluster connection, try in " << max(0.0, remainingTime)
                    << " ms later, attempt " << attempt << " of " << connectionAttemptLimit << ".";
                    util::ILogger::getLogger().warning(errorStream.str());

                    if (remainingTime > 0) {
                        util::sleep((unsigned) remainingTime / 1000);//MTODO
                    }
                }
                throw exception::IllegalStateException("ClusterService",
                                                       "Unable to connect to any address in the config! =>" +
                                                       std::string(lastError.what()));
            }

            void ClusterService::fireMembershipEvent(const MembershipEvent &event) {
                util::LockGuard guard(listenerLock);
                for (std::set<MembershipListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                        // TODO: Java client executes this call using a thread, apply similar logic here
                        (*it)->memberAdded(event);
                    } else if (event.getEventType() == MembershipEvent::MEMBER_REMOVED) {
                        (*it)->memberRemoved(event);
                    }
                }

                for (std::set<InitialMembershipListener *>::iterator it = initialListeners.begin();
                     it != initialListeners.end(); ++it) {
                    if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                        (*it)->memberAdded(event);
                    } else if (event.getEventType() == MembershipEvent::MEMBER_REMOVED) {
                        (*it)->memberRemoved(event);
                    }
                }
            }


            void ClusterService::fireMemberAttributeEvent(const MemberAttributeEvent &event) {
                util::LockGuard guard(listenerLock);
                for (std::set<MembershipListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    (*it)->memberAttributeChanged(event);
                }

                for (std::set<InitialMembershipListener *>::iterator it = initialListeners.begin();
                     it != initialListeners.end(); ++it) {
                    (*it)->memberAttributeChanged(event);
                }
            }

            void ClusterService::setMembers(std::auto_ptr<std::map<Address, Member, addressComparator> > map) {
                util::LockGuard guard(membersLock);
                members = map;
            }

            std::string ClusterService::membersString() {
                std::vector<Member> currentMembers = getMemberList();

                std::stringstream memberInfo;
                memberInfo << std::endl << "Members [" << currentMembers.size() << "]  {" << std::endl;

                for (std::vector<Member>::const_iterator it = currentMembers.begin();
                     it != currentMembers.end(); ++it) {
                    memberInfo << "\t" << *it << std::endl;
                }
                memberInfo << "}" << std::endl;

                return memberInfo.str();
            }
        }
    }
}

