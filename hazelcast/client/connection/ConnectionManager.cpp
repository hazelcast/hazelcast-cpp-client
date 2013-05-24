//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "ConnectionManager.h"
#include "../ClientConfig.h"
#include "../connection/Connection.h"
#include "../protocol/ClientPingRequest.h"
#include "../protocol/AuthenticationRequest.h"
#include "../protocol/HazelcastServerError.h"
#include "../serialization/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ConnectionManager::ConnectionManager(hazelcast::client::serialization::SerializationService& serializationService, hazelcast::client::ClientConfig& clientConfig)
            : serializationService(serializationService)
            , clientConfig(clientConfig)
            , heartBeatChecker(5, serializationService)
            , live(true)  //TODO get from config
            , principal(NULL) {

            };


            ConnectionManager::~ConnectionManager() {
                if (principal != NULL) {
                    delete principal;
                }
            };

            Connection *ConnectionManager::newConnection(Address const & address) {
                Connection *connection = new Connection(address, serializationService);
                authenticate(*connection, true);
                return connection;
            };

            Connection& ConnectionManager::getRandomConnection() {
                checkLive();
                Address& address = clientConfig.getAddresses().at(0);//TODO implement load balancer and stuff
//                    if (address == null) {
//                        throw new IOException("LoadBalancer '" + router + "' could not find a address to route to");
//                    }
                return getConnection(address);
            }

            Connection& ConnectionManager::getConnection(const Address& address) {
                checkLive();
                ConnectionPool *pool = getConnectionPool(address);
                Connection *connection = NULL;
//                try {
                connection = pool == NULL ? &getRandomConnection() : pool->take(this);
//                } catch (Exception e) {
//                    e.printStackTrace();
//                }
                // Could be that this address is dead and that's why pool is not able to create and give a connection.
                // We will call it again, and hopefully at some time LoadBalancer will give us the right target for the connection.
                if (connection == NULL) {
                    checkLive();
//                    try {
//                        Thread.sleep(1000);
//                    } catch (InterruptedException ignored) {
//                    }
                    return getRandomConnection();
                }
                if (!heartBeatChecker.checkHeartBeat(*connection)) {
                    connection->close();
                    return getRandomConnection();
                }
                return *connection;
            };

            void ConnectionManager::releaseConnection(Connection *connection) {
                if (live) {
                    ConnectionPool *pool = getConnectionPool(connection->getEndpoint());
                    if (pool != NULL) {
                        pool->release(connection);
                    } else {
                        connection->close();
                    }
                } else {
                    connection->close();
                }
            };

            ConnectionPool *ConnectionManager::getConnectionPool(const Address& address) {
                checkLive();
                ConnectionPool *pool = poolMap.get(address);
                if (pool == NULL) {
//                if (client.getClientClusterService().getMember(address) == null){
//                    return null;
//                }
                    pool = new ConnectionPool(address, serializationService);
                    ConnectionPool *current = poolMap.putIfAbsent(address, *pool);
                    if (current != NULL) {
                        delete pool;
                        return current;
                    }
                }
                return pool;
            }

            void ConnectionManager::authenticate(Connection& connection, bool reAuth) {
                connection.write(hazelcast::client::protocol::ProtocolConstants::PROTOCOL);
                hazelcast::client::protocol::AuthenticationRequest auth(clientConfig.getCredentials());
                auth.setPrincipal(principal);
                auth.setReAuth(reAuth);

                connection.write(serializationService.toData(auth));
                hazelcast::client::serialization::Data data;
                data.setSerializationContext(serializationService.getSerializationContext());
                connection.read(data);
                if (data.isServerError()) {
                    hazelcast::client::protocol::HazelcastServerError x = serializationService.toObject<hazelcast::client::protocol::HazelcastServerError>(data);
                    throw x;
                } else {
                    hazelcast::client::protocol::Principal *principal = new hazelcast::client::protocol::Principal();
                    *principal = serializationService.toObject<hazelcast::client::protocol::Principal>(data);
                    this->principal = principal;
                }
            };

            void ConnectionManager::checkLive() {
                if (!live) {
                    throw hazelcast::client::HazelcastException("Instance not active!");
                }
            }


        }
    }
}