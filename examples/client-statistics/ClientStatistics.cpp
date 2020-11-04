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
#include <stdint.h>

#include <hazelcast/client/HazelcastClient.h>

using namespace hazelcast::client;

/**
 * Important note: Please make sure a cluster is running and management center is enabled at the members and url's
 * point to the management center url where the management center is started. See the example configuration file
 * `hazelcast-management-center-enabled.xml` for member configuration.
 *
 */
int main() {
    ClientConfig config;

    config.setProperty("hazelcast.client.statistics.enabled", "true");

    /**
     * Collect and send statistics every 5 seconds
     */
    config.setProperty("hazelcast.client.statistics.period.seconds", "5");

    config.addNearCacheConfig(config::NearCacheConfig("MyMap"));
    hazelcast::client::HazelcastClient hz(config);

    auto map = hz.getMap("MyMap");
    
    map->put(2, 500).get();

    // generate a near-cache miss
    map->get<int, int>(2).get();

    // generate two near-cache hits
    map->get<int, int>(2).get();
    map->get<int, int>(2).get();

    // sleep more than the statistics collection time and keep the client running. Statistics is now populated at the
    // member side, so you can see them at the Management Center.
    std::this_thread::sleep_for(std::chrono::seconds(100));

    std::cout << "Finished" << std::endl;

    return 0;
}
