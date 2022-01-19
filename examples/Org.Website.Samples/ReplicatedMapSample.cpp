/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
int
main()
{
    // Start the Hazelcast Client and connect to an already running Hazelcast
    // Cluster on 127.0.0.1
    auto hz = hazelcast::new_client().get();
    // Get a Replicated Map called "my-replicated-map"
    auto map = hz.get_replicated_map("my-replicated-map").get();
    // Add items to the set with duplicates
    // Put and Get a value from the Replicated Map
    auto replacedValue =
      map->put<std::string, std::string>("key", "value").get();
    // key/value replicated to all members
    std::cout << "replacedValue = " << replacedValue.value_or("null");
    // Will be null as its first update
    auto value = map->get<std::string, std::string>("key").get();
    // the value is retrieved from a random member in the cluster
    std::cout << "value for key = " << value.value_or("null");
    // Shutdown this Hazelcast Client
    hz.shutdown().get();

    return 0;
}
