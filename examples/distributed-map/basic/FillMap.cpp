/*
 * Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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

int
main()
{
    auto hz = hazelcast::new_client().get();

    auto map = hz.get_map("map").get();
    map->put<std::string, std::string>("1", "Tokyo").get();
    map->put<std::string, std::string>("2", "Paris").get();
    map->put<std::string, std::string>("3", "New York").get();
    std::cout << "Finished loading map" << std::endl;

    auto binaryMap = hz.get_map("MyBinaryMap").get();
    std::vector<char> value(100);
    binaryMap->put(3, value).get();
    std::cout
      << "Inserted an entry with key 3 and a binary value to the binary map->"
      << std::endl;

    auto valueFromMap = binaryMap->get<int32_t, std::vector<char>>(3).get();
    if (valueFromMap) {
        std::cout << "The binary map returned a binary array of size "
                  << valueFromMap->size() << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
