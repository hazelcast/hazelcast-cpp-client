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
                SSLConfig &get_ssl_config();

                /**
                 * Sets the SSLConfig
                 *
                 * @param config the SSLConfig.
                 * @return the updated ClientNetworkConfig.
                 * @see #getSSLConfig()
                 */
                ClientNetworkConfig &set_ssl_config(const config::SSLConfig &config);

                /**
                * @param connectionTimeout Timeout value for nodes to accept client connection requests.
                *                          A zero value means wait until connection established or an error occurs.
                *                          The resolution of time is up to milliseconds.
                *
                * @return itself ClientNetworkConfig
                */
                ClientNetworkConfig &set_connection_timeout(const std::chrono::milliseconds &timeout);

                /**
                * Connection timeout value for connecting to a member server.
                *
                * @return connection timeout set for a single connection attempt.
                */
                std::chrono::milliseconds get_connection_timeout() const;

                /**
                 * Sets configuration to connect nodes in aws environment.
                 *
                 * @param clientAwsConfig the ClientAwsConfig
                 * @see #getAwsConfig()
                 */
                ClientNetworkConfig &set_aws_config(const ClientAwsConfig &client_aws_config);

                /**
                 * Returns the current ClientAwsConfig.
                 *
                 * @return ClientAwsConfig
                 */
                ClientAwsConfig &get_aws_config();

                /**
                 * See ClientNetworkConfig#setSmartRouting(boolean)  for details
                 *
                 * @return true if client is smart
                 */
                bool is_smart_routing() const;

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
                ClientNetworkConfig &set_smart_routing(bool smart_routing);

                /**
                 * See ClientNetworkConfig#setConnectionAttemptLimit(int32_t) for details
                 *
                 * @return connection attempt Limit
                 */
                int32_t get_connection_attempt_limit() const;

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
                ClientNetworkConfig &set_connection_attempt_limit(int32_t connection_attempt_limit);

                /**
                 * Period for the next attempt to find a member to connect.
                 * <p>
                 * See \ClientNetworkConfig::connectionAttemptLimit
                 *
                 * @return connection attempt period
                 */
                std::chrono::milliseconds get_connection_attempt_period() const;

                /**
                 * Period for the next attempt to find a member to connect.
                 *
                 * @param period time to wait before another attempt. The resolution of time is up to milliseconds.
                 * @return configured \ClientNetworkConfig for chaining
                 */
                ClientNetworkConfig &set_connection_attempt_period(const std::chrono::milliseconds &interval);

                /**
                 * Returns the list of candidate addresses that client will use to establish initial connection
                 *
                 * @return list of addresses
                 */
                std::vector<Address> get_addresses() const;

                /**
                 * Adds given addresses to candidate address list that client will use to establish initial connection
                 *
                 * @param addresses to be added to initial address list
                 * @return configured \ClientNetworkConfig for chaining
                 */
                ClientNetworkConfig &add_addresses(const std::vector<Address> &addresses);

                /**
                 * Adds given addresses to candidate address list that client will use to establish initial connection
                 *
                 * @param addresses to be added to initial address list
                 * @return configured \ClientNetworkConfig for chaining
                 */
                ClientNetworkConfig &set_addresses(const std::vector<Address> &addresses);

                /**
                * Adds given address to candidate address list that client will use to establish initial connection
                *
                * @param address to be added to initial address list
                * @return configured \ClientNetworkConfig for chaining
                */
                ClientNetworkConfig &add_address(const Address &address);

                SocketOptions &get_socket_options();

            private:
                static int32_t CONNECTION_ATTEMPT_PERIOD;

                config::SSLConfig sslConfig_;
                config::ClientAwsConfig clientAwsConfig_;

                std::chrono::milliseconds connectionTimeout_;
                bool smartRouting_;

                int32_t connectionAttemptLimit_;
                std::chrono::milliseconds connectionAttemptPeriod_;

                std::vector<Address> addressList_;

                SocketOptions socketOptions_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


