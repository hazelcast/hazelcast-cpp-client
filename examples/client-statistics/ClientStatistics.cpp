/*
 * Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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

#include <hazelcast/client/hazelcast_client.h>

using namespace hazelcast::client;

/**
 * This example demonstrates how to enable client statistics.
 */
int
main()
{
    client_config config;

    config.set_property("hazelcast.client.statistics.enabled", "true");

    /**
     * Collect and send statistics every 5 seconds
     */
    config.set_property("hazelcast.client.statistics.period.seconds", "5");

    config.add_near_cache_config(config::near_cache_config("MyMap"));
    auto hz = hazelcast::new_client(std::move(config)).get();

    auto map = hz.get_map("MyMap").get();

    map->put(2, 500).get();

    // generate a near-cache miss
    map->get<int, int>(2).get();

    // generate two near-cache hits
    map->get<int, int>(2).get();
    map->get<int, int>(2).get();

    // sleep more than the statistics collection time and keep the client
    // running. Statistics is now populated at the member side, so you can see
    // them at the Management Center.
    std::this_thread::sleep_for(std::chrono::seconds(100));

    std::cout << "Finished" << std::endl;

    return 0;
}
