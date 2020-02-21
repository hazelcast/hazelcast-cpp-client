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
    // Get a Blocking Queue called "my-distributed-queue"
    IQueue<std::string> queue = hz.getQueue<std::string>("my-distributed-queue");
    // Offer a String into the Distributed Queue
    queue.offer("item");
    // Poll the Distributed Queue and return the String
    std::shared_ptr<std::string> item = queue.poll();
    //Timed blocking Operations
    queue.offer("anotheritem", 500);
    std::shared_ptr<std::string> anotherItem = queue.poll(5 * 1000);
    //Indefinitely blocking Operations
    queue.put("yetanotheritem");
    std::cout << *queue.take() << std::endl;
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}
