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

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/config/SSLConfig.h"

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
                #ifdef HZ_BUILD_WITH_SSL
                /**
                 * Returns the current {@link SSLConfig}.
                 *
                 * @return the SSLConfig.
                 * @see #setSSLConfig(SSLConfig)
                 */
                const SSLConfig &getSSLConfig() const;

                /**
                 * Sets the {@link SSLConfig}.
                 *
                 * @param sslConfig the SSLConfig.
                 * @return the updated ClientNetworkConfig.
                 * @see #getSSLConfig()
                 */
                ClientNetworkConfig &setSSLConfig(const config::SSLConfig &sslConfig);
                #endif // HZ_BUILD_WITH_SSL

            private:
                #ifdef HZ_BUILD_WITH_SSL
                config::SSLConfig sslConfig;
                #endif
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_CONFIG_CLIENTNETWORKCONFIG_H_ */
