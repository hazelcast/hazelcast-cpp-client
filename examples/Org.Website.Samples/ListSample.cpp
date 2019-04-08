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
#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    HazelcastClient hz;
    // Get the Distributed List from Cluster.
    IList<std::string> list = hz.getList<std::string>("my-distributed-list");
    // Add elements to the list
    list.add("item1");
    list.add("item2");

    // Remove the first element
    std::cout << "Removed: " << *list.remove(0);
    // There is only one element left
    std::cout << "Current size is " << list.size() << std::endl;
    // Clear the list
    list.clear();
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}
