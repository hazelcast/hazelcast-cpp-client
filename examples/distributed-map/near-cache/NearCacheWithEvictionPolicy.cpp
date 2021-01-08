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
#include "NearCacheSupport.h"

using namespace hazelcast::client;

int main() {
    client_config config;
    const char *mapName = "EvictionPolicyMap";
    config::near_cache_config nearCacheConfig(mapName, config::OBJECT);
    nearCacheConfig.set_invalidate_on_change(false);
    nearCacheConfig.get_eviction_config().set_eviction_policy(config::LRU)
            .set_maximum_size_policy(config::eviction_config::ENTRY_COUNT).set_size(100);
    config.add_near_cache_config(nearCacheConfig);
    hazelcast_client client(std::move(config));
    client.start().get();

    auto map = client.get_map(mapName).get();

    for (int i = 1; i <= 100; i++) {
        map->put(i, std::string{"foo-"} + std::to_string(i)).get();
    }
    NearCacheSupport::print_near_cache_stats(map, "The put(1..100, article) calls have no effect on the empty Near Cache");

    for (int i = 1; i <= 100; i++) {
        map->get<int, std::string>(i).get();
    }
    NearCacheSupport::print_near_cache_stats(map, "The first get(1..100) calls populate the Near Cache");

    for (int i = 1; i <= 100; i++) {
        map->get<int, std::string>(i).get();
    }
    NearCacheSupport::print_near_cache_stats(map, "The second get(1..100) calls are served from the Near Cache");

    map->put<int, std::string>(101, "bar").get();
    NearCacheSupport::print_near_cache_stats(map, "The put(101, \"bar\") call has no effect on the populated Near Cache");

    map->get<int, std::string>(101).get();
    NearCacheSupport::print_near_cache_stats(map,
                                          "The first get(101) call triggers the eviction and population of the Near Cache");

    NearCacheSupport::wait_for_near_cache_eviction_count(map, 1);
    NearCacheSupport::print_near_cache_stats(map, "The Near Cache has been evicted");

    map->get<int, std::string>(101).get();
    NearCacheSupport::print_near_cache_stats(map, "The second get(101) call is served from the Near Cache");


    std::cout << "Finished" << std::endl;

    return 0;
}
