/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/shared_ptr.hpp>
#include <stdint.h>
#include <memory>

#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"
#include "hazelcast/util/ConcurrentSet.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/util/Future.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/connection/InSelector.h"
#include "hazelcast/client/connection/OutSelector.h"
#include "hazelcast/client/protocol/Principal.h"
#include "hazelcast/client/internal/socket/SocketFactory.h"
#include "hazelcast/util/Atomic.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/client/connection/AddressTranslator.h"
#include "hazelcast/client/connection/ConnectionListenable.h"

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
            class ClientContext;

            namespace impl {
                class ClientExecutionServiceImpl;

                class ClientInvocationFuture;
            }
        }

        namespace connection {
            class Connection;

            class ConnectionListener;

            class AuthenticationFuture;

            class ClientConnectionStrategy;

            class AddressProvider;

            class HeartbeatManager;

            /**
            * Responsible for managing {@link Connection} objects.
            */
            class HAZELCAST_API ClientConnectionManagerImpl : public ConnectionListenable {
            public:
                ClientConnectionManagerImpl(spi::ClientContext &client,
                                            const boost::shared_ptr<AddressTranslator> &addressTranslator,
                                            const std::vector<boost::shared_ptr<AddressProvider> > &addressProviders);

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
                boost::shared_ptr<Connection> getOrConnect(const Address &address);

                /**
                 * @param address to be connected
                 * @return associated connection if available, returns null and triggers new connection creation otherwise
                 * @throws IOException if connection is not able to be triggered
                 */
                boost::shared_ptr<Connection> getOrTriggerConnect(const Address &target);

                /**
                 * Connects to the translated ip address (if translation is needed, such as when aws is used)
                * @param address
                * @return Return the newly created connection.
                */
                boost::shared_ptr<Connection> connectAsOwner(const Address &address);

                /**
                * @param address
                * @param ownerConnection
                */
                std::vector<boost::shared_ptr<Connection> > getActiveConnections();

                boost::shared_ptr<Address> getOwnerConnectionAddress();

                void setOwnerConnectionAddress(const boost::shared_ptr<Address> &ownerConnectionAddress);

                boost::shared_ptr<Connection> getActiveConnection(const Address &target);

                boost::shared_ptr<Connection> getActiveConnection(int fileDescriptor);

                boost::shared_ptr<Connection> getOwnerConnection();

                const boost::shared_ptr<protocol::Principal> getPrincipal();

                void setPrincipal(const boost::shared_ptr<protocol::Principal> &principal);

                void connectToCluster();

                boost::shared_ptr<util::Future<bool> > connectToClusterAsync();

                bool isAlive();

                void onClose(Connection &connection);

                virtual void addConnectionListener(const boost::shared_ptr<ConnectionListener> &connectionListener);

            private:
                static int DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC;
                static int DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC;

                boost::shared_ptr<Connection> getConnection(const Address &target, bool asOwner);

                boost::shared_ptr<AuthenticationFuture> triggerConnect(const Address &target, bool asOwner);

                boost::shared_ptr<Connection> createSocketConnection(const Address &address);

                boost::shared_ptr<Connection> getOrConnect(const Address &address, bool asOwner);

                void authenticate(const Address &target, boost::shared_ptr<Connection> &connection, bool asOwner,
                                  boost::shared_ptr<AuthenticationFuture> &future);

                std::auto_ptr<protocol::ClientMessage>
                encodeAuthenticationRequest(bool asOwner, serialization::pimpl::SerializationService &ss,
                                            const protocol::Principal *principal);

                void onAuthenticated(const Address &target, const boost::shared_ptr<Connection> &connection);

                void fireConnectionAddedEvent(const boost::shared_ptr<Connection> &connection);

                void removeFromActiveConnections(const boost::shared_ptr<Connection> &connection);

                void fireConnectionRemovedEvent(const boost::shared_ptr<Connection> &connection);

                void disconnectFromCluster(const boost::shared_ptr<Connection> &connection);

                boost::shared_ptr<util::impl::SimpleExecutorService>
                createSingleThreadExecutorService(spi::ClientContext &client);

                void fireConnectionEvent(const hazelcast::client::LifecycleEvent::LifeCycleState &state);

                void connectToClusterInternal();

                std::set<Address> getPossibleMemberAddresses();

                std::auto_ptr<ClientConnectionStrategy> initializeStrategy(spi::ClientContext &client);

                void startEventLoopGroup();

                void stopEventLoopGroup();

                class InitConnectionTask : public util::Runnable {
                public:
                    InitConnectionTask(const Address &target, const bool asOwner,
                                       const boost::shared_ptr<AuthenticationFuture> &future,
                                       ClientConnectionManagerImpl &connectionManager);

                    void run();

                    const std::string getName() const;

                private:
                    boost::shared_ptr<Connection> getConnection(const Address &target);

                    const Address target;
                    const bool asOwner;
                    boost::shared_ptr<AuthenticationFuture> future;
                    ClientConnectionManagerImpl &connectionManager;
                };

                class AuthCallback : public impl::ExecutionCallback<boost::shared_ptr<protocol::ClientMessage> > {
                public:
                    AuthCallback(const boost::shared_ptr<Connection> &connection, bool asOwner, const Address &target,
                                 boost::shared_ptr<AuthenticationFuture> &future,
                                 ClientConnectionManagerImpl &connectionManager);

                    virtual void onResponse(const boost::shared_ptr<protocol::ClientMessage> &response);

                    virtual void onFailure(const boost::shared_ptr<exception::IException> &e);

                private:
                    const boost::shared_ptr<Connection> connection;
                    bool asOwner;
                    Address target;
                    boost::shared_ptr<AuthenticationFuture> future;
                    ClientConnectionManagerImpl &connectionManager;

                    void onAuthenticationFailed(const Address &target, const boost::shared_ptr<Connection> &connection,
                                                const boost::shared_ptr<exception::IException> &cause);
                };

                class DisconnecFromClusterTask : public util::Runnable {
                public:
                    DisconnecFromClusterTask(const boost::shared_ptr<Connection> &connection,
                                             ClientConnectionManagerImpl &connectionManager,
                                             ClientConnectionStrategy &connectionStrategy);

                    virtual void run();

                    virtual const std::string getName() const;

                private:
                    const boost::shared_ptr<Connection> connection;
                    ClientConnectionManagerImpl &connectionManager;
                    ClientConnectionStrategy &connectionStrategy;
                };

                class ConnectToClusterTask : public util::Callable<bool> {
                public:
                    ConnectToClusterTask(ClientConnectionManagerImpl &connectionManager);

                    virtual bool call();

                    virtual const std::string getName() const;

                private:
                    ClientConnectionManagerImpl &connectionManager;
                };

                class ShutdownTask : public util::Runnable {
                public:
                    ShutdownTask(spi::ClientContext &clientContext);

                    virtual void run();

                    virtual const std::string getName() const;

                private:
                    spi::ClientContext &client;
                };

                class TimeoutAuthenticationTask : public util::Runnable {
                public:
                    TimeoutAuthenticationTask(const boost::shared_ptr<spi::impl::ClientInvocationFuture> &future,
                                              ClientConnectionManagerImpl &clientConnectionManager);

                    virtual void run();

                    virtual const std::string getName() const;

                private:
                    boost::shared_ptr<spi::impl::ClientInvocationFuture> future;
                    ClientConnectionManagerImpl &clientConnectionManager;
                };

                util::AtomicBoolean alive;

                util::ILogger &logger;
                int64_t connectionTimeoutMillis;

                spi::ClientContext &client;
                SocketInterceptor *socketInterceptor;
                util::SynchronizedMap<int, Connection> socketConnections;
                InSelector inSelector;
                OutSelector outSelector;
                util::Thread inSelectorThread;
                util::Thread outSelectorThread;

                spi::impl::ClientExecutionServiceImpl &executionService;

                boost::shared_ptr<AddressTranslator> translator;
                util::SynchronizedMap<Address, Connection> activeConnections;
                util::SynchronizedMap<int, Connection> activeConnectionsFileDescriptors;
                util::SynchronizedMap<int, Connection> pendingSocketIdToConnection;
                util::SynchronizedMap<Address, AuthenticationFuture> connectionsInProgress;
                // TODO: change with CopyOnWriteArraySet<ConnectionListener> as in Java
                util::ConcurrentSet<boost::shared_ptr<ConnectionListener> > connectionListeners;
                const Credentials *credentials;

                util::Atomic<boost::shared_ptr<Address> > ownerConnectionAddress;
                util::Atomic<boost::shared_ptr<Address> > previousOwnerConnectionAddress;

                util::Atomic<boost::shared_ptr<protocol::Principal> > principal;
                std::auto_ptr<ClientConnectionStrategy> connectionStrategy;
                boost::shared_ptr<util::impl::SimpleExecutorService> clusterConnectionExecutor;
                int32_t connectionAttemptPeriod;
                int32_t connectionAttemptLimit;
                bool shuffleMemberList;
                std::vector<boost::shared_ptr<AddressProvider> > addressProviders;

                util::Atomic<int> connectionIdGen;
                internal::socket::SocketFactory socketFactory;

                util::Mutex lock;
                std::auto_ptr<HeartbeatManager> heartbeat;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CONNECTION_MANAGER

