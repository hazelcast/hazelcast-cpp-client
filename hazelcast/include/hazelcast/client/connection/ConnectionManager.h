//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.

#ifndef HAZELCAST_CONNECTION_MANAGER
#define HAZELCAST_CONNECTION_MANAGER

#include "hazelcast/client/connection/HeartBeatChecker.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/util/ConcurrentSmartMap.h"
#include "hazelcast/client/connection/SocketInterceptor.h"
#include <boost/atomic.hpp>

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

            class HAZELCAST_API ConnectionManager {
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

                virtual void shutdown();

                virtual void checkLive();

            protected:
                util::ConcurrentSmartMap<Address, ConnectionPool, addressComparator> poolMap;
                spi::ClusterService &clusterService;
                serialization::SerializationService &serializationService;
                ClientConfig &clientConfig;
                boost::shared_ptr<protocol::Principal> principal;
                HeartBeatChecker heartBeatChecker;
                std::auto_ptr<connection::SocketInterceptor> socketInterceptor;
                boost::atomic<bool> live;

            };
        }
    }
}

#endif //HAZELCAST_CONNECTION_MANAGER
