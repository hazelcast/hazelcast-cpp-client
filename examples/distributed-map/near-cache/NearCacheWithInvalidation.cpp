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
    const char *mapName = "InvalidationMap";
    config::near_cache_config nearCacheConfig(mapName, config::OBJECT);
    nearCacheConfig.set_invalidate_on_change(true);
    nearCacheConfig.get_eviction_config().set_eviction_policy(config::NONE)
            .set_maximum_size_policy(config::eviction_config::ENTRY_COUNT);
    config.add_near_cache_config(nearCacheConfig);
    hazelcast_client client(std::move(config));
    client.start().get();

    hazelcast_client noNearCacheclient;

    auto map = client.get_map(mapName).get();
    auto noNearCacheMap = noNearCacheclient.get_map(mapName).get();

    noNearCacheMap->put<int, std::string>(1, "foo").get();
    NearCacheSupport::print_near_cache_stats(map, "The noNearCacheMap->put(key, \"foo\")) call has no effect on the Near Cache of map");

    map->get<int, std::string>(1).get();
    NearCacheSupport::print_near_cache_stats(map, "The first get(1) call populates the Near Cache");

    noNearCacheMap->put<int, std::string>(1, "foo").get();
    NearCacheSupport::print_near_cache_stats(map, "The noNearCacheMap->put(key, \"foo\")) call will invalidate the Near Cache of map");

    NearCacheSupport::wait_for_invalidation_events();
    NearCacheSupport::print_near_cache_stats(map, "The Near Cache of map is empty after the invalidation event has been processed");

    map->get<int, std::string>(1).get();
    NearCacheSupport::print_near_cache_stats(map, "The next map1.get(key) call populates the Near Cache again");

    std::cout << "Finished" << std::endl;

    return 0;
}
