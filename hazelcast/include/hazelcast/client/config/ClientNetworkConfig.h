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

#include <memory>
#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/config/SSLConfig.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/config/SocketOptions.h"

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
                 * Returns the current \SSLConfig.
                 *
                 * @return the SSLConfig.
                 * @see #setSSLConfig(SSLConfig)
                 */
                SSLConfig &getSSLConfig();

                /**
                 * Sets the SSLConfig
                 *
                 * @param config the SSLConfig.
                 * @return the updated ClientNetworkConfig.
                 * @see #getSSLConfig()
                 */
                ClientNetworkConfig &setSSLConfig(const config::SSLConfig &config);

                /**
                * @param connectionTimeout Timeout value for nodes to accept client connection requests.
                *                          A zero value means wait until connection established or an error occurs.
                *                          The resolution of time is up to milliseconds.
                *
                * @return itself ClientNetworkConfig
                */
                ClientNetworkConfig &setConnectionTimeout(const std::chrono::milliseconds &timeout);

                /**
                * Connection timeout value for connecting to a member server.
                *
                * @return connection timeout set for a single connection attempt.
                */
                std::chrono::milliseconds getConnectionTimeout() const;

                /**
                 * Sets configuration to connect nodes in aws environment.
                 *
                 * @param clientAwsConfig the ClientAwsConfig
                 * @see #getAwsConfig()
                 */
                ClientNetworkConfig &setAwsConfig(const ClientAwsConfig &clientAwsConfig);

                /**
                 * Returns the current ClientAwsConfig.
                 *
                 * @return ClientAwsConfig
                 */
                ClientAwsConfig &getAwsConfig();

                /**
                 * See ClientNetworkConfig#setSmartRouting(boolean)  for details
                 *
                 * @return true if client is smart
                 */
                bool isSmartRouting() const;

                /**
                 * If {@code true}, client will route the key based operations to owner of the key on best-effort basis.
                 * Note that it uses a cached version of PartitionService#getPartitions() and doesn't
                 * guarantee that the operation will always be executed on the owner. The cached table is updated every 10 seconds.
                 * <p>
                 * If {@code smartRouting == false}, all operations will be routed to single member. Operations will need two
                 * hops if the chosen member is not owner of the key. Client will have only single open connection. Useful, if
                 * there are many clients and we want to avoid each of them connecting to each member.
                 * <p>
                 * Default value is {@code true}.
                 *
                 * @param smartRouting true if smart routing should be enabled.
                 * @return configured ClientNetworkConfig for chaining
                 */
                ClientNetworkConfig &setSmartRouting(bool smartRouting);

                /**
                 * See ClientNetworkConfig#setConnectionAttemptLimit(int32_t) for details
                 *
                 * @return connection attempt Limit
                 */
                int32_t getConnectionAttemptLimit() const;

                /**
                 * While client is trying to connect initially to one of the members in the ClientNetworkConfig#addressList,
                 * all might be not available. Instead of giving up, throwing Exception and stopping client, it will
                 * attempt to retry as much as ClientNetworkConfig#connectionAttemptLimit() times.
                 *
                 * @param connectionAttemptLimit number of times to attempt to connect
                 *                               A zero value means try forever.
                 *                               A negative value means default value
                 * @return configured \ClientNetworkConfig for chaining
                 */
                ClientNetworkConfig &setConnectionAttemptLimit(int32_t connectionAttemptLimit);

                /**
                 * Period for the next attempt to find a member to connect.
                 * <p>
                 * See \ClientNetworkConfig::connectionAttemptLimit
                 *
                 * @return connection attempt period
                 */
                std::chrono::milliseconds getConnectionAttemptPeriod() const;

                /**
                 * Period for the next attempt to find a member to connect.
                 *
                 * @param period time to wait before another attempt. The resolution of time is up to milliseconds.
                 * @return configured \ClientNetworkConfig for chaining
                 */
                ClientNetworkConfig &setConnectionAttemptPeriod(const std::chrono::milliseconds &interval);

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
                 * @return configured \ClientNetworkConfig for chaining
                 */
                ClientNetworkConfig &addAddresses(const std::vector<Address> &addresses);

                /**
                 * Adds given addresses to candidate address list that client will use to establish initial connection
                 *
                 * @param addresses to be added to initial address list
                 * @return configured \ClientNetworkConfig for chaining
                 */
                ClientNetworkConfig &setAddresses(const std::vector<Address> &addresses);

                /**
                * Adds given address to candidate address list that client will use to establish initial connection
                *
                * @param address to be added to initial address list
                * @return configured \ClientNetworkConfig for chaining
                */
                ClientNetworkConfig &addAddress(const Address &address);

                SocketOptions &getSocketOptions();

            private:
                static int32_t CONNECTION_ATTEMPT_PERIOD;

                config::SSLConfig sslConfig;
                config::ClientAwsConfig clientAwsConfig;

                std::chrono::milliseconds connectionTimeout;
                bool smartRouting;

                int32_t connectionAttemptLimit;
                std::chrono::milliseconds connectionAttemptPeriod;

                std::vector<Address> addressList;

                SocketOptions socketOptions;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


