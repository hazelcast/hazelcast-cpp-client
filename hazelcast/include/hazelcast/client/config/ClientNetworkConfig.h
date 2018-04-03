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

                /**
                 * See {@link com.hazelcast.client.config.ClientNetworkConfig#setSmartRouting(boolean)}  for details
                 *
                 * @return true if client is smart
                 */
                bool isSmartRouting() const;

                /**
                 * If {@code true}, client will route the key based operations to owner of the key on best-effort basis.
                 * Note that it uses a cached version of {@link com.hazelcast.core.PartitionService#getPartitions()} and doesn't
                 * guarantee that the operation will always be executed on the owner. The cached table is updated every 10 seconds.
                 * <p>
                 * If {@code smartRouting == false}, all operations will be routed to single member. Operations will need two
                 * hops if the chosen member is not owner of the key. Client will have only single open connection. Useful, if
                 * there are many clients and we want to avoid each of them connecting to each member.
                 * <p>
                 * Default value is {@code true}.
                 *
                 * @param smartRouting true if smart routing should be enabled.
                 * @return configured {@link com.hazelcast.client.config.ClientNetworkConfig} for chaining
                 */
                ClientNetworkConfig &setSmartRouting(bool smartRouting);

                /**
                 * See {@link ClientNetworkConfig#setConnectionAttemptLimit(int32_t)} for details
                 *
                 * @return connection attempt Limit
                 */
                int32_t getConnectionAttemptLimit() const;

                /**
                 * While client is trying to connect initially to one of the members in the {@link ClientNetworkConfig#addressList},
                 * all might be not available. Instead of giving up, throwing Exception and stopping client, it will
                 * attempt to retry as much as {@link ClientNetworkConfig#connectionAttemptLimit} times.
                 *
                 * @param connectionAttemptLimit number of times to attempt to connect
                 *                               A zero value means try forever.
                 *                               A negative value means default value
                 * @return configured {@link ClientNetworkConfig} for chaining
                 */
                ClientNetworkConfig &setConnectionAttemptLimit(int32_t connectionAttemptLimit);

                /**
                 * Period for the next attempt to find a member to connect.
                 * <p>
                 * See {@link ClientNetworkConfig#connectionAttemptLimit}.
                 *
                 * @return connection attempt period in millis
                 */
                int32_t getConnectionAttemptPeriod() const;

                /**
                 * Period for the next attempt to find a member to connect.
                 *
                 * @param connectionAttemptPeriod time to wait before another attempt in millis
                 * @return configured {@link ClientNetworkConfig} for chaining
                 */
                ClientNetworkConfig  &setConnectionAttemptPeriod(int32_t connectionAttemptPeriod);

                /**
                 * Returns the list of candidate addresses that client will use to establish initial connection
                 *
                 * @return list of addresses
                 */
                std::vector<Address> getAddresses() const;

                /**
                 * Adds given addresses to candidate address list that client will use to establish initial connection
                 *
                 * @param addresses to be added to initial address list
                 * @return configured {@link com.hazelcast.client.config.ClientNetworkConfig} for chaining
                 */
                ClientNetworkConfig &addAddresses(const std::vector<Address> &addresses);

                /**
                 * Adds given addresses to candidate address list that client will use to establish initial connection
                 *
                 * @param addresses to be added to initial address list
                 * @return configured {@link ClientNetworkConfig} for chaining
                 */
                ClientNetworkConfig &setAddresses(const std::vector<Address> &addresses);

                /**
                * Adds given address to candidate address list that client will use to establish initial connection
                *
                * @param address to be added to initial address list
                * @return configured {@link ClientNetworkConfig} for chaining
                */
                ClientNetworkConfig &addAddress(const Address &address);

            private:
                static int32_t CONNECTION_ATTEMPT_PERIOD;

                config::SSLConfig sslConfig;
                config::ClientAwsConfig clientAwsConfig;

                int64_t connectionTimeout;
                bool smartRouting;

                int32_t connectionAttemptLimit;
                int32_t connectionAttemptPeriod;

                std::vector<Address> addressList;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_CONFIG_CLIENTNETWORKCONFIG_H_ */
