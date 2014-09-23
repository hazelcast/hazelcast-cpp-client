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
#include "hazelcast/client/connection/OwnerConnectionFuture.h"
#include "hazelcast/client/connection/HeartBeater.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/Future.h"
#include <boost/shared_ptr.hpp>

namespace hazelcast {

    namespace util {
        class CountDownLatch;
    }

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

            /**
            * Responsible for managing {@link com.hazelcast.client.connection.nio.ClientConnection} objects.
            */
            class HAZELCAST_API ConnectionManager{
            public:
                ConnectionManager(spi::ClientContext& clientContext, bool smartRouting);              

                /**
                * Start clientConnectionManager
                */
                bool start();

                /**
                * Creates a new owner connection to given address
                *
                * @param address to be connection to established
                * @return ownerConnection
                * @throws Exception
                */
                boost::shared_ptr<Connection> createOwnerConnection(const Address& address);

                /**
                * Gets a shared ptr to connection if available to given address
                *
                * @param address
                */
                boost::shared_ptr<Connection> getConnectionIfAvailable(const Address& address);

                /**
                * Tries to connect to an address in member list.
                * Gets an address a hint first tries that if not successful, tries connections from LoadBalancer
                *
                * @param address hintAddress
                * @return authenticated connection
                * @throws Exception authentication failed or no connection found
                */
                boost::shared_ptr<Connection> getOrConnect(const Address& resolvedAddress, int tryCount);

                /**
                * Tries to connect to an address in member list.
                *
                * @return authenticated connection
                * @throws Exception authentication failed or no connection found
                */
                boost::shared_ptr<Connection> getRandomConnection(int tryCount);

                /**
                * Called when an connection is closed.
                * Clears related resources of given clientConnection.
                *
                * @param clientConnection closed connection
                */
                void onConnectionClose(const Address& address);

                /**
                * Shutdown clientConnectionManager
                */
                void shutdown();

                /**
                * Called when an owner connection is closed
                */
                void onCloseOwnerConnection();

                /**
                * @param address
                * @param ownerConnection
                */
                connection::Connection *connectTo(const Address& address, bool ownerConnection);

                /**
                * @param address
                * @param ownerConnection
                */
                std::vector< boost::shared_ptr<Connection> > getConnections();

                /**
                * Called heartbeat timeout is detected on a connection.
                *
                * @param connection to be marked.
                */
                void onDetectingUnresponsiveConnection(Connection& connection);

                /**
                 * Called when a member left the cluster
                 * @param address address of the member
                */
                void removeEndpoint(const Address& address);
            private:

                boost::shared_ptr<Connection> getOrConnectResolved(const Address& resolvedAddress);

                boost::shared_ptr<Connection> getOrConnect(const Address& resolvedAddress);

                boost::shared_ptr<Connection> getRandomConnection();

                void authenticate(Connection& connection, bool firstConnection);

                void checkLive();

                std::vector<byte> PROTOCOL;
                util::SynchronizedMap<Address, Connection, addressComparator> connections;
                spi::ClientContext& clientContext;
                std::auto_ptr<SocketInterceptor> socketInterceptor;
                InSelector inSelector;
                OutSelector outSelector;
                std::auto_ptr<util::Thread> inSelectorThread;
                std::auto_ptr<util::Thread> outSelectorThread;
                util::AtomicBoolean live;
                util::Mutex lockMutex;
                boost::shared_ptr<protocol::Principal> principal;

                connection::HeartBeater heartBeater;
                std::auto_ptr<util::Thread> heartBeatThread;
                /** Can be separated via inheritance as Dumb ConnectionManager**/
                bool smartRouting;
                OwnerConnectionFuture ownerConnectionFuture;
            };
        }
    }
}

#endif //HAZELCAST_CONNECTION_MANAGER

