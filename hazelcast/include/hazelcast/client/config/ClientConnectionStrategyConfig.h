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
#ifndef HAZELCAST_CLIENT_CONFIG_CLIENTCONNECTIONSTRATEGYCONFIG_H_
#define HAZELCAST_CLIENT_CONFIG_CLIENTCONNECTIONSTRATEGYCONFIG_H_

#include "hazelcast/util/HazelcastDll.h"

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
            class HAZELCAST_API ClientConnectionStrategyConfig {
            public:
                ClientConnectionStrategyConfig();

                /**
                 * Reconnect options.
                 */
                enum ReconnectMode {
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
                     * {@link com.hazelcast.client.HazelcastClientOfflineException }
                     */
                            ASYNC
                };

                /**
                 * Client instance creation won't block on {@link HazelcastClient#newHazelcastClient()} if this value is true
                 * @return if client connects to cluster asynchronously
                 */

                bool isAsyncStart() const;

                /**
                 * Set true for non blocking {@link HazelcastClient(const ClientConfig &)}. The client creation won't wait to
                 * connect to cluster. The client instace will throw exception until it connects to cluster and become ready.
                 * If set to false, {@link HazelcastClient(const ClientConfig &)} will block until a cluster connection established and it's
                 * ready to use client instance
                 *
                 * default value is false
                 * @param asyncStart true for async client creation
                 * @return the updated ClientConnectionStrategyConfig
                 */
                ClientConnectionStrategyConfig &setAsyncStart(bool asyncStart);

                /**
                 * @return reconnect mode
                 */
                ReconnectMode getReconnectMode() const;

                /**
                 * How a client reconnect to cluster after a disconnect can be configured. This parameter is used by default strategy and
                 * custom implementations may ignore it if configured.
                 * default value is {@link ReconnectMode#ON}
                 * @param reconnectMode
                 * @return the updated ClientConnectionStrategyConfig
                 */
                ClientConnectionStrategyConfig &setReconnectMode(ReconnectMode reconnectMode);

            private:
                bool asyncStart;
                ReconnectMode reconnectMode;

            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_CONFIG_CLIENTCONNECTIONSTRATEGYCONFIG_H_ */
