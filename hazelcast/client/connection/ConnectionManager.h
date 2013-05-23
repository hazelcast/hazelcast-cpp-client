//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_CONNECTION_MANAGER
#define HAZELCAST_CONNECTION_MANAGER

#include "../util/ConcurrentMap.h"
#include "../util/ConcurrentQueue.h"
#include "../Address.h"
#include "Connection.h"
#include "HeartBeatChecker.h"
#include "ConnectionPool.h"
#include "../protocol/Credentials.h"
#include "../protocol/Principal.h"

namespace hazelcast {
    namespace client {
        class serialization::SerializationService;

        class protocol::Credentials;

        class protocol::Principal;

        class ClientConfig;

        namespace connection {
            class Connection;

            class ConnectionManager {
            public:
                ConnectionManager(hazelcast::client::serialization::SerializationService&, hazelcast::client::ClientConfig&);

                Connection* newConnection(const Address& address);

                Connection& getRandomConnection();

                Connection& getConnection(const Address& address);

                ConnectionPool* getConnectionPool(const Address& address);

            private:
                hazelcast::client::util::ConcurrentMap<Address, ConnectionPool > poolMap;
                hazelcast::client::serialization::SerializationService& serializationService;
                hazelcast::client::ClientConfig& clientConfig;
                hazelcast::client::protocol::Principal principal;
                HeartBeatChecker heartBeatChecker;
                volatile bool live;

                void checkLive();

                void authenticate(Connection& connection, const hazelcast::client::protocol::Credentials& credentials, bool reAuth);
            };
        }
    }
}

#endif //HAZELCAST_CONNECTION_MANAGER
