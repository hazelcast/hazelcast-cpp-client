/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include "NearCacheSupport.h"

using namespace hazelcast::client;

int main() {
    hazelcast::client::client_config config;
    const char *mapName = "TTLMap";
    address serverAddr("127.0.0.1", 5701);
    config.get_network_config().add_address(serverAddr);
    config::near_cache_config nearCacheConfig(mapName, config::OBJECT);
    nearCacheConfig.set_invalidate_on_change(false);
    nearCacheConfig.set_time_to_live_seconds(1);
    nearCacheConfig.get_eviction_config().set_eviction_policy(config::NONE)
            .set_maximum_size_policy(config::eviction_config::ENTRY_COUNT);
    config.add_near_cache_config(nearCacheConfig);
    auto hz = hazelcast::new_client(std::move(config)).get();

    auto map = hz.get_map(mapName).get();

    map->put<int, std::string>(1, "myValue");
    NearCacheSupport::print_near_cache_stats(map, "The put(1, article) call has no effect on the empty Near Cache");

    map->get<int, std::string>(1).get();
    NearCacheSupport::print_near_cache_stats(map, "The first get(1) call populates the Near Cache");

    map->get<int, std::string>(1).get();
    NearCacheSupport::print_near_cache_stats(map, "The second get(1) call is served from the Near Cache");

    std::this_thread::sleep_for(std::chrono::seconds(2));
    printf("We've waited for the time-to-live-seconds, so the Near Cache entry is expired.");

    map->get<int, std::string>(1).get();
    NearCacheSupport::print_near_cache_stats(map, "The third get(1) call is fetching the value again from the map");

    std::cout << "Finished" << std::endl;

    return 0;
}
