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
#include <hazelcast/client/hazelcast_client.h>

int main() {
    hazelcast::client::hazelcast_client hz;

    hz.start().get();

    auto map = hz.get_map("evictiontestmap").get();

    int numberOfKeysToLock = 4;
    int numberOfEntriesToAdd = 1000;

    for (int i = 0; i < numberOfEntriesToAdd; i++) {
        map->put(i, i).get();
    }

    for (int i = 0; i < numberOfKeysToLock; i++) {
        map->lock(i).get();
    }

    std::cout << "# Map size before evict_all\t:" << map->size().get() << std::endl;

    // should keep locked keys and evict all others.
    map->evict_all().get();

    std::cout << "# After calling evict_all..." << std::endl;
    std::cout << "# Expected map size\t: " << numberOfKeysToLock << std::endl;
    std::cout << "# Actual map size\t: " << map->size().get() << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
