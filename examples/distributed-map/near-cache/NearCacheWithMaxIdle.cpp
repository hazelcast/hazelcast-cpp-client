/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include <stdio.h>

#include "NearCacheSupport.h"

using namespace hazelcast::client;

int
main()
{
    client_config config;
    const char* mapName = "MaxIdleMap";
    address serverAddr("127.0.0.1", 5701);
    config.get_network_config().add_address(serverAddr);
    config::near_cache_config nearCacheConfig(mapName, config::OBJECT);
    nearCacheConfig.set_invalidate_on_change(false);
    nearCacheConfig.get_eviction_config()
      .set_eviction_policy(config::NONE)
      .set_maximum_size_policy(config::eviction_config::ENTRY_COUNT);
    nearCacheConfig.set_max_idle_seconds(1);
    config.add_near_cache_config(nearCacheConfig);
    hazelcast_client hz{ hazelcast::new_client(std::move(config)).get() };

    auto map = hz.get_map(mapName).get();

    map->put<int, std::string>(1, "foo").get();
    NearCacheSupport::print_near_cache_stats(
      map, "The put(1, article) call has no effect on the empty Near Cache");

    map->get<int, std::string>(1).get();
    NearCacheSupport::print_near_cache_stats(
      map, "The first get(1) call populates the Near Cache");

    // with this short sleep time, the Near Cache entry should not expire
    for (int i = 0; i < 20; i++) {
        map->get<int, std::string>(1).get();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    NearCacheSupport::print_near_cache_stats(
      map,
      "We have called get(1) every 100 ms, so the Near cache entry could not "
      "expire");

    std::this_thread::sleep_for(std::chrono::seconds(2));
    printf("We've waited for max-idle-seconds, so the Near Cache entry is "
           "expired.\n");

    map->get<int, std::string>(1).get();
    NearCacheSupport::print_near_cache_stats(
      map, "The next get(1) call is fetching the value again from the map");

    std::cout << "Finished" << std::endl;

    return 0;
}
