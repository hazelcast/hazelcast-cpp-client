/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/adaptor/RawPointerQueue.h>

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IQueue<std::string> q = hz.getQueue<std::string>("queue");
    hazelcast::client::adaptor::RawPointerQueue<std::string> queue(q);
    queue.offer("Tokyo");
    queue.offer("Paris");
    queue.offer("New York");
    std::cout << "Finished loading queue" << std::endl;

    std::auto_ptr<hazelcast::client::adaptor::DataArray<std::string> > vals = queue.toArray();

    std::cout << "There are " << queue.size() << " values in the queue" << std::endl;

    for (size_t i = 0; i < vals->size(); ++i) {
        std::auto_ptr<std::string> val = vals->get(i);
        if (NULL == val.get()) {
            std::cout << "Value " << i << " is NULL" << std::endl;
        } else {
            std::cout << "Value: " << *val << std::endl;
        }
    }
    
    std::auto_ptr<std::string> item = queue.peek();
    if (NULL == item.get()) {
        std::cout << "Head of the queue is NULL" << std::endl;
    } else {
        std::cout << "Head of the queue is " << *item << std::endl;
    }

    item = queue.take();
    if (NULL == item.get()) {
        std::cout << "Take operation returned NULL" << std::endl;
    } else {
        std::cout << "Take operation returned " << *item << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
