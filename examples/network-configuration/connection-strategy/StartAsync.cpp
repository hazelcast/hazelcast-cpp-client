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
#include <chrono>
#include <future>

#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/lifecycle_listener.h>
#include <hazelcast/client/lifecycle_event.h>


int main() {
    hazelcast::client::client_config config;

    /**
     * Set true for non blocking {@link hazelcast_client(const client_config &)}. The client creation won't wait to
     * connect to cluster. The client instace will throw exception until it connects to cluster and become ready.
     * If set to false, {@link hazelcast_client(const client_config &)} will block until a cluster connection established and it's
     * ready to use client instance.
     *
     * default value is false
     */
    config.get_connection_strategy_config().set_async_start(true);

    // Add a lifecycle listener so that we can track when the client is connected
    std::promise<void> connected;
    config.add_listener(
        hazelcast::client::lifecycle_listener().
            on_connected([&connected](){
                connected.set_value();
            })
    );

    hazelcast::client::hazelcast_client hz(std::move(config));

    auto connection_future = connected.get_future();
    if (connection_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        std::cout << "Async client is not connected yet." << std::endl;
    }

    // the client may not have connected to the cluster yet at this point since the cluster connection is async!!!
    connection_future.wait();

    std::cout << "Async client is connected now." << std::endl;

    std::cout << "Finished" << std::endl;

    hz.shutdown().get();

    return 0;
}

