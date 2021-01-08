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

    auto map = hz.get_multi_map("map").get();

    for (auto &key : map->key_set<std::string>().get()) {
        std::cout << key << " -> (";
        for (auto &value : map->get<std::string, std::string>(key).get()) {
            std::cout << value << ", \n";
        }
        std::cout << ")" << "\n";
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
