/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include <thread>

#include "hazelcast/client/connection/DefaultClientConnectionStrategy.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/config/ClientConnectionStrategyConfig.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/util/Executor.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            DefaultClientConnectionStrategy::DefaultClientConnectionStrategy(spi::ClientContext &clientContext,
                                                                             util::ILogger &logger,
                                                                             const config::ClientConnectionStrategyConfig &clientConnectionStrategyConfig)
                    : ClientConnectionStrategy(clientContext, logger, clientConnectionStrategyConfig) {
            }

            void DefaultClientConnectionStrategy::start() {
                clientStartAsync = clientConnectionStrategyConfig.isAsyncStart();
                reconnectMode = clientConnectionStrategyConfig.getReconnectMode();
                if (clientStartAsync) {
                    clientContext.getConnectionManager().connectToClusterAsync();
                } else {
                    clientContext.getConnectionManager().connectToCluster();
                }
            }

            void DefaultClientConnectionStrategy::beforeGetConnection(const Address &target) {
                if (isClusterAvailable()) {
                    return;
                }
                if (clientStartAsync && !disconnectedFromCluster) {
                    throw exception::HazelcastClientOfflineException(
                            "DefaultClientConnectionStrategy::beforeGetConnection", "Client is connecting to cluster.");
                }
                if (reconnectMode == config::ClientConnectionStrategyConfig::ASYNC && disconnectedFromCluster) {
                    throw exception::HazelcastClientOfflineException(
                            "DefaultClientConnectionStrategy::beforeGetConnection", "Client is offline.");
                }
            }

            void DefaultClientConnectionStrategy::beforeOpenConnection(const Address &target) {
                if (isClusterAvailable()) {
                    return;
                }
                if (reconnectMode == config::ClientConnectionStrategyConfig::ASYNC && disconnectedFromCluster) {
                    throw exception::HazelcastClientOfflineException(
                            "DefaultClientConnectionStrategy::beforeGetConnection", "Client is offline");
                }
            }

            void DefaultClientConnectionStrategy::onConnectToCluster() {
                disconnectedFromCluster.store(false);
            }

            void DefaultClientConnectionStrategy::onDisconnectFromCluster() {
                disconnectedFromCluster.store(true);
                if (reconnectMode == config::ClientConnectionStrategyConfig::OFF) {
                    shutdownWithExternalThread(clientContext.getHazelcastClientImplementation());
                    return;
                }
                if (clientContext.getLifecycleService().isRunning()) {
                    try {
                        clientContext.getConnectionManager().connectToClusterAsync();
                    } catch (exception::RejectedExecutionException &) {
                        shutdownWithExternalThread(clientContext.getHazelcastClientImplementation());
                    }
                }
            }

            void DefaultClientConnectionStrategy::onConnect(const std::shared_ptr<Connection> &connection) {
            }

            void DefaultClientConnectionStrategy::onDisconnect(const std::shared_ptr<Connection> &connection) {
            }

            void DefaultClientConnectionStrategy::shutdown() {
            }

            bool DefaultClientConnectionStrategy::isClusterAvailable() const {
                return clientContext.getConnectionManager().getOwnerConnectionAddress().get() != NULL;
            }

            void
            DefaultClientConnectionStrategy::shutdownWithExternalThread(
                    std::weak_ptr<client::impl::HazelcastClientInstanceImpl> clientImpl) {

                std::thread shutdownThread([=] {
                    std::shared_ptr<client::impl::HazelcastClientInstanceImpl> clientInstance = clientImpl.lock();
                    if (!clientInstance.get() || !clientInstance->getLifecycleService().isRunning()) {
                        return;
                    }

                    try {
                        clientInstance->getLifecycleService().shutdown();
                    } catch (exception::IException &exception) {
                        clientInstance->getLogger()->severe() << "Exception during client shutdown task "
                                                              << clientInstance->getName() + ".clientShutdown-" << ":"
                                                              << exception;
                    }
                });

                shutdownThread.detach();
            }
        }
    }
}

