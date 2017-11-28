/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/HazelcastClient.h>

using namespace hazelcast::client;

int main() {
    ClientConfig config;
    HazelcastClient client(config);

    MixedMap mixedMapProxy = *client.getMixedMap("MyMap");

    mixedMapProxy.put<int, int>(3, 5);
    mixedMapProxy.put<int, std::string>(10, "MyStringValue");
    TypedData result = mixedMapProxy.get<int>(3);
    std::cout << "Got result for key 3. Object type:" << result.getType() << " value:" << *result.get<int>()
              << std::endl;

    result = mixedMapProxy.get<int>(10);
    std::cout << "Got result for key 10. Object type:" << result.getType() << " value:" << *result.get<int>()
              << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
