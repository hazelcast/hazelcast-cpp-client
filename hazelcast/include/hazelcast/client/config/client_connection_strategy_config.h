/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/util/export.h"
#include "hazelcast/client/config/connection_retry_config.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            /**
             * Client connection strategy configuration is used for setting custom strategies and configuring strategy parameters.
             */
            class HAZELCAST_API client_connection_strategy_config {
            public:
                client_connection_strategy_config();

                /**
                 * Reconnect options.
                 */
                enum reconnect_mode {
                    /**
                     * Prevent reconnect to cluster after a disconnect
                     */
                            OFF,
                    /**
                     * Reconnect to cluster by blocking invocations
                     */
                            ON,
                    /**
                     * Reconnect to cluster without blocking invocations. Invocations will receive
                     * {@link hazelcast_client_offline}
                     */
                            ASYNC
                };

                /**
                 * Client instance creation won't block on {@link hazelcast_client(ClientConfig &)} if this value is true
                 * @return if client connects to cluster asynchronously
                 */

                bool is_async_start() const;

                /**
                 * Set true for non blocking {@link hazelcast_client(const client_config &)}. The client creation won't wait to
                 * connect to cluster. The client instace will throw exception until it connects to cluster and become ready.
                 * If set to false, {@link hazelcast_client(const client_config &)} will block until a cluster connection established and it's
                 * ready to use client instance
                 *
                 * default value is false
                 * @param asyncStart true for async client creation
                 * @return the updated ClientConnectionStrategyConfig
                 */
                client_connection_strategy_config &set_async_start(bool async_start);

                /**
                 * @return reconnect mode
                 */
                reconnect_mode get_reconnect_mode() const;

                /**
                 * How a client reconnect to cluster after a disconnect can be configured. This parameter is used by default strategy and
                 * custom implementations may ignore it if configured.
                 * default value is {@link ReconnectMode#ON}
                 * @param reconnectMode
                 * @return the updated ClientConnectionStrategyConfig
                 */
                client_connection_strategy_config &set_reconnect_mode(reconnect_mode reconnect_mode);

                /**
                 * connection_retry_config controls the period among the retries and when a client should give up
                 * retrying. Exponential behaviour can be chosen or jitter can be added to wait periods.
                 *
                 * @return connection_retry_config the connection retry config to read or modify.
                 */
                connection_retry_config &get_retry_config();

                /**
                 * connection_retry_config controls the period among the retries and when a client should give up
                 * retrying. Exponential behaviour can be chosen or jitter can be added to wait periods.
                 *
                 * @param connection_retry_config the connection retry config to read or modify.
                 * @return the updated client_connection_strategy_config
                 */
                client_connection_strategy_config &set_retry_config(connection_retry_config retry_config);

            private:
                bool async_start_;
                reconnect_mode reconnect_mode_;
                connection_retry_config retry_config_;
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
