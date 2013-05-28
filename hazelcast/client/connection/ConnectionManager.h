//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_CONNECTION_MANAGER
#define HAZELCAST_CONNECTION_MANAGER

#include "../../util/ConcurrentMap.h"
#include "HeartBeatChecker.h"
#include "ConnectionPool.h"

namespace hazelcast {
    namespace client {

        namespace serialization {
            class SerializationService;
        }

        namespace protocol {
            class Principal;

            class Credentials;
        }

        class ClientConfig;

        namespace connection {
            class Connection;

            class ConnectionManager {
            public:
                ConnectionManager(hazelcast::client::serialization::SerializationService&, hazelcast::client::ClientConfig&);

                ~ConnectionManager();

                Connection *newConnection(const Address& address);

                Connection& getRandomConnection();

                Connection& getConnection(const Address& address);

                void releaseConnection(Connection *connection);

                ConnectionPool *getConnectionPool(const Address& address);

                void authenticate(Connection& connection, bool reAuth);

            private:
                hazelcast::util::ConcurrentMap<Address, ConnectionPool > poolMap;
                hazelcast::client::serialization::SerializationService& serializationService;
                hazelcast::client::ClientConfig& clientConfig;
                hazelcast::client::protocol::Principal *principal;
                HeartBeatChecker heartBeatChecker;
                volatile bool live;

                void checkLive();

            };
        }
    }
}

#endif //HAZELCAST_CONNECTION_MANAGER
