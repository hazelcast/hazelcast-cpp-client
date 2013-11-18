//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.

#ifndef HAZELCAST_CONNECTION_MANAGER
#define HAZELCAST_CONNECTION_MANAGER

#include "HeartBeatChecker.h"
#include "Address.h"
#include "ConcurrentSmartMap.h"
#include "SocketInterceptor.h"
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

            class ConnectionPool;

            class ConnectionManager {
            public:
                ConnectionManager(spi::ClusterService &clusterService, serialization::SerializationService &serializationService, ClientConfig &clientConfig);

                ~ConnectionManager();

                virtual Connection *newConnection(const Address &address);

                virtual Connection *firstConnection(const Address &address);

                virtual Connection *getConnection(const Address &address);

                virtual Connection *getRandomConnection();

                virtual void releaseConnection(Connection *connection);

                virtual util::AtomicPointer <ConnectionPool> getConnectionPool(const Address &address);

                virtual void removeConnectionPool(const Address &address);

                virtual void authenticate(Connection &connection, bool reAuth, bool firstConnection);

            protected:
                util::ConcurrentSmartMap<Address, ConnectionPool, addressComparator> poolMap;
                spi::ClusterService &clusterService;
                serialization::SerializationService &serializationService;
                ClientConfig &clientConfig;
                protocol::Principal *principal;
                HeartBeatChecker heartBeatChecker;
                std::auto_ptr<connection::SocketInterceptor> socketInterceptor;


            };
        }
    }
}

#endif //HAZELCAST_CONNECTION_MANAGER
