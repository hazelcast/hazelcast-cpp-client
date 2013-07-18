//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/protocol/AuthenticationRequest.h"
#include "ServerException.h"
#include "hazelcast/client/spi/ClusterService.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ConnectionManager::ConnectionManager(spi::ClusterService& clusterService, serialization::SerializationService& serializationService, ClientConfig& clientConfig)
            : clusterService(clusterService)
            , serializationService(serializationService)
            , clientConfig(clientConfig)
            , heartBeatChecker(clientConfig.getConnectionTimeout(), serializationService)
            , live(true)
            , principal(NULL) {

            };


            ConnectionManager::~ConnectionManager() {
                if (principal != NULL) {
                    delete principal;
                }
            };

            Connection *ConnectionManager::newConnection(Address const & address) {
                Connection *connection = new Connection(address, serializationService);
                authenticate(*connection, true, true);
                return connection;
            };

            Connection *ConnectionManager::getRandomConnection() {
                checkLive();
                const Address& address = clientConfig.getLoadBalancer()->next().getAddress();
                return getConnection(address);
            }

            Connection *ConnectionManager::getConnection(const Address& address) {
                checkLive();
                ConnectionPool *pool = getConnectionPool(address);
                if (pool == NULL )
                    return NULL;
                Connection *connection = NULL;
                connection = pool->take();
                if (connection != NULL && !heartBeatChecker.checkHeartBeat(*connection)) {
                    delete connection;
                    return NULL;
                }
                return connection;
            };

            void ConnectionManager::releaseConnection(Connection *connection) {
                if (live) {
                    ConnectionPool *pool = getConnectionPool(connection->getEndpoint());
                    if (pool != NULL) {
                        pool->release(connection);
                    } else {
                        delete connection;
                    }
                } else {
                    delete connection;
                }
            };

            ConnectionPool *ConnectionManager::getConnectionPool(const Address& address) {
                checkLive();
//                std::cout << "get address " << address << std::endl;
                ConnectionPool *pool = poolMap.get(address);
                if (pool == NULL) {
//                    std::cout << "get address NULL " << std::endl;
                    if (!clusterService.isMemberExists(address)) {
                        return NULL;
                    }
                    pool = new ConnectionPool(address, serializationService, *this);

                    ConnectionPool *previousPool = poolMap.putIfAbsent(address, pool);
                    if (previousPool) delete pool;
                    return previousPool == NULL ? pool : previousPool;
                }
//                std::cout << "getted pool address " << pool->address << std::endl;
                return pool;
            };

            void ConnectionManager::removeConnectionPool(const Address &address) {
                ConnectionPool *pool = poolMap.remove(address);
                if (pool != NULL) {
                    pool->destroy();
                    delete pool;
                }
            };

            void ConnectionManager::authenticate(Connection& connection, bool reAuth, bool firstConnection) {
                connection.connect();
                connection.write(protocol::ProtocolConstants::PROTOCOL);
                protocol::AuthenticationRequest auth(clientConfig.getCredentials());
                auth.setPrincipal(principal);
                auth.setReAuth(reAuth);
                auth.setFirstConnection(firstConnection);

                serialization::Data toData = serializationService.toData<protocol::AuthenticationRequest>(&auth);
                connection.write(toData);
                serialization::Data data1 = connection.read(serializationService.getSerializationContext());
                Address address = serializationService.toObject<Address>(data1);
                connection.setEndpoint(address);
                serialization::Data data2 = connection.read(serializationService.getSerializationContext());

                this->principal = new protocol::Principal(serializationService.toObject<protocol::Principal>(data2));
            };

            void ConnectionManager::checkLive() {
                if (!live) {
                    throw exception::IException("ConnectionManager::checkLive()", "Instance not active!");
                }
            };


            void ConnectionManager::shutdown() {
                live = false;
                poolMap.clear();
            };
        }
    }
}