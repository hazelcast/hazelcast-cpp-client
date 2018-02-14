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
    const char *mapName = "BinaryMap";
    Address serverAddr("127.0.0.1", 5701);
    config.addAddress(serverAddr);
    boost::shared_ptr<config::NearCacheConfig<int, std::string> > nearCacheConfig(
            new config::NearCacheConfig<int, std::string>(mapName, config::BINARY));
    nearCacheConfig->setInvalidateOnChange(false);
    nearCacheConfig->getEvictionConfig()->setEvictionPolicy(config::NONE)
            .setMaximumSizePolicy(config::EvictionConfig<int, std::string>::ENTRY_COUNT);
    config.addNearCacheConfig(nearCacheConfig);
    HazelcastClient client(config);

    IMap<int, std::string> map = client.getMap<int, std::string>(mapName);

    // the first get() will populate the Near Cache
    boost::shared_ptr<std::string> firstGet = map.get(1);
    // the second and third get() will be served from the Near Cache
    boost::shared_ptr<std::string> secondGet = map.get(1);
    boost::shared_ptr<std::string> thirdGet = map.get(1);

    NearCacheSupport::printNearCacheStats(map);

    printf("Since we use in-memory format BINARY, the instances from the Near Cache will be identical.\n");
    printf("Compare first and second instance: %d\n", (firstGet == secondGet));
    printf("Compare second and third instance: %d\n", (secondGet == thirdGet));

    std::cout << "Finished" << std::endl;

    return 0;
}
