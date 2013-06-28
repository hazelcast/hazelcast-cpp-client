//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "ClusterService.h"
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
            , clusterThread(connectionManager, clientConfig, *this, lifecycleService , serializationService)
            , credentials(clientConfig.getCredentials())
            , redoOperation(clientConfig.isRedoOperation()) {

            }

            void ClusterService::start() {
                serialization::ClassDefinitionBuilder cd(-3, 3);
                serialization::ClassDefinition *ptr = cd.addUTFField("uuid").addUTFField("ownerUuid").build();
                serializationService.getSerializationContext().registerClassDefinition(ptr);

                connection::Connection *connection = connectToOne(clientConfig.getAddresses());
                clusterThread.setInitialConnection(connection);
                clusterThread.start();
                while (membersRef.get() == NULL) {
                    try {
                        sleep(1);
                    } catch (void* ) {
                        throw  HazelcastException("ClusterService::start");
                    }
                }
            }


            connection::Connection *ClusterService::getConnection(Address const & address) {
                if (!lifecycleService.isRunning()) {
                    throw HazelcastException("Instance is not active!");
                }
                connection::Connection *connection = NULL;
                int retryCount = clientConfig.getConnectionAttemptLimit();
                while (connection == NULL && retryCount > 0) {
                    connection = connectionManager.getConnection(address);
                    if (connection == NULL) {
                        retryCount--;
                        usleep(clientConfig.getAttemptPeriod() * 1000);
                    }
                }
                if (connection == NULL) {
                    throw HazelcastException("Unable to connect!!!");
                }
                return connection;
            };

            connection::Connection *ClusterService::getRandomConnection() {
                if (!lifecycleService.isRunning()) {
                    throw HazelcastException("HazelcastInstanceNotActiveException")/*HazelcastInstanceNotActiveException()*/;
                }
                connection::Connection *connection = NULL;
                int retryCount = clientConfig.getConnectionAttemptLimit();
                while (connection == NULL && retryCount > 0) {
                    connection = connectionManager.getRandomConnection();
                    if (connection == NULL) {
                        retryCount--;
                        usleep(clientConfig.getAttemptPeriod() * 1000);
                    }
                }
                if (connection == NULL) {
                    throw HazelcastException("Unable to connect!!!");
                }
                return connection;
            };

            connection::Connection *ClusterService::connectToOne(const std::vector<Address>& socketAddresses) {
                const int connectionAttemptLimit = clientConfig.getConnectionAttemptLimit();
                int attempt = 0;
                while (true) {
                    time_t tryStartTime = std::time(NULL);
                    std::vector<Address>::const_iterator it;
                    for (it = socketAddresses.begin(); it != socketAddresses.end(); it++) {
                        try {
                            std::cout << "Trying to connect: " + (*it).getHost() + ":" + util::to_string((*it).getPort()) << std::endl;
                            return connectionManager.newConnection((*it));
                        } catch (HazelcastException& ignored) {
                        }
                    }
                    if (attempt++ >= connectionAttemptLimit) {
                        break;
                    }
                    const double remainingTime = clientConfig.getAttemptPeriod() - std::difftime(std::time(NULL), tryStartTime);
                    std::cerr << "Unable to get alive cluster connection, try in " << std::max(0.0, remainingTime)
                            << " ms later, attempt " << attempt << " of " << connectionAttemptLimit << "." << std::endl;

                    if (remainingTime > 0) {
                        sleep(remainingTime);
                    }
                }
                throw  HazelcastException("Unable to connect to any address in the config!");
            };


            Address ClusterService::getMasterAddress() {
                vector<connection::Member> list = getMemberList();
                return list[0].getAddress();
            }

            void ClusterService::addMembershipListener(MembershipListener *listener) {
                listenerLock.lock();
                listeners.insert(listener);
                listenerLock.unlock();
            };

            bool ClusterService::removeMembershipListener(MembershipListener *listener) {
                listenerLock.lock();
                bool b = listeners.erase(listener) == 1;
                listenerLock.unlock();
                return b;
            };

            void ClusterService::fireMembershipEvent(connection::MembershipEvent& event) {
                listenerLock.lock();
                for (std::set<MembershipListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    if (event.getEventType() == connection::MembershipEvent::MEMBER_ADDED) {
                        (*it)->memberAdded(event);
                    } else {
                        (*it)->memberRemoved(event);
                    }
                }
                listenerLock.unlock();
            };

            bool ClusterService::isMemberExists(Address const & address) {
                std::map<Address, connection::Member> *pMap = membersRef.get();
                return pMap->count(address) > 0;;
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

            vector<connection::Member>  ClusterService::getMemberList() {
                return util::values(membersRef.get());
            };

        }
    }
}
