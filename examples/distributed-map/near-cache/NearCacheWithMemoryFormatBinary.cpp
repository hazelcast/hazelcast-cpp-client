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
#include <stdio.h>

#include "NearCacheSupport.h"

using namespace hazelcast::client;

int main() {
    ClientConfig config;
    const char *mapName = "BinaryMap";
    Address serverAddr("127.0.0.1", 5701);
    config.getNetworkConfig().addAddress(serverAddr);
    std::shared_ptr<config::NearCacheConfig<int, std::string> > nearCacheConfig(
            new config::NearCacheConfig<int, std::string>(mapName, config::BINARY));
    nearCacheConfig->setInvalidateOnChange(false);
    nearCacheConfig->getEvictionConfig()->setEvictionPolicy(config::NONE)
            .setMaximumSizePolicy(config::EvictionConfig<int, std::string>::ENTRY_COUNT);
    config.addNearCacheConfig(nearCacheConfig);
    HazelcastClient client(config);

    auto map = client.getMap(mapName);

    // the first get() will populate the Near Cache
    auto firstGet = map->get<int, std::string>(1).get();
    // the second and third get() will be served from the Near Cache
    auto secondGet = map->get<int, std::string>(1).get();
    auto thirdGet = map->get<int, std::string>(1).get();

    NearCacheSupport::printNearCacheStats(map);

    printf("Since we use in-memory format BINARY, the instances from the Near Cache will be identical.\n");
    printf("Compare first and second instance: %d\n", (firstGet == secondGet));
    printf("Compare second and third instance: %d\n", (secondGet == thirdGet));

    std::cout << "Finished" << std::endl;

    return 0;
}
