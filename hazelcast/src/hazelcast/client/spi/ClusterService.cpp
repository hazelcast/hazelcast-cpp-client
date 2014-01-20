//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/serialization/ClassDefinitionBuilder.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/MemberShipEvent.h"
#include "hazelcast/client/exception/TargetDisconnectedException.h"
#include "hazelcast/util/CallPromise.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ClusterService::ClusterService(ClientContext &clientContext)
            : clientContext(clientContext)
            , clusterThread(clientContext)
            , active(false) {

            }

            void ClusterService::start() {
                serialization::ClassDefinitionBuilder cd(-3, 3);
                boost::shared_ptr<serialization::ClassDefinition> ptr(cd.addUTFField("uuid").addUTFField("ownerUuid").build());
                clientContext.getSerializationService().getSerializationContext().registerClassDefinition(ptr);

                boost::thread *t = new boost::thread(boost::bind(&connection::ClusterListenerThread::run, &clusterThread));
                clusterThread.setThread(t);
                while (!clusterThread.isReady) {
                    try {
                        boost::this_thread::sleep(boost::posix_time::seconds(1));
                    } catch(...) {
                        throw  exception::IException("ClusterService::start", "ClusterService can not be started");
                    }
                }
                //MTODO init membershipEvent
                active = true;
            }


            void ClusterService::stop() {
                active = false;
                clusterThread.stop();
            }

            std::auto_ptr<Address> ClusterService::getMasterAddress() {
                vector<connection::Member> list = getMemberList();
                if (list.empty()) {
                    return std::auto_ptr<Address>(NULL);
                }
                return std::auto_ptr<Address>(new Address(list[0].getAddress()));
            }

            void ClusterService::addMembershipListener(MembershipListener *listener) {
                boost::lock_guard<boost::mutex> guard(listenerLock);
                listeners.insert(listener);
            };

            bool ClusterService::removeMembershipListener(MembershipListener *listener) {
                boost::lock_guard<boost::mutex> guard(listenerLock);
                bool b = listeners.erase(listener) == 1;
                return b;
            };


            bool ClusterService::isMemberExists(Address const &address) {
                boost::lock_guard<boost::mutex> guard(membersLock);
                return members.count(address) > 0;;
            };

            connection::Member ClusterService::getMember(Address &address) {
                boost::lock_guard<boost::mutex> guard(membersLock);
                return members[address];

            }

            connection::Member ClusterService::getMember(const std::string &uuid) {
                vector<connection::Member> list = getMemberList();
                for (vector<connection::Member>::iterator it = list.begin(); it != list.end(); ++it) {
                    if (uuid.compare(it->getUuid())) {
                        return *it;
                    }
                }
                return connection::Member();
            };

            std::vector<connection::Member>  ClusterService::getMemberList() {
                typedef std::map<Address, connection::Member, addressComparator> MemberMap;
                std::vector<connection::Member> v;
                boost::lock_guard<boost::mutex> guard(membersLock);
                MemberMap::const_iterator it;
                for (it = members.begin(); it != members.end(); it++) {
                    v.push_back(it->second);
                }
                return v;
            };


            //--------- Used by CLUSTER LISTENER THREAD ------------

            connection::Connection* ClusterService::connectToOne(const std::vector<Address> &socketAddresses) {
                active = false;
                const int connectionAttemptLimit = clientContext.getClientConfig().getConnectionAttemptLimit();
                int attempt = 0;
                std::exception lastError;
                while (true) {
                    time_t tryStartTime = std::time(NULL);
                    std::vector<Address>::const_iterator it;
                    for (it = socketAddresses.begin(); it != socketAddresses.end(); it++) {
                        try {
                            connection::Connection *pConnection = clientContext.getConnectionManager().ownerConnection(*it);
                            active = true;
                            return pConnection;
                        } catch (exception::IOException &e) {
                            lastError = e;
                            std::cerr << "IO error  during initial connection..\n" << e.what() << std::endl;
                        } catch (exception::ServerException &e) {
                            lastError = e;
                            std::cerr << "IO error from server during initial connection..\n" << e.what() << std::endl;

                        }
                    }
                    if (attempt++ >= connectionAttemptLimit) {
                        break;
                    }
                    const double remainingTime = clientContext.getClientConfig().getAttemptPeriod() - std::difftime(std::time(NULL), tryStartTime);
                    using namespace std;
                    std::cerr << "Unable to get alive cluster connection, try in " << max(0.0, remainingTime)
                            << " ms later, attempt " << attempt << " of " << connectionAttemptLimit << "." << std::endl;

                    if (remainingTime > 0) {
                        boost::this_thread::sleep(boost::posix_time::milliseconds(remainingTime));
                    }
                }
                throw  exception::IException("ClusterService", "Unable to connect to any address in the config!" + std::string(lastError.what()));
            };


            void ClusterService::fireMembershipEvent(connection::MembershipEvent &event) {
                boost::lock_guard<boost::mutex> guard(listenerLock);
                for (std::set<MembershipListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    if (event.getEventType() == connection::MembershipEvent::MEMBER_ADDED) {
                        (*it)->memberAdded(event);
                    } else {
                        (*it)->memberRemoved(event);
                    }
                }
            };

            void ClusterService::setMembers(const std::map<Address, connection::Member, addressComparator > &map) {
                boost::lock_guard<boost::mutex> guard(membersLock);
                members = map;
            }
        }
    }
}
