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
// Created by İhsan Demir on 21/12/15.
//
#include <hazelcast/client/HazelcastClient.h>

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<std::string, std::string> map = hz.getMap<std::string, std::string>("map");
    map.put("1", "Tokyo");
    map.put("2", "Paris");
    map.put("3", "New York");
    std::cout << "Finished loading map" << std::endl;

    hazelcast::client::IMap<int, std::vector<char> > binaryMap = hz.getMap<int, std::vector<char> >("MyBinaryMap");
    std::vector<char> value(100);
    int key = 3;
    binaryMap.put(key, value);
    std::cout << "Inserted an entry with key 3 and a binary value to the binary map." << std::endl;

    boost::shared_ptr<std::vector<char> > valueFromMap = binaryMap.get(key);
    if (NULL != valueFromMap.get()) {
        std::cout << "The binary map returned a binary array of size " << valueFromMap->size() << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
