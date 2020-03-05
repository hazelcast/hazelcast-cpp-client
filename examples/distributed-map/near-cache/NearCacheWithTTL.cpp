/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
    hazelcast::client::ClientConfig config;
    const char *mapName = "TTLMap";
    Address serverAddr("127.0.0.1", 5701);
    config.addAddress(serverAddr);
    std::shared_ptr<config::NearCacheConfig<int, std::string> > nearCacheConfig(
            new config::NearCacheConfig<int, std::string>(mapName, config::OBJECT));
    nearCacheConfig->setInvalidateOnChange(false);
    nearCacheConfig->setTimeToLiveSeconds(1);
    nearCacheConfig->getEvictionConfig()->setEvictionPolicy(config::NONE)
            .setMaximumSizePolicy(config::EvictionConfig<int, std::string>::ENTRY_COUNT);
    config.addNearCacheConfig(nearCacheConfig);
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<int, std::string> map = hz.getMap<int, std::string>(mapName);

    map.put(1, "myValue");
    NearCacheSupport::printNearCacheStats(map, "The put(1, article) call has no effect on the empty Near Cache");

    map.get(1);
    NearCacheSupport::printNearCacheStats(map, "The first get(1) call populates the Near Cache");

    map.get(1);
    NearCacheSupport::printNearCacheStats(map, "The second get(1) call is served from the Near Cache");

    hazelcast::util::sleep(2);
    printf("We've waited for the time-to-live-seconds, so the Near Cache entry is expired.");

    map.get(1);
    NearCacheSupport::printNearCacheStats(map, "The third get(1) call is fetching the value again from the map");

    std::cout << "Finished" << std::endl;

    return 0;
}
