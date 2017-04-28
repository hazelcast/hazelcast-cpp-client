/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_CONFIG_CLIENTNETWORKCONFIG_H_
#define HAZELCAST_CLIENT_CONFIG_CLIENTNETWORKCONFIG_H_

#include <memory>
#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/config/SSLConfig.h"
#include "hazelcast/client/config/ClientAwsConfig.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            /**
             * Contains configuration parameters for client network related behaviour
             */
            class HAZELCAST_API ClientNetworkConfig  {
            public:

                /**
                * Constructor with default values.
                * connectionTimeout(5000)
                */
                ClientNetworkConfig();

                /**
                 * Returns the current {@link SSLConfig}.
                 *
                 * @return the SSLConfig.
                 * @see #setSSLConfig(SSLConfig)
                 */
                SSLConfig &getSSLConfig();

                /**
                 * Sets the {@link SSLConfig}.
                 *
                 * @param sslConfig the SSLConfig.
                 * @return the updated ClientNetworkConfig.
                 * @see #getSSLConfig()
                 */
                ClientNetworkConfig &setSSLConfig(const config::SSLConfig &sslConfig);

                /**
                * @param connectionTimeout Timeout value in millis for nodes to accept client connection requests.
                *                          A zero value means wait until connection established or an error occurs.
                *
                * @return itself ClientNetworkConfig
                */
                ClientNetworkConfig &setConnectionTimeout(int64_t connectionTimeoutInMillis);

                /**
                * Timeout value for nodes to accept client connection requests.
                *
                * @return int connectionTimeout in millis
                */
                int64_t getConnectionTimeout() const;

                /**
                 * Sets configuration to connect nodes in aws environment.
                 *
                 * @param clientAwsConfig the ClientAwsConfig
                 * @see #getAwsConfig()
                 */
                ClientNetworkConfig &setAwsConfig(const ClientAwsConfig &clientAwsConfig);

                /**
                 * Returns the current {@link ClientAwsConfig}.
                 *
                 * @return ClientAwsConfig
                 */
                ClientAwsConfig &getAwsConfig();
            private:
                config::SSLConfig sslConfig;
                config::ClientAwsConfig clientAwsConfig;

                int64_t connectionTimeout;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_CONFIG_CLIENTNETWORKCONFIG_H_ */
