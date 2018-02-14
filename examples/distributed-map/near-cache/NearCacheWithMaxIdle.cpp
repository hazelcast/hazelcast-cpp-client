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
#include <stdio.h>

#include "NearCacheSupport.h"

using namespace hazelcast::client;

int main() {
    ClientConfig config;
    const char *mapName = "MaxIdleMap";
    Address serverAddr("127.0.0.1", 5701);
    config.addAddress(serverAddr);
    boost::shared_ptr<config::NearCacheConfig<int, std::string> > nearCacheConfig(
            new config::NearCacheConfig<int, std::string>(mapName, config::OBJECT));
    nearCacheConfig->setInvalidateOnChange(false);
    nearCacheConfig->getEvictionConfig()->setEvictionPolicy(config::NONE)
            .setMaximumSizePolicy(config::EvictionConfig<int, std::string>::ENTRY_COUNT);
    nearCacheConfig->setMaxIdleSeconds(1);
    config.addNearCacheConfig(nearCacheConfig);
    HazelcastClient client(config);

    IMap<int, std::string> map = client.getMap<int, std::string>(mapName);

    map.put(1, "foo");
    NearCacheSupport::printNearCacheStats(map, "The put(1, article) call has no effect on the empty Near Cache");

    map.get(1);
    NearCacheSupport::printNearCacheStats(map, "The first get(1) call populates the Near Cache");

    // with this short sleep time, the Near Cache entry should not expire
    for (int i = 0; i < 20; i++) {
        map.get(1);
        hazelcast::util::sleepmillis(100);
    }
    NearCacheSupport::printNearCacheStats(map, "We have called get(1) every 100 ms, so the Near cache entry could not expire");

    hazelcast::util::sleep(2);
    printf("We've waited for max-idle-seconds, so the Near Cache entry is expired.\n");

    map.get(1);
    NearCacheSupport::printNearCacheStats(map, "The next get(1) call is fetching the value again from the map");

    std::cout << "Finished" << std::endl;

    return 0;
}
