/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
    // Get the Distributed Map from Cluster.
    auto map = hz.get_map("my-distributed-map").get();
    // Standard Put and Get.
    map->put<std::string, std::string>("key", "value").get();
    map->get<std::string, std::string>("key").get();
    // Concurrent Map methods, optimistic updating
    map->put_if_absent<std::string, std::string>("somekey", "somevalue").get();
    // use deferred future continuation
    auto future =
      map->replace<std::string, std::string>("key", "value", "newvalue")
        .then(boost::launch::deferred, [](boost::future<bool> f) {
            if (f.get()) {
                std::cout << "Replaced successfully\n";
                return;
            }
            std::cerr << "Failed to replace\n";
        });

    // Wait until replace is completed
    future.get();

    // Shutdown this Hazelcast Client
    hz.shutdown().get();

    return 0;
}
