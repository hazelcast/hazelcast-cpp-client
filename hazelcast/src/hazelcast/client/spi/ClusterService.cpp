//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/serialization/ClassDefinitionBuilder.h"
#include "hazelcast/client/connection/MemberShipEvent.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ClusterService::ClusterService(spi::PartitionService& partitionService, spi::LifecycleService& lifecycleService, connection::ConnectionManager& connectionManager, serialization::SerializationService& serializationService, ClientConfig & clientConfig)
            : partitionService(partitionService)
            , lifecycleService(lifecycleService)
            , connectionManager(connectionManager)
            , serializationService(serializationService)
            , clientConfig(clientConfig)
            , clusterThread(connectionManager, clientConfig, *this, lifecycleService, serializationService)
            , credentials(clientConfig.getCredentials())
            , redoOperation(clientConfig.isRedoOperation()) {

            }

            void ClusterService::start() {
                serialization::ClassDefinitionBuilder cd(-3, 3);
                util::AtomicPointer<serialization::ClassDefinition> ptr(cd.addUTFField("uuid").addUTFField("ownerUuid").build());
                serializationService.getSerializationContext().registerClassDefinition(ptr);

                connection::Connection *connection = connectToOne(clientConfig.getAddresses());
                clusterThread.setInitialConnection(connection);
                boost::thread clusterListener(boost::bind(&connection::ClusterListenerThread::run, &clusterThread));
                while (membersRef.get() == NULL) {
                    try {
                        boost::this_thread::sleep(boost::posix_time::seconds(1));
                    }catch(...){
                        throw  exception::IException("ClusterService::start", "ClusterService can not be started");
                    }
                }
            }


            connection::Connection *ClusterService::getConnection(Address const & address) {
                if (!lifecycleService.isRunning()) {
                    throw exception::IException("ClusterService", "Instance is not active!");
                }
                connection::Connection *connection = NULL;
                connection = connectionManager.getConnection(address);
                if (connection != NULL) {
                    return connection;
                }
                beforeRetry();
                return getRandomConnection();
            };

            connection::Connection *ClusterService::getRandomConnection() {
                if (!lifecycleService.isRunning()) {
                    throw exception::IException("ClusterService", "Instance is not active!");
                }
                int retryCount = RETRY_COUNT;
                connection::Connection *connection = NULL;
                while (connection == NULL && retryCount > 0) {
                    connection = connectionManager.getRandomConnection();
                    if (connection == NULL) {
                        retryCount--;
                        beforeRetry();
                    }
                }
                if (connection == NULL) {
                    throw exception::IException("ClusterService", "Unable to connect!!!");
                }
                return connection;
            }

            connection::Connection *ClusterService::connectToOne(const std::vector<Address>& socketAddresses) {
                const int connectionAttemptLimit = clientConfig.getConnectionAttemptLimit();
                int attempt = 0;
                std::exception lastError;
                while (true) {
                    time_t tryStartTime = std::time(NULL);
                    std::vector<Address>::const_iterator it;
                    for (it = socketAddresses.begin(); it != socketAddresses.end(); it++) {
                        try {
                            return connectionManager.newConnection((*it));
                        } catch (exception::IOException & e) {
                            lastError = e;
                            std::cerr << "IO error  during initial connection..\n" << e.what() << std::endl;
                        } catch (exception::ServerException & e) {
                            lastError = e;
                            std::cerr << "IO error  during initial connection..\n" << e.what() << std::endl;

                        }
                    }
                    if (attempt++ >= connectionAttemptLimit) {
                        break;
                    }
                    const double remainingTime = clientConfig.getAttemptPeriod() - std::difftime(std::time(NULL), tryStartTime);
                    std::cerr << "Unable to get alive cluster connection, try in " << std::max(0.0, remainingTime)
                            << " ms later, attempt " << attempt << " of " << connectionAttemptLimit << "." << std::endl;

                    if (remainingTime > 0) {
                        boost::this_thread::sleep(boost::posix_time::milliseconds(remainingTime));
                    }
                }
                throw  exception::IException("ClusterService", "Unable to connect to any address in the config!" + std::string(lastError.what()));
            };


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

            void ClusterService::fireMembershipEvent(connection::MembershipEvent& event) {
                boost::lock_guard<boost::mutex> guard(listenerLock);
                for (std::set<MembershipListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    if (event.getEventType() == connection::MembershipEvent::MEMBER_ADDED) {
                        (*it)->memberAdded(event);
                    } else {
                        (*it)->memberRemoved(event);
                    }
                }
            };

            bool ClusterService::isMemberExists(Address const & address) {
                return membersRef->count(address) > 0;;
            };

            connection::Member ClusterService::getMember(const std::string& uuid) {
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
                if (membersRef == NULL)
                    return v;
                typename MemberMap::const_iterator it;
                boost::lock_guard<util::AtomicPointer< MemberMap > > lockGuard(membersRef);
                for (it = membersRef->begin(); it != membersRef->end(); it++) {
                    v.push_back(it->second);
                }
                return v;
            };

            void ClusterService::beforeRetry() {
                boost::this_thread::sleep(boost::posix_time::milliseconds(RETRY_WAIT_TIME));
                partitionService.refreshPartitions();
            }


        }
    }
}
