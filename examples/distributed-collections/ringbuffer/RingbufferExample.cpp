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
//
// Created by İhsan Demir on 21/12/15.
//
#include <hazelcast/client/HazelcastClient.h>

int main() {
    hazelcast::client::HazelcastClient hz;

    std::shared_ptr<hazelcast::client::Ringbuffer<std::string> > rb = hz.getRingbuffer<std::string>("myringbuffer");

    std::cout << "Capacity of the ringbuffer is:" << rb->capacity() << std::endl;

    int64_t sequenceNumber = rb->add("First Item");

    std::cout << "Added the first item at sequence " << sequenceNumber << std::endl;

    rb->add("Second item");

    std::cout << "There are " << rb->size() << " items in the ring buffer " << std::endl;

    std::unique_ptr<std::string> val = rb->readOne(sequenceNumber);

    if ((std::string *)NULL != val.get()) {
        std::cout << "The item at read at sequence " << sequenceNumber << " is " << *val << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
