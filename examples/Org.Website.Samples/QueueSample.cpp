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
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    auto hz = hazelcast::new_client().get();
    // Get a Blocking Queue called "my-distributed-queue"
    auto queue = hz.get_queue("my-distributed-queue").get();
    // Offer a String into the Distributed Queue
    queue->offer("item").get();
    // Poll the Distributed Queue and return the String
    auto item = queue->poll<std::string>().get();
    //Timed blocking Operations
    queue->offer("anotheritem", std::chrono::milliseconds(500)).get();
    auto anotherItem = queue->poll<std::string>(std::chrono::seconds(5)).get();
    //Indefinitely blocking Operations
    queue->put("yetanotheritem");
    std::cout << *queue->take<std::string>().get() << std::endl;
    // Shutdown this Hazelcast Client
    hz.shutdown().get();

    return 0;
}
