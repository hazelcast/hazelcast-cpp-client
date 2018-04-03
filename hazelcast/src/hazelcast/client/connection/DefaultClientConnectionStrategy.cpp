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

#include "hazelcast/client/connection/DefaultClientConnectionStrategy.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/config/ClientConnectionStrategyConfig.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/util/Executor.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            DefaultClientConnectionStrategy::DefaultClientConnectionStrategy(spi::ClientContext &clientContext,
                                                                             util::ILogger &logger,
                                                                             const config::ClientConnectionStrategyConfig &clientConnectionStrategyConfig)
                    : ClientConnectionStrategy(clientContext, logger, clientConnectionStrategyConfig) {
                executor = util::Executors::newSingleThreadExecutor(clientContext.getName() + ".clientShutdown-");
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
            }

            void DefaultClientConnectionStrategy::onDisconnectFromCluster() {
                disconnectedFromCluster = true;
                if (reconnectMode == config::ClientConnectionStrategyConfig::OFF) {
                    shutdownWithExternalThread();
                    return;
                }
                if (clientContext.getLifecycleService().isRunning()) {
                    try {
                        clientContext.getConnectionManager().connectToClusterAsync();
                    } catch (exception::RejectedExecutionException &) {
                        shutdownWithExternalThread();
                    }
                }
            }

            void DefaultClientConnectionStrategy::onConnect(const boost::shared_ptr<Connection> &connection) {
            }

            void DefaultClientConnectionStrategy::onDisconnect(const boost::shared_ptr<Connection> &connection) {
            }

            void DefaultClientConnectionStrategy::onHeartbeatStopped(const boost::shared_ptr<Connection> &connection) {

                if (connection->isAuthenticatedAsOwner()) {
                    boost::shared_ptr<exception::TargetDisconnectedException> exception = (
                            exception::ExceptionBuilder<exception::TargetDisconnectedException>(
                                    "DefaultClientConnectionStrategy::onHeartbeatStopped")
                                    << "Heartbeat timed out to owner connection " << *connection).buildShared();
                    connection->close("", exception);
                }
            }

            void DefaultClientConnectionStrategy::onHeartbeatResumed(const boost::shared_ptr<Connection> &connection) {
            }

            void DefaultClientConnectionStrategy::shutdown() {
            }

            bool DefaultClientConnectionStrategy::isClusterAvailable() const {
                return clientContext.getConnectionManager().getOwnerConnectionAddress().get() != NULL;
            }

            void DefaultClientConnectionStrategy::shutdownWithExternalThread() {
                executor->execute(boost::shared_ptr<util::Runnable>(new ShutdownTask(clientContext)));
            }

            DefaultClientConnectionStrategy::ShutdownTask::ShutdownTask(spi::ClientContext &clientContext)
                    : clientContext(clientContext) {}

            void DefaultClientConnectionStrategy::ShutdownTask::run() {
                try {
                    clientContext.getLifecycleService().shutdown();
                } catch (exception::IException &exception) {
                    util::ILogger::getLogger().severe() << "Exception during client shutdown " << exception;
                }
            }

            const std::string DefaultClientConnectionStrategy::ShutdownTask::getName() const {
                return "ShutdownTask";
            }
        }
    }
}

