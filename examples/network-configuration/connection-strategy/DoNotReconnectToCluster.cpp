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
#include <boost/thread/latch.hpp>

#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/config/ClientConnectionStrategyConfig.h>
#include <hazelcast/client/LifecycleListener.h>
#include <hazelcast/client/LifecycleEvent.h>


int main() {
    hazelcast::client::ClientConfig config;

    /**
     * How a client reconnect to cluster after a disconnect can be configured. This parameter is used by default strategy and
     * custom implementations may ignore it if configured.
     * default value is {@link ReconnectMode#ON}
     *
     * This example forces client NOT to reconnect if it ever disconnects from the cluster.
     */
    config.getConnectionStrategyConfig().setReconnectMode(hazelcast::client::config::ClientConnectionStrategyConfig::OFF);

    hazelcast::client::HazelcastClient hz(config);

    auto map = hz.getMap("MyMap");

    map->put(1, 100);

    boost::latch disconnectedLatch(1);
    boost::latch connectedLatch(1);

    hz.addLifecycleListener([&](const hazelcast::client::LifecycleEvent &lifecycleEvent) {
        if (lifecycleEvent.getState() == hazelcast::client::LifecycleEvent::CLIENT_DISCONNECTED) {
            disconnectedLatch.count_down();
        } else if (lifecycleEvent.getState() == hazelcast::client::LifecycleEvent::CLIENT_CONNECTED) {
            connectedLatch.count_down();
        }
    });

    // Please shut down the cluster at this point.
    disconnectedLatch.wait();

    std::cout << "Client is disconnected from the cluster now." << std::endl;

    if (connectedLatch.wait_for(boost::chrono::seconds(10)) != boost::cv_status::no_timeout) {
        std::cout << "The client did not connect to the cluster 10 seconds after disconnection as expected." << std::endl;
    }

    hz.shutdown();
    std::cout << "Finished" << std::endl;

    return 0;
}

