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

#pragma once

#include "hazelcast/util/SynchronizedQueue.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/client/config/ClientConnectionStrategyConfig.h"
#include "hazelcast/client/connection/ClientConnectionStrategy.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class HazelcastClientInstanceImpl;
        }
        namespace connection {

            /**
             * Default client connection strategy supporting async client start, reconnection modes and disabling reconnection.
             */
            class DefaultClientConnectionStrategy : public ClientConnectionStrategy {
            public:
                DefaultClientConnectionStrategy(spi::ClientContext &clientContext, util::ILogger &logger,
                                                const config::ClientConnectionStrategyConfig &clientConnectionStrategyConfig);


                virtual void start();

                virtual void beforeGetConnection(const Address &target);

                virtual void beforeOpenConnection(const Address &target);

                virtual void onConnectToCluster();

                virtual void onDisconnectFromCluster();

                virtual void onConnect(const std::shared_ptr<Connection> &connection);

                virtual void onDisconnect(const std::shared_ptr<Connection> &connection);

                virtual void shutdown();

                static void
                shutdownWithExternalThread(std::weak_ptr<client::impl::HazelcastClientInstanceImpl> clientImpl);

            private:
                util::AtomicBoolean disconnectedFromCluster;
                bool clientStartAsync;
                config::ClientConnectionStrategyConfig::ReconnectMode reconnectMode;
                std::atomic_bool isShutdown;

                bool isClusterAvailable() const;

                void checkShutdown(const std::string &methodName);
            };
        }
    }
}



