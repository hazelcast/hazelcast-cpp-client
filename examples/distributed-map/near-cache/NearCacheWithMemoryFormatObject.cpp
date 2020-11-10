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
    const char *mapName = "ObjectMap";
    address serverAddr("127.0.0.1", 5701);
    config.get_network_config().add_address(serverAddr);
    config::near_cache_config nearCacheConfig(mapName, config::OBJECT);
    nearCacheConfig.set_invalidate_on_change(false);
    nearCacheConfig.get_eviction_config().set_eviction_policy(config::NONE)
            .set_maximum_size_policy(config::eviction_config::ENTRY_COUNT);
    config.add_near_cache_config(nearCacheConfig);
    hazelcast_client client(config);

    auto map = client.get_map(mapName);

    auto firstGet = map->get<int, std::string>(1).get();
    // the second and third get() will be served from the Near Cache
    auto secondGet = map->get<int, std::string>(1).get();
    auto thirdGet = map->get<int, std::string>(1).get();

    NearCacheSupport::print_near_cache_stats(map);

    printf("Since we use in-memory format OBJECT, the instances from the Near Cache will be identical.\n");
    printf("Compare first and second instance: %d\n", (firstGet == secondGet));
    printf("Compare second and third instance: %d\n", (secondGet == thirdGet));

    std::cout << "Finished" << std::endl;

    return 0;
}
