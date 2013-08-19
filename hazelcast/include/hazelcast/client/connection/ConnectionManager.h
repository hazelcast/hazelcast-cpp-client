//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_CONNECTION_MANAGER
#define HAZELCAST_CONNECTION_MANAGER

#include "ConcurrentSmartMap.h"
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

        namespace spi {
            class ClusterService;
        }

        class MembershipListener;

        class ClientConfig;

        namespace connection {
            class Connection;

            class SocketInterceptor;

            class ConnectionManager {
            public:

                virtual Connection *newConnection(const Address& address) = 0;

                virtual Connection *getRandomConnection() = 0;

                virtual Connection *getConnection(const Address& address) = 0;

                virtual void releaseConnection(Connection *connection) = 0;

                virtual util::AtomicPointer <ConnectionPool> getConnectionPool(const Address& address) = 0;

                virtual void removeConnectionPool(const Address& address) = 0;

                virtual void authenticate(Connection& connection, bool reAuth, bool firstConnection) = 0;

            private:


            };
        }
    }
}

#endif //HAZELCAST_CONNECTION_MANAGER
