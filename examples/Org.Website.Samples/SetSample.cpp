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
    // Get the Distributed Set from Cluster.
    auto set = hz.get_set("my-distributed-set").get();
    // Add items to the set with duplicates
    set->add("item1").get();
    set->add("item1").get();
    set->add("item2").get();
    set->add("item2").get();
    set->add("item2").get();
    set->add("item3").get();
    // Get the items. Note that there are no duplicates.
    auto future = set->to_array<std::string>().then(boost::launch::deferred, [] (boost::future<std::vector<std::string>> f) {
        for(auto &v : f.get()) {
            std::cout << v << '\n';
        }
    });

    // do something else
    //...
    // print the output
    future.get();

    // Shutdown this Hazelcast Client
    hz.shutdown().get();

    return 0;
}
