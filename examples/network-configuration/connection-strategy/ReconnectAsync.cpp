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
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/config/ClientConnectionStrategyConfig.h>

int main() {
    hazelcast::client::ClientConfig config;

    /**
     * How a client reconnect to cluster after a disconnect can be configured. This parameter is used by default strategy and
     * custom implementations may ignore it if configured.
     * default value is {@link ReconnectMode#ON}
     *
     * This example forces client NOT to reconnect if it ever disconnects from the cluster.
     */
    config.getConnectionStrategyConfig().setReconnectMode(hazelcast::client::config::ClientConnectionStrategyConfig::ASYNC);

    hazelcast::client::HazelcastClient hz(config);

    // it is guaranteed to be connected at this point but if the client somehow disconnects from the cluster, all
    // invocations will receive HazelcastOfflineException until the cluster is connected.

    hazelcast::client::IMap<int, int> map = hz.getMap<int, int>("MyMap");

    map.put(1, 100);

    std::cout << "Finished" << std::endl;

    return 0;
}

