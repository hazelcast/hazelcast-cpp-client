/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include <atomic>
#include <stdint.h>
#include <memory>
#include <random>
#include <thread>
#include <future>
#include <vector>
#include <boost/asio.hpp>

#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/util/ConcurrentSet.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/protocol/Principal.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/internal/socket/SocketFactory.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/client/connection/AddressTranslator.h"
#include "hazelcast/client/connection/ConnectionListenable.h"
#include "hazelcast/client/connection/HeartbeatManager.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#pragma warning(disable: 4003) //for  not enough actual parameters for macro 'min' in asio wait_traits
#endif

namespace hazelcast {
    namespace client {
        class Credentials;

        namespace protocol {
            class ClientMessage;
        }

        class SocketInterceptor;

        namespace spi {
            namespace impl {
                class ClientExecutionServiceImpl;
            }
        }

        namespace impl {
            class HazelcastClientInstanceImpl;
        }

        namespace connection {
            class Connection;

            class ConnectionListener;

            class AuthenticationFuture;

            class ClientConnectionStrategy;

            class AddressProvider;

            /**
            * Responsible for managing {@link Connection} objects.
            */
            class HAZELCAST_API ClientConnectionManagerImpl
                    : public ConnectionListenable, public std::enable_shared_from_this<ClientConnectionManagerImpl> {
            public:
                typedef std::tuple<std::shared_ptr<AuthenticationFuture>, std::shared_ptr<Connection>> FutureTuple;

                ClientConnectionManagerImpl(spi::ClientContext &client,
                                            const std::shared_ptr<AddressTranslator> &addressTranslator,
                                            const std::vector<std::shared_ptr<AddressProvider> > &addressProviders);

                virtual ~ClientConnectionManagerImpl();

                /**
                * Start clientConnectionManager
                */
                bool start();

                /**
                * Shutdown clientConnectionManager. It does not throw any excpetion.
                */
                void shutdown();

                /**
                 * @param address to be connected
                 * @return associated connection if available, creates new connection otherwise
                 * @throws IOException if connection is not established
                 */
                std::shared_ptr<Connection> getOrConnect(const Address &address);

                /**
                 * @param address to be connected
                 * @return associated connection if available, returns null and triggers new connection creation otherwise
                 * @throws IOException if connection is not able to be triggered
                 */
                std::shared_ptr<Connection> getOrTriggerConnect(const Address &target);

                /**
                 * Connects to the translated ip address (if translation is needed, such as when aws is used)
                * @param address
                * @return Return the newly created connection.
                */
                std::shared_ptr<Connection> connectAsOwner(const Address &address);

                /**
                * @param address
                * @param ownerConnection
                */
                std::vector<std::shared_ptr<Connection> > getActiveConnections();

                std::shared_ptr<Address> getOwnerConnectionAddress();

                void setOwnerConnectionAddress(const std::shared_ptr<Address> &ownerAddress);

                std::shared_ptr<Connection> getActiveConnection(const Address &target);

                std::shared_ptr<Connection> getOwnerConnection();

                const std::shared_ptr<protocol::Principal> getPrincipal();

                void setPrincipal(const std::shared_ptr<protocol::Principal> &p);

                void connectToCluster();

                std::future<bool> connectToClusterAsync();

                bool isAlive();

                void onClose(Connection &connection);

                virtual void addConnectionListener(const std::shared_ptr<ConnectionListener> &connectionListener);

                util::ILogger &getLogger();

                void authenticate(const Address &target, std::shared_ptr<Connection> &connection, bool asOwner,
                                  std::shared_ptr<AuthenticationFuture> &future);

                void reAuthenticate(const Address &target, std::shared_ptr<Connection> &connection, bool asOwner,
                                    std::shared_ptr<AuthenticationFuture> &future);

            private:
                static int DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC;
                static int DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC;

                std::shared_ptr<Connection> getConnection(const Address &target, bool asOwner);

                std::shared_ptr<FutureTuple> triggerConnect(const Address &target, bool asOwner);

                std::shared_ptr<Connection> getOrConnect(const Address &address, bool asOwner);

                std::unique_ptr<protocol::ClientMessage>
                encodeAuthenticationRequest(bool asOwner, serialization::pimpl::SerializationService &ss,
                                            const protocol::Principal *p);

                void onAuthenticated(const Address &target, const std::shared_ptr<Connection> &connection);

                void fireConnectionAddedEvent(const std::shared_ptr<Connection> &connection);

                void removeFromActiveConnections(const std::shared_ptr<Connection> &connection);

                void fireConnectionRemovedEvent(const std::shared_ptr<Connection> &connection);

                void disconnectFromCluster(const std::shared_ptr<Connection> connection);

                void fireConnectionEvent(const hazelcast::client::LifecycleEvent::LifeCycleState &state);

                void connectToClusterInternal();

                std::set<Address> getPossibleMemberAddresses();

                std::unique_ptr<ClientConnectionStrategy> initializeStrategy(spi::ClientContext &context);

                void shuffle(std::vector<Address> &memberAddresses) const;

                class AuthCallback {
                public:
                    AuthCallback(const std::shared_ptr<Connection> &connection, bool asOwner, const Address &target,
                                 std::shared_ptr<AuthenticationFuture> &f,
                                 ClientConnectionManagerImpl &connectionManager);

                    virtual ~AuthCallback();

                    virtual void onResponse(protocol::ClientMessage response);

                    virtual void onFailure(std::exception_ptr e);

                private:
                    const std::shared_ptr<Connection> connection;
                    bool asOwner;
                    Address target;
                    std::shared_ptr<AuthenticationFuture> authFuture;
                    std::shared_ptr<ClientConnectionManagerImpl> connectionManager;
                    std::shared_ptr<boost::asio::steady_timer> timeoutTimer;

                    void onAuthenticationFailed(const Address &target, const std::shared_ptr<Connection> &connection,
                                                std::exception_ptr cause);

                    virtual void handleAuthenticationException(std::exception_ptr e);

                    void cancelTimeoutTask();

                    void scheduleTimeoutTask();
                };

                util::AtomicBoolean alive;

                util::ILogger &logger;
                std::chrono::steady_clock::duration connectionTimeoutMillis;

                spi::ClientContext &client;
                SocketInterceptor *socketInterceptor;

                spi::impl::ClientExecutionServiceImpl &executionService;

                std::shared_ptr<AddressTranslator> translator;
                util::SynchronizedMap<Address, Connection> activeConnections;
                util::SynchronizedMap<Address, FutureTuple> connectionsInProgress;
                // TODO: change with CopyOnWriteArraySet<ConnectionListener> as in Java
                util::ConcurrentSet<std::shared_ptr<ConnectionListener> > connectionListeners;
                const Credentials *credentials;

                util::Sync<std::shared_ptr<Address> > ownerConnectionAddress;
                util::Sync<std::shared_ptr<Address> > previousOwnerConnectionAddress;

                util::Sync<std::shared_ptr<protocol::Principal> > principal;
                std::unique_ptr<ClientConnectionStrategy> connectionStrategy;
                boost::asio::thread_pool clusterConnectionExecutor;
                int32_t connectionAttemptPeriod;
                int32_t connectionAttemptLimit;
                int32_t ioThreadCount;
                bool shuffleMemberList;
                std::vector<std::shared_ptr<AddressProvider> > addressProviders;

                std::atomic<int> connectionIdGen;
                internal::socket::SocketFactory socketFactory;

                std::mutex lock;
                HeartbeatManager heartbeat;

                boost::asio::io_context ioContext;
                std::vector<std::thread> ioThreads;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CONNECTION_MANAGER

