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

#include <future>
#include <chrono>

#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/config/ClientConnectionStrategyConfig.h>
#include <hazelcast/client/LifecycleListener.h>
#include <hazelcast/client/LifecycleEvent.h>

hazelcast::client::LifecycleListener make_connection_listener(std::promise<void> &connected, std::promise<void> &disconnected) {
    return hazelcast::client::LifecycleListener()
        .on_connected([&connected](){
            connected.set_value();
        })
        .on_disconnected([&disconnected](){
            disconnected.set_value();
        });
}

int main() {
    hazelcast::client::client_config config;

    /**
     * How a client reconnect to cluster after a disconnect can be configured. This parameter is used by default strategy and
     * custom implementations may ignore it if configured.
     * default value is {@link ReconnectMode#ON}
     *
     * This example forces client to reconnect to the cluster in an async manner.
     */
    config.get_connection_strategy_config().set_reconnect_mode(hazelcast::client::config::ClientConnectionStrategyConfig::ASYNC);

    hazelcast::client::HazelcastClient hz(config);

    auto map = hz.get_map("MyMap");

    map->put(1, 100);

    std::promise<void> connected, disconnected;
    hz.add_lifecycle_listener(make_connection_listener(connected, disconnected));

    // Please shut down the cluster at this point.
    disconnected.get_future().wait();

    std::cout << "Client is disconnected from the cluster now." << std::endl;

    auto connection_future = connected.get_future();
    if (connection_future.wait_for(std::chrono::seconds(10)) == std::future_status::ready) {
        std::cout << "The client is connected to the cluster within 10 seconds after disconnection as expected." << std::endl;
    }

    hz.shutdown();
    std::cout << "Finished" << std::endl;

    return 0;
}

