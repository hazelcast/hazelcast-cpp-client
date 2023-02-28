/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include <future>
#include <chrono>

#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/config/client_connection_strategy_config.h>
#include <hazelcast/client/lifecycle_listener.h>
#include <hazelcast/client/lifecycle_event.h>

hazelcast::client::lifecycle_listener
make_connection_listener(std::promise<void>& connected,
                         std::promise<void>& disconnected)
{
    return hazelcast::client::lifecycle_listener()
      .on_connected([&connected]() { connected.set_value(); })
      .on_disconnected([&disconnected]() { disconnected.set_value(); });
}

int
main()
{
    hazelcast::client::client_config config;

    /**
     * How a client reconnect to cluster after a disconnect can be configured.
     * The following example configures a total timeout of 30 seconds to connect
     * to a cluster, by initial backoff time being 100 milliseconds and doubling
     * the time before every try with a jitter of 0.8 up to a maximum of 3
     * seconds backoff between each try.
     */
    config.get_connection_strategy_config()
      .get_retry_config()
      .set_cluster_connect_timeout(std::chrono::seconds(30))
      .set_multiplier(2.0)
      .set_jitter(0.8)
      .set_initial_backoff_duration(std::chrono::milliseconds(100))
      .set_max_backoff_duration(std::chrono::seconds(3));

    auto hz = hazelcast::new_client(std::move(config)).get();

    auto map = hz.get_map("MyMap").get();

    map->put(1, 100);

    std::promise<void> connected, disconnected;
    hz.add_lifecycle_listener(
      make_connection_listener(connected, disconnected));

    // Please shut down the cluster at this point.
    disconnected.get_future().wait();

    std::cout << "Client is disconnected from the cluster now." << std::endl;

    auto connection_future = connected.get_future();
    if (connection_future.wait_for(std::chrono::seconds(30)) ==
        std::future_status::ready) {
        std::cout << "The client is connected to the cluster within 10 seconds "
                     "after disconnection as expected."
                  << std::endl;
    }

    hz.shutdown().get();
    std::cout << "Finished" << std::endl;

    return 0;
}
