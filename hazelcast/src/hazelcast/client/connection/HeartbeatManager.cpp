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
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/protocol/codec/ClientPingCodec.h"
#include "hazelcast/client/connection/HeartbeatManager.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/impl/ClientInvocationFuture.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            HeartbeatManager::HeartbeatManager(spi::ClientContext &client) : client(client), clientConnectionManager(
                    client.getConnectionManager()), logger(util::ILogger::getLogger()) {
                ClientProperties &clientProperties = client.getClientProperties();
                int timeoutSeconds = clientProperties.getHeartbeatTimeout().getInteger();
                heartbeatTimeout = timeoutSeconds > 0 ? timeoutSeconds * 1000 : util::IOUtil::to_value<int>(
                        (std::string) ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT) * 1000;

                int intervalSeconds = clientProperties.getHeartbeatInterval().getInteger();
                heartbeatInterval = intervalSeconds > 0 ? intervalSeconds * 1000 : util::IOUtil::to_value<int>(
                        (std::string) ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT) * 1000;
            }

            void HeartbeatManager::start() {
                spi::impl::ClientExecutionServiceImpl &clientExecutionService = client.getClientExecutionService();

                clientExecutionService.scheduleWithRepetition(
                        boost::shared_ptr<util::Runnable>(new util::RunnableDelegator(*this)), heartbeatInterval,
                        heartbeatInterval);
            }

            void HeartbeatManager::run() {
                if (!clientConnectionManager.isAlive()) {
                    return;
                }

                int64_t now = util::currentTimeMillis();
                BOOST_FOREACH (boost::shared_ptr<Connection> connection,
                               clientConnectionManager.getActiveConnections()) {
                                checkConnection(now, connection);
                            }
            }

            const std::string HeartbeatManager::getName() const {
                return "HeartbeatManager";
            }

            void HeartbeatManager::checkConnection(int64_t now, boost::shared_ptr<Connection> &connection) {
                if (!connection->isAlive()) {
                    return;
                }

                if (now - connection->lastReadTimeMillis() > heartbeatTimeout) {
                    if (connection->isHeartBeating()) {
                        logger.warning() << "Heartbeat failed over the connection: " << *connection;
                        connection->onHeartbeatFailed();
                        fireHeartbeatStopped(connection);
                    }
                }

                if (now - connection->lastReadTimeMillis() > heartbeatInterval) {
                    std::auto_ptr<protocol::ClientMessage> request = protocol::codec::ClientPingCodec::RequestParameters::encode();
                    boost::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client, request, "", connection);
                    clientInvocation->setBypassHeartbeatCheck(true);
                    connection->onHeartbeatRequested();
                    boost::shared_ptr<spi::impl::ClientInvocationFuture> clientInvocationFuture = spi::impl::ClientInvocation::invokeUrgent(
                            clientInvocation);
                    clientInvocationFuture->andThen(boost::shared_ptr<
                            impl::ExecutionCallback<boost::shared_ptr<protocol::ClientMessage> > >(
                            new HearbeatCallback(connection, logger)));
                } else {
                    if (!connection->isHeartBeating()) {
                        logger.warning() << "Heartbeat is back to healthy for the connection: " << *connection;
                        connection->onHeartbeatResumed();
                        fireHeartbeatResumed(connection);
                    }
                }
            }

            void HeartbeatManager::fireHeartbeatStopped(boost::shared_ptr<Connection> &connection) {
                std::vector<boost::shared_ptr<spi::impl::ConnectionHeartbeatListener> > listeners = heartbeatListeners;
                BOOST_FOREACH (boost::shared_ptr<spi::impl::ConnectionHeartbeatListener> heartbeatListener, listeners) {
                                heartbeatListener->heartbeatStopped(connection);
                            }
            }

            void HeartbeatManager::fireHeartbeatResumed(boost::shared_ptr<Connection> &connection) {
                std::vector<boost::shared_ptr<spi::impl::ConnectionHeartbeatListener> > listeners = heartbeatListeners;
                BOOST_FOREACH (boost::shared_ptr<spi::impl::ConnectionHeartbeatListener> heartbeatListener, listeners) {
                                heartbeatListener->heartbeatResumed(connection);
                            }
            }

            void HeartbeatManager::addConnectionHeartbeatListener(
                    const boost::shared_ptr<spi::impl::ConnectionHeartbeatListener> &connectionHeartbeatListener) {
                do {
                    std::vector<boost::shared_ptr<spi::impl::ConnectionHeartbeatListener> > listeners = heartbeatListeners;
                    std::vector<boost::shared_ptr<spi::impl::ConnectionHeartbeatListener> > newListeners = listeners;
                    newListeners.push_back(connectionHeartbeatListener);
                    if (heartbeatListeners.compareAndSet(listeners, newListeners)) {
                        break;
                    }
                } while (true);
            }

            void HeartbeatManager::shutdown() {
                heartbeatListeners = std::vector<boost::shared_ptr<spi::impl::ConnectionHeartbeatListener> >();
            }

            HeartbeatManager::HearbeatCallback::HearbeatCallback(const boost::shared_ptr<Connection> &connection,
                                                                 util::ILogger &logger)
                    : connection(connection), logger(logger) {}

            void
            HeartbeatManager::HearbeatCallback::onResponse(const boost::shared_ptr<protocol::ClientMessage> &response) {
                if (connection->isAlive()) {
                    connection->onHeartbeatReceived();
                }
            }

            void HeartbeatManager::HearbeatCallback::onFailure(const boost::shared_ptr<exception::IException> &e) {
                if (connection->isAlive()) {
                    logger.warning() << "Error receiving ping answer from the connection: " << *connection << *e;
                }
            }

            void HeartbeatManager::HeartbeatTask::run() {
                while (heartbeatManager.clientConnectionManager.isAlive()) {
                    heartbeatManager.run();
                }
            }

            const std::string HeartbeatManager::HeartbeatTask::getName() const {
                return "HeartbeatTask";
            }

            HeartbeatManager::HeartbeatTask::HeartbeatTask(HeartbeatManager &heartbeatManager) : heartbeatManager(
                    heartbeatManager) {
            }
        }
    }
}

