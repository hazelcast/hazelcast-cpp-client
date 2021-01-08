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
#include <hazelcast/client/hazelcast.h>

using namespace hazelcast::client;
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    hazelcast_client hz;
    // Get the Distributed MultiMap from Cluster.
    auto multiMap = hz.get_multi_map("my-distributed-multimap").get();
    // Put values in the map against the same key
    multiMap->put("my-key", "value1").get();
    multiMap->put("my-key", "value2").get();
    multiMap->put("my-key", "value3").get();
    // Print out all the values for associated with key called "my-key"
    multiMap->get<std::string, std::string>("my-key").then(boost::launch::deferred,
                                                           [](boost::future<std::vector<std::string>> f) {
                                                               for (auto &value : f.get()) {
                                                                   std::cout << value << '\n';
                                                               }
                                                           }).get();
    // remove specific key/value pair
    multiMap->remove("my-key", "value2").get();
    // Shutdown this Hazelcast Client
    hz.stop().get();

    return 0;
}
