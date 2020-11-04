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
    ClientConfig config;
    const char *mapName = "InvalidationMap";
    config::NearCacheConfig nearCacheConfig(mapName, config::OBJECT);
    nearCacheConfig.setInvalidateOnChange(true);
    nearCacheConfig.getEvictionConfig().setEvictionPolicy(config::NONE)
            .setMaximumSizePolicy(config::EvictionConfig::ENTRY_COUNT);
    config.addNearCacheConfig(nearCacheConfig);
    HazelcastClient client(config);

    HazelcastClient noNearCacheclient;

    auto map = client.getMap(mapName);
    auto noNearCacheMap = noNearCacheclient.getMap(mapName);

    noNearCacheMap->put<int, std::string>(1, "foo").get();
    NearCacheSupport::printNearCacheStats(map, "The noNearCacheMap->put(key, \"foo\")) call has no effect on the Near Cache of map");

    map->get<int, std::string>(1).get();
    NearCacheSupport::printNearCacheStats(map, "The first get(1) call populates the Near Cache");

    noNearCacheMap->put<int, std::string>(1, "foo").get();
    NearCacheSupport::printNearCacheStats(map, "The noNearCacheMap->put(key, \"foo\")) call will invalidate the Near Cache of map");

    NearCacheSupport::waitForInvalidationEvents();
    NearCacheSupport::printNearCacheStats(map, "The Near Cache of map is empty after the invalidation event has been processed");

    map->get<int, std::string>(1).get();
    NearCacheSupport::printNearCacheStats(map, "The next map1.get(key) call populates the Near Cache again");

    std::cout << "Finished" << std::endl;

    return 0;
}
