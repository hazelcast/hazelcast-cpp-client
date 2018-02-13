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
//
// Created by ihsan demir on 13 Jan 2017.
//
#include "NearCacheSupport.h"

using namespace hazelcast::client;

int main() {
    ClientConfig config;
    const char *mapName = "InvalidationMap";
    Address serverAddr("127.0.0.1", 5701);
    config.addAddress(serverAddr);
    boost::shared_ptr<config::NearCacheConfig<int, std::string> > nearCacheConfig(
            new config::NearCacheConfig<int, std::string>(mapName, config::OBJECT));
    nearCacheConfig->setInvalidateOnChange(true);
    nearCacheConfig->getEvictionConfig()->setEvictionPolicy(config::NONE)
            .setMaximumSizePolicy(config::EvictionConfig<int, std::string>::ENTRY_COUNT);
    config.addNearCacheConfig(nearCacheConfig);
    HazelcastClient client(config);

    ClientConfig noNearCacheConfig;
    config.addAddress(serverAddr);
    HazelcastClient noNearCacheclient(noNearCacheConfig);

    IMap<int, std::string> map = client.getMap<int, std::string>(mapName);
    IMap<int, std::string> noNearCacheMap = noNearCacheclient.getMap<int, std::string>(mapName);

    noNearCacheMap.put(1, "foo");
    NearCacheSupport::printNearCacheStats(map, "The noNearCacheMap.put(key, \"foo\")) call has no effect on the Near Cache of map");

    map.get(1);
    NearCacheSupport::printNearCacheStats(map, "The first get(1) call populates the Near Cache");

    noNearCacheMap.put(1, "foo");
    NearCacheSupport::printNearCacheStats(map, "The noNearCacheMap.put(key, \"foo\")) call will invalidate the Near Cache of map");

    NearCacheSupport::waitForInvalidationEvents();
    NearCacheSupport::printNearCacheStats(map, "The Near Cache of map is empty after the invalidation event has been processed");

    map.get(1);
    NearCacheSupport::printNearCacheStats(map, "The next map1.get(key) call populates the Near Cache again");

    std::cout << "Finished" << std::endl;

    return 0;
}
