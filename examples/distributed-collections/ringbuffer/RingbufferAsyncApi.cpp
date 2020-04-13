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

#include <hazelcast/client/HazelcastClient.h>

int main() {
    hazelcast::client::HazelcastClient hz;

    std::shared_ptr<hazelcast::client::Ringbuffer<std::string> > rb = hz.getRingbuffer<std::string>("myringbuffer");

    // add an item in an unblocking way
    auto future = rb->addAsync("new item", hazelcast::client::Ringbuffer<std::string>::OVERWRITE);

    // let the result processed by a callback
    future.then([=](boost::future<std::shared_ptr<int64_t>> f) {
        try {
            std::cout << "The sequence id of the added item is " << *f.get() << std::endl;
        } catch (hazelcast::client::exception::IException &e) {
            std::cerr << "The response is a failure with exception:" << e << std::endl;
        }
    });

    std::vector<std::string> items;
    items.push_back("item2");
    items.push_back("item3");
    // Add items in a non-blocking way
    future = rb->addAllAsync(items, hazelcast::client::Ringbuffer<std::string>::OVERWRITE);

    // do some other work

    // wait for the addAllAsync to complete and print the sequenceId of the last written item.
    std::cout << "Sequence id of the last written item is :" << *future.get() << std::endl;

    auto resultSetFuture = rb->readManyAsync<void>(0, 2, 3, NULL);

    // do some other work

    // get the result set
    auto readItems = resultSetFuture.get();
    std::cout << "Read " << readItems->readCount() << " items." << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
