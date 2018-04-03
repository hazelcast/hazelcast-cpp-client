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

#ifndef HAZELCAST_CLIENT_CONNECTION_HEARBEATMANAGER_H_
#define HAZELCAST_CLIENT_CONNECTION_HEARBEATMANAGER_H_

#include "hazelcast/util/Thread.h"
#include "hazelcast/util/Atomic.h"
#include "hazelcast/client/impl/ExecutionCallback.h"
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
            class HAZELCAST_API HeartbeatManager : public util::Runnable {
            public:
                HeartbeatManager(spi::ClientContext &client);

                void start();

                void shutdown();

                virtual void run();

                virtual const std::string getName() const;

                void addConnectionHeartbeatListener(
                        const boost::shared_ptr<spi::impl::ConnectionHeartbeatListener> &connectionHeartbeatListener);

            private:
                class HearbeatCallback : public impl::ExecutionCallback<boost::shared_ptr<protocol::ClientMessage> > {
                public:
                    HearbeatCallback(const boost::shared_ptr<Connection> &connection, util::ILogger &logger);

                    virtual void onResponse(const boost::shared_ptr<protocol::ClientMessage> &response);

                    virtual void onFailure(const boost::shared_ptr<exception::IException> &e);

                private:
                    boost::shared_ptr<connection::Connection> connection;
                    util::ILogger &logger;
                };

                class HeartbeatTask : public util::Runnable {
                public:
                    HeartbeatTask(HeartbeatManager &heartbeatManager);

                    virtual void run();

                    virtual const std::string getName() const;

                private:
                    HeartbeatManager &heartbeatManager;
                };

                spi::ClientContext &client;
                ClientConnectionManagerImpl &clientConnectionManager;
                util::ILogger &logger;
                int64_t heartbeatInterval;
                int64_t heartbeatTimeout;
                util::Atomic<std::vector<boost::shared_ptr<spi::impl::ConnectionHeartbeatListener> > > heartbeatListeners;
                boost::shared_ptr<util::ExecutorService> executor;

                void checkConnection(int64_t now, boost::shared_ptr<Connection> &connection);

                void fireHeartbeatStopped(boost::shared_ptr<Connection> &connection);

                void fireHeartbeatResumed(boost::shared_ptr<Connection> &connection);
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_CONNECTION_HEARBEATMANAGER_H_

