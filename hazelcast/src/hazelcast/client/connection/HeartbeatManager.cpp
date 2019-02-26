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
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/protocol/codec/ClientPingCodec.h"
#include "hazelcast/client/connection/HeartbeatManager.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            HeartbeatManager::HeartbeatManager(spi::ClientContext &client) : client(client), clientConnectionManager(
                    client.getConnectionManager()), logger(client.getLogger()) {
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
                    if (connection->isAlive()) {
                        logger.warning() << "Heartbeat failed over the connection: " << *connection;
                        onHeartbeatStopped(connection, "Heartbeat timed out");
                    }
                }

                if (now - connection->lastReadTimeMillis() > heartbeatInterval) {
                    std::auto_ptr<protocol::ClientMessage> request = protocol::codec::ClientPingCodec::encodeRequest();
                    boost::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client, request, "", connection);
                    clientInvocation->invokeUrgent();
                }
            }

            void
            HeartbeatManager::onHeartbeatStopped(boost::shared_ptr<Connection> &connection, const std::string &reason) {
                connection->close(reason.c_str(), (exception::ExceptionBuilder<exception::TargetDisconnectedException>(
                        "HeartbeatManager::onHeartbeatStopped") << "Heartbeat timed out to connection "
                                                                << *connection).buildShared());
            }

            void HeartbeatManager::shutdown() {
            }
        }
    }
}

