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

#ifndef HAZELCAST_CLIENT_CONNECTION_CLIENTCONNECTIONSTRATEGY_H_
#define HAZELCAST_CLIENT_CONNECTION_CLIENTCONNECTIONSTRATEGY_H_

#include <boost/shared_ptr.hpp>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        class ILogger;
    }
    namespace client {
        class Address;

        namespace config {
            class ClientConnectionStrategyConfig;
        }
        namespace spi {
            class ClientContext;
        }
        namespace connection {
            class Connection;

            /**
             * An abstract class called from {@link ConnectionManager} to customize
             * how client connect to cluster, and provide various behaviours like async start or restart.
             */
            class HAZELCAST_API ClientConnectionStrategy {
            public:
                ClientConnectionStrategy(spi::ClientContext &clientContext, util::ILogger &logger,
                                         const config::ClientConnectionStrategyConfig &clientConnectionStrategyConfig);

                virtual ~ClientConnectionStrategy();

                /**
                 * Called after {@link ConnectionManager} started.
                 * Connecting to cluster can be triggered from this method using one of
                 * {@link ConnectionManager#connectToCluster} or
                 * {@link ConnectionManager#connectToClusterAsync}
                 */
                virtual void start() = 0;

                /**
                 * The purpose of this method is to validate a connection request by target, and exit the blocking invocation.
                 * For all connection requests on {@link ConnectionManager} this method will be called.
                 *
                 * The build in retry mechanism can be stopped by throwing an instance of non retryable exceptions;
                 * {@link IOException}, {@link HazelcastInstanceNotActiveException} or
                 * {@link RetryableException}
                 *
                 * The thrown exception will be received on the blocking user. Any blocking invocation will exit by that exception.
                 * @param target address of the requested connection
                 */
                virtual void beforeGetConnection(const Address &target) = 0;

                /**
                 * If a new connection is required to open by {@link ConnectionManager},
                 * this method will be called.
                 *
                 * This request can be rejected by throwing an instance of non retryable exceptions;
                 * {@link java.io.IOException}, {@link HazelcastInstanceNotActiveException} or
                 * {@link RetryableException}
                 * @param target address of the requested connection
                 */
                virtual void beforeOpenConnection(const Address &target) = 0;

                /**
                 * If a cluster connection is established, this method will be called.
                 * if an exception is thrown, the already established connection will be closed.
                 */
                virtual void onConnectToCluster() = 0;

                /**
                 * If the cluster connection is lost for any reason, this method will be called.
                 *
                 */
                virtual void onDisconnectFromCluster() = 0;

                /**
                 * If the {@link ConnectionManager} opens a new connection to a member,
                 * this method will be called with the connection parameter
                 * @param connection the new established connection
                 */
                virtual void onConnect(const boost::shared_ptr<Connection> &connection) = 0;

                /**
                 * If a connection is disconnected, this method will be called with the connection parameter
                 * @param connection the closed connection
                 */
                virtual void onDisconnect(const boost::shared_ptr<Connection> &connection) = 0;

                /**
                 * The {@link ConnectionManager} will inform this method that the provided connection's heartbeat stopped
                 * @param connection the connection that heartbeat failed
                 */
                virtual void onHeartbeatStopped(const boost::shared_ptr<Connection> &connection) = 0;

                /**
                 * The {@link ConnectionManager} will inform this method that the provided connection's heartbeat resumed
                 * @param connection the connection that heartbeat resumed
                 */
                virtual void onHeartbeatResumed(const boost::shared_ptr<Connection> &connection) = 0;

                /**
                 * The {@link ConnectionManager} will call this method as a last step of its shutdown.
                 */
                virtual void shutdown() = 0;

            protected:
                spi::ClientContext &clientContext;
                util::ILogger &logger;
                const config::ClientConnectionStrategyConfig &clientConnectionStrategyConfig;
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_CONNECTION_CLIENTCONNECTIONSTRATEGY_H_

