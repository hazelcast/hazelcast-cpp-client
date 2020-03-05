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

#ifndef HAZELCAST_CLIENT_CONNECTION_HEARBEATMANAGER_H_
#define HAZELCAST_CLIENT_CONNECTION_HEARBEATMANAGER_H_

#include "hazelcast/util/Thread.h"

#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/util/Executor.h"

namespace hazelcast {
    namespace util {
        class ILogger;

    }
    namespace client {
        namespace spi {
            class ClientContext;
            namespace impl {
                class ConnectionHeartbeatListener;
            }
        }
        namespace connection {
            class ClientConnectionManagerImpl;

            class Connection;

            /**
             * HeartbeatManager manager used by connection manager.
             */
            class HeartbeatManager : public util::Runnable {
            public:
                HeartbeatManager(spi::ClientContext &client);

                void start();

                void shutdown();

                virtual void run();

                virtual const std::string getName() const;

            private:
                spi::ClientContext &client;
                ClientConnectionManagerImpl &clientConnectionManager;
                util::ILogger &logger;
                int64_t heartbeatInterval;
                int64_t heartbeatTimeout;

                void checkConnection(int64_t now, std::shared_ptr<Connection> &connection);

                void onHeartbeatStopped(std::shared_ptr<Connection> &connection, const std::string &reason);
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_CONNECTION_HEARBEATMANAGER_H_

