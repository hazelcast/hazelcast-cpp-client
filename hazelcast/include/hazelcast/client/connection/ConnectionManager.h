/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 5/21/13.

#ifndef HAZELCAST_CONNECTION_MANAGER
#define HAZELCAST_CONNECTION_MANAGER

#include "hazelcast/client/Address.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/connection/InSelector.h"
#include "hazelcast/client/connection/OutSelector.h"
#include "hazelcast/client/connection/OwnerConnectionFuture.h"
#include "hazelcast/client/connection/HeartBeater.h"
#include "hazelcast/client/protocol/Principal.h"
#include "hazelcast/util/Atomic.h"
#include "hazelcast/util/Thread.h"

#include <boost/shared_ptr.hpp>
#include <stdint.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class SocketInterceptor;

        namespace spi {
            class ClientContext;
        }

        namespace connection {
            class Connection;

            /**
            * Responsible for managing {@link com.hazelcast.client.connection.nio.ClientConnection} objects.
            */
            class HAZELCAST_API ConnectionManager {
            public:
                ConnectionManager(spi::ClientContext &clientContext, bool smartRouting);

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
                boost::shared_ptr<Connection> createOwnerConnection(const Address &address);

                /**
                * Gets a shared ptr to connection if available to given address
                *
                * @param address
                */
                boost::shared_ptr<Connection> getConnectionIfAvailable(const Address &address);

                /**
                * Gets a shared ptr to connection if available to the provided socket descriptor
                *
                * @param The socket descriptor for the connection.
                */
                boost::shared_ptr<Connection> getConnectionIfAvailable(int socketDescriptor);

                /**
                * Tries to connect to an address in member list.
                * Gets an address a hint first tries that if not successful, tries connections from LoadBalancer
                *
                * @param address hintAddress
                * @return authenticated connection
                * @throws Exception authentication failed or no connection found
                */
                boost::shared_ptr<Connection> getOrConnect(const Address &resolvedAddress, int tryCount);

                /**
                * Tries to connect to an address in member list.
                *
                * @param tryCount The number of times it shall try during connection establishment if not connected
                * @return authenticated connection
                * @throws Exception authentication failed or no connection found
                */
                boost::shared_ptr<Connection> getRandomConnection(int tryCount);

                /**
                * Tries to connect to an address in member list.
                *
                * This check is to guarantee that the same server is not retried. This may happen since the
                * cluster member update and the partition update may be received a little later and/or the load
                * balancer may produce the same address.
                *
                * @param tryCount The number of times it shall try during connection establishment if not connected
                * @param retryWaitTime The number of seconds to wait if the found random address is the same as the last
                *              tried address comes out to be the same as the randomly selected server address.
                * @return authenticated connection
                * @throws Exception authentication failed or no connection found
                */
                boost::shared_ptr<Connection> getRandomConnection(int tryCount, const std::string &lastTriedAddress,
                                                                  int retryWaitTime);

                /**
                * Called when an connection is closed.
                * Clears related resources of given clientConnection.
                *
                * @param clientConnection closed connection
                */
                void onConnectionClose(const Address &address, int socketId);

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
                * @return Return the newly created connection.
                */
                std::auto_ptr<Connection> connectTo(const Address &address, bool ownerConnection);

                /**
                * @param address
                * @param ownerConnection
                */
                std::vector<boost::shared_ptr<Connection> > getConnections();

                /**
                * Called heartbeat timeout is detected on a connection.
                *
                * @param connection to be marked.
                */
                void onDetectingUnresponsiveConnection(Connection &connection);

                /**
                 * Called when a member left the cluster
                 * @param address address of the member
                */
                void removeEndpoint(const Address &address);

                /**
                 *
                 * TODO: Keep the call id per connection inside the connection object and we may not need to use atomic
                 * int since only one connection io thread shall call it during the actual send operation!!!
                 */
                int64_t getNextCallId();

            private:

                boost::shared_ptr<Connection> getOrConnectResolved(const Address &resolvedAddress);

                boost::shared_ptr<Connection> getOrConnect(const Address &resolvedAddress);

                boost::shared_ptr<Connection> getRandomConnection();

                void authenticate(Connection *connection);

                void checkLive();

                void processSuccessfulAuthenticationResult(Connection *connection, std::auto_ptr<Address> addr,
                                                           std::auto_ptr<std::string> uuid,
                                                           std::auto_ptr<std::string> ownerUuid);

                boost::shared_ptr<Connection> getOwnerConnection();

                std::vector<byte> PROTOCOL;
                util::SynchronizedMap<Address, Connection, addressComparator> connections;
                util::SynchronizedMap<int, Connection> socketConnections;
                spi::ClientContext &clientContext;
                SocketInterceptor *socketInterceptor;
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

                util::Atomic<int64_t> callIdGenerator;
                util::Atomic<int> connectionIdCounter;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CONNECTION_MANAGER

