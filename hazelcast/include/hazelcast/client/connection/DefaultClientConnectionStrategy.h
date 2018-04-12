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

#ifndef HAZELCAST_CLIENT_CONNECTION_DEFAULTCLIENTCONNECTIONSTRATEGY_H_
#define HAZELCAST_CLIENT_CONNECTION_DEFAULTCLIENTCONNECTIONSTRATEGY_H_

#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/SynchronizedQueue.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/client/config/ClientConnectionStrategyConfig.h"
#include "hazelcast/util/Executor.h"
#include "hazelcast/client/connection/ClientConnectionStrategy.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            /**
             * Default client connection strategy supporting async client start, reconnection modes and disabling reconnection.
             */
            class HAZELCAST_API DefaultClientConnectionStrategy : public ClientConnectionStrategy {
            public:
                DefaultClientConnectionStrategy(spi::ClientContext &clientContext, util::ILogger &logger,
                                                const config::ClientConnectionStrategyConfig &clientConnectionStrategyConfig);


                virtual void start();

                virtual void beforeGetConnection(const Address &target);

                virtual void beforeOpenConnection(const Address &target);

                virtual void onConnectToCluster();

                virtual void onDisconnectFromCluster();

                virtual void onConnect(const boost::shared_ptr<Connection> &connection);

                virtual void onDisconnect(const boost::shared_ptr<Connection> &connection);

                virtual void onHeartbeatStopped(const boost::shared_ptr<Connection> &connection);

                virtual void onHeartbeatResumed(const boost::shared_ptr<Connection> &connection);

                virtual void shutdown();

            private:
                class ShutdownTask : public util::Runnable {
                public:
                    ShutdownTask(spi::ClientContext &clientContext);

                    virtual void run();

                    virtual const std::string getName() const;

                private:
                    spi::ClientContext &clientContext;
                };

                util::AtomicBoolean disconnectedFromCluster;
                bool clientStartAsync;
                config::ClientConnectionStrategyConfig::ReconnectMode reconnectMode;
                // This queue is used for avoiding memory leak
                util::SynchronizedQueue<util::Thread> shutdownThreads;

                bool isClusterAvailable() const;

                void shutdownWithExternalThread();
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_CONNECTION_DEFAULTCLIENTCONNECTIONSTRATEGY_H_

