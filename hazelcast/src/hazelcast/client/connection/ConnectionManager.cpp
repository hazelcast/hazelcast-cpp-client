//
// Created by sancar koyunlu on 8/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/ConnectionPool.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/protocol/AuthenticationRequest.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ConnectionManager::ConnectionManager(spi::ClusterService &clusterService, serialization::SerializationService &serializationService, ClientConfig &clientConfig)
            : clusterService(clusterService)
            , serializationService(serializationService)
            , clientConfig(clientConfig)
            , heartBeatChecker(clientConfig.getConnectionTimeout(), serializationService)
            , socketInterceptor(this->clientConfig.getSocketInterceptor())
            , live(true) {

            };


            ConnectionManager::~ConnectionManager() {
            };

            Connection *ConnectionManager::newConnection(Address const &address) {
                checkLive();
                Connection *connection = new Connection(address, serializationService);
                authenticate(*connection, true, true);
                return connection;
            };

            Connection *ConnectionManager::firstConnection(const Address &address) {
                return newConnection(address);
            }

            Connection *ConnectionManager::getConnection(const Address &address) {
                checkLive();
                util::AtomicPointer<ConnectionPool> pool = getConnectionPool(address);
                if (pool.isNull())
                    return NULL;
                Connection *connection = NULL;
                connection = pool->take();
                if (connection != NULL && !heartBeatChecker.checkHeartBeat(*connection)) {
                    std::cerr << "Closing connection : " << *connection << " reason => Member is not responding to heartbeat";
                    delete connection;
                    return NULL;
                }
                return connection;
            };


            Connection *ConnectionManager::getRandomConnection() {
                checkLive();
                const Address &address = clientConfig.getLoadBalancer()->next().getAddress();
                return getConnection(address);
            }

            void ConnectionManager::releaseConnection(Connection *connection) {
                util::AtomicPointer<ConnectionPool> pool = getConnectionPool(connection->getEndpoint());
                if (!pool.isNull()) {
                    pool->release(connection);
                } else {
                    std::cerr << "Closing connection : " << *connection << " reason => Member pool is removed";
                    delete connection;
                }
            };

            util::AtomicPointer <ConnectionPool> ConnectionManager::getConnectionPool(const Address &address) {
                checkLive();
                util::AtomicPointer<ConnectionPool> pool = poolMap.get(address);
                if (!pool.isNull()) {
                    return pool;
                }
                if (!clusterService.isMemberExists(address)) {
                    util::AtomicPointer<ConnectionPool> x;
                    return x;
                }
                util::AtomicPointer<ConnectionPool> np(new ConnectionPool(address, serializationService, *this), address.hashCode());

                util::AtomicPointer<ConnectionPool> previousPool = poolMap.putIfAbsent(address, np);
                if (previousPool.isNull()) {
                    return np;
                } else {
                    return previousPool;
                }
            };

            void ConnectionManager::removeConnectionPool(const Address &address) {
                poolMap.remove(address);
            };

            void ConnectionManager::authenticate(Connection &connection, bool reAuth, bool firstConnection) {
                checkLive();
                connection.connect();
                connection.write(protocol::ProtocolConstants::PROTOCOL);
                if (socketInterceptor.get() != NULL) {
                    socketInterceptor.get()->onConnect(connection.getSocket());
                }
                protocol::AuthenticationRequest auth(clientConfig.getCredentials());
                auth.setPrincipal(principal.get());
                auth.setReAuth(reAuth);
                auth.setFirstConnection(firstConnection);

                serialization::Data toData = serializationService.toData<protocol::AuthenticationRequest>(&auth);
                connection.write(toData);
                serialization::Data data1 = connection.read();
                boost::shared_ptr<Address> address = serializationService.toObject<Address>(data1);
                connection.setEndpoint(*address);
                serialization::Data data2 = connection.read();

                this->principal = serializationService.toObject<protocol::Principal>(data2);
            };


            void ConnectionManager::shutdown() {
                live = false;
            }

            void ConnectionManager::checkLive() {
                if (!live) {
                    throw exception::InstanceNotActiveException("ConnectionManager::checkLive", "Instance is not active");
                }
            }
        }
    }
}