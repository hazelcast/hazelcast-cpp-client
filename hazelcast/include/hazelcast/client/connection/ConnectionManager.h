//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.

#ifndef HAZELCAST_CONNECTION_MANAGER
#define HAZELCAST_CONNECTION_MANAGER

#include "hazelcast/client/Address.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/connection/InSelector.h"
#include "hazelcast/client/connection/OutSelector.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/Future.h"

namespace hazelcast {
    namespace client {

        namespace serialization {
            namespace pimpl {
                class Data;
            }

        }

        namespace protocol {
            class Principal;

            class UsernamePasswordCredentials;
        }

        namespace spi {
            class ClientContext;
        }

        class MembershipListener;

        namespace connection {
            class Connection;

            class HAZELCAST_API ConnectionManager {
            public:
                ConnectionManager(spi::ClientContext &clientContext, bool smartRouting);

                bool start();

                connection::Connection *ownerConnection(const Address &address);

                boost::shared_ptr<Connection> getConnectionIfAvailable(const Address &address);

                boost::shared_ptr<Connection> getOrConnect(const Address &resolvedAddress, int tryCount);

                boost::shared_ptr<Connection> getRandomConnection(int tryCount);

                void removeConnection(const Address &address);

                void stop();

                int getNextCallId();

                void removeEventHandler(int callId);

            protected:
                connection::Connection *connectTo(const Address &address, bool reAuth);

                boost::shared_ptr<Connection> getOrConnectResolved(const Address &resolvedAddress);

                boost::shared_ptr<Connection> getOrConnect(const Address &resolvedAddress);

                boost::shared_ptr<Connection> getRandomConnection();

                void authenticate(Connection &connection, bool reAuth, bool firstConnection);

                void checkLive();

                std::vector<byte> PROTOCOL;
                util::SynchronizedMap<Address, Connection, addressComparator> connections;
                spi::ClientContext &clientContext;
                std::auto_ptr<SocketInterceptor> socketInterceptor;
                InSelector iListener;
                OutSelector oListener;
                std::auto_ptr<util::Thread> iListenerThread;
                std::auto_ptr<util::Thread> oListenerThread;
                util::AtomicBoolean live;
                util::Mutex lockMutex;
                boost::shared_ptr<protocol::Principal> principal;
                util::AtomicInt callIdGenerator;
                bool smartRouting;
                Address ownerConnectionAddress;

            };
        }
    }
}

#endif //HAZELCAST_CONNECTION_MANAGER
