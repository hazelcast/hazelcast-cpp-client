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
#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;
int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    HazelcastClient hz;
    auto rb = hz.getRingbuffer("rb");
    // add two items into ring buffer
    rb->add(100).get();
    rb->add(200).get();
    // we start from the oldest item.
    // if you want to start from the next item, call rb.tailSequence()+1
    int64_t sequence = rb->headSequence().get();
    std::cout << *rb->readOne<int>(sequence).get() << std::endl;
    sequence++;
    std::cout << *rb->readOne<int>(sequence).get() << std::endl;
    // Shutdown this Hazelcast Client
    hz.shutdown();

}
