/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/config/ssl_config.h"
#include "hazelcast/client/config/client_aws_config.h"
#include "hazelcast/client/config/socket_options.h"
#include "hazelcast/client/config/cloud_config.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace config {
/**
 * Contains configuration parameters for client network related behaviour
 */
class HAZELCAST_API client_network_config
{
public:
    /**
     * Constructor with default values.
     * connectionTimeout(5000)
     */
    client_network_config();

    /**
     * Returns the current \ssl_config .
     *
     * @return the ssl_config .
     * @see # set_ssl_config (ssl_config )
     */
    ssl_config& get_ssl_config();

    /**
     * Sets the ssl_config
     *
     * @param config the ssl_config .
     * @return the updated client_network_config.
     * @see # get_ssl_config ()
     */
    client_network_config& set_ssl_config(const config::ssl_config& config);

    /**
     * @param connectionTimeout Timeout value for nodes to accept client
     * connection requests. A zero value means wait until connection established
     * or an error occurs. The resolution of time is up to milliseconds.
     *
     * @return itself client_network_config
     */
    client_network_config& set_connection_timeout(
      const std::chrono::milliseconds& timeout);

    /**
     * Connection timeout value for connecting to a member server.
     *
     * @return connection timeout set for a single connection attempt.
     */
    std::chrono::milliseconds get_connection_timeout() const;

    /**
     * Sets configuration to connect nodes in aws environment.
     *
     * @param clientAwsConfig the client_aws_config
     * @see #get_aws_config()
     */
    client_network_config& set_aws_config(
      const client_aws_config& client_aws_config);

    /**
     * Returns the current client_aws_config.
     *
     * @return client_aws_config
     */
    client_aws_config& get_aws_config();

    /**
     * Returns the current cloud_config.
     *
     * @return cloud_config
     */
    cloud_config& get_cloud_config();

    /**
     * See client_network_config#setSmartRouting(boolean)  for details
     *
     * @return true if client is smart
     */
    bool is_smart_routing() const;

    /**
     * If {@code true}, client will route the key based operations to owner of
     * the key on best-effort basis. Note that it uses a cached version of
     * PartitionService#getPartitions() and doesn't guarantee that the operation
     * will always be executed on the owner. The cached table is updated every
     * 10 seconds. <p> If {@code smartRouting == false}, all operations will be
     * routed to single member. Operations will need two hops if the chosen
     * member is not owner of the key. Client will have only single open
     * connection. Useful, if there are many clients and we want to avoid each
     * of them connecting to each member. <p> Default value is {@code true}.
     *
     * @param smartRouting true if smart routing should be enabled.
     * @return configured client_network_config for chaining
     */
    client_network_config& set_smart_routing(bool smart_routing);

    /**
     * Returns the list of candidate addresses that client will use to establish
     * initial connection
     *
     * @return list of addresses
     */
    std::vector<address> get_addresses() const;

    /**
     * Adds given addresses to candidate address list that client will use to
     * establish initial connection
     *
     * @param addresses to be added to initial address list
     * @return configured \client_network_config for chaining
     */
    client_network_config& add_addresses(const std::vector<address>& addresses);

    /**
     * Adds given addresses to candidate address list that client will use to
     * establish initial connection
     *
     * @param addresses to be added to initial address list
     * @return configured \client_network_config for chaining
     */
    client_network_config& set_addresses(const std::vector<address>& addresses);

    /**
     * Adds given address to candidate address list that client will use to
     * establish initial connection
     *
     * @param address to be added to initial address list
     * @return configured \client_network_config for chaining
     */
    client_network_config& add_address(const address& address);

    socket_options& get_socket_options();

    /**
     *
     * @return true if the public address of the server needs to be used when
     * connecting to the cluster.
     */
    bool use_public_address() const;

    /**
     *
     * @param should_use_public_address `true` mean force usage of the public
     * address of the server when connecting to the server. Otherwise, set to
     * `false` to use the private address. Default is `false`.
     * @return configured \client_network_config for chaining
     */
    client_network_config& use_public_address(bool should_use_public_address);

private:
    config::ssl_config ssl_config_;
    config::client_aws_config client_aws_config_;
    config::cloud_config cloud_config_;

    std::chrono::milliseconds connection_timeout_;
    bool smart_routing_;

    std::vector<address> address_list_;

    socket_options socket_options_;

    bool use_public_address_{ false };
};
} // namespace config
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
