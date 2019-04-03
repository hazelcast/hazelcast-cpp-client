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
    // Get an Atomic Counter, we'll call it "counter"
    IAtomicLong counter = hz.getIAtomicLong("counter");
    // Add and Get the "counter"
    counter.addAndGet(3); // value is now 3
    // Display the "counter" value
    std::cout << "counter: " << counter.get() << std::endl;
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}
