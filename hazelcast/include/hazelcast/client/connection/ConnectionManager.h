//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.

#ifndef HAZELCAST_CONNECTION_MANAGER
#define HAZELCAST_CONNECTION_MANAGER

#include "hazelcast/client/connection/HeartBeatChecker.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/connection/SocketInterceptor.h"
#include "hazelcast/client/connection/IListener.h"
#include "hazelcast/client/connection/OListener.h"
#include <boost/atomic.hpp>
#include <boost/thread/future.hpp>

namespace hazelcast {
    namespace client {

        namespace serialization {
            class Data;
        }

        namespace protocol {
            class Principal;

            class Credentials;
        }

        namespace spi {
            class ClientContext;
        }

        class MembershipListener;

        namespace connection {
            class Connection;

            class ConnectionPool;

            class HAZELCAST_API ConnectionManager {
            public:
                ConnectionManager(spi::ClientContext &clientContext, bool smartRouting);

                ~ConnectionManager();

                void start();

                connection::Connection *ownerConnection(const Address &address);

                boost::shared_ptr<Connection> getOrConnect(const Address &resolvedAddress);

                boost::shared_ptr<Connection> getRandomConnection();

                void authenticate(Connection &connection, bool reAuth, bool firstConnection);

                void shutdown();

                void checkLive();

                int getNextCallId();

                void removeEventHandler(int callId);

            protected:
                connection::Connection *connectTo(const Address &address);

                boost::shared_ptr<Connection> getOrConnectResolved(const Address &resolvedAddress);

                util::SynchronizedMap<Address, Connection, addressComparator> connections;
                spi::ClientContext &clientContext;
                std::auto_ptr<connection::SocketInterceptor> socketInterceptor;
                IListener iListener;
                OListener oListener;
                std::auto_ptr<boost::thread> iListenerThread;
                std::auto_ptr<boost::thread> oListenerThread;
                boost::atomic<bool> live;
                boost::mutex lockMutex;
                boost::shared_ptr<protocol::Principal> principal;
                boost::atomic<long> callIdGenerator;
                bool smartRouting;
                Address ownerConnectionAddress;

            };
        }
    }
}

#endif //HAZELCAST_CONNECTION_MANAGER
