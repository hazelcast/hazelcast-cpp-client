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
#include <hazelcast/client/IAtomicLong.h>

int main() {
    hazelcast::client::HazelcastClient hz;

    hazelcast::client::IAtomicLong counter = hz.getIAtomicLong("counter");

    // Initiate an increment for the atomic long but do not block
    auto future = counter.incrementAndGetAsync();

    // Do some other work

    // Get the result of the incrementAndGetAsync api using the future
    std::shared_ptr<int64_t> result = future.get();

    // It will print the value as 1
    std::cout << "The counter value is " << *result << std::endl;

    future = counter.getAndIncrementAsync();

    // This will print 1
    std::cout << "Result of getAndIncrementAsync is " << *future.get() << std::endl;

    // Initiate a get call
    future = counter.getAsync();

    // do some other stuff

    // Obtain the result of the getAsync call. It will print the value as 2.
    std::cout << "The counter value is " << *future.get() << std::endl;

    // Initiate a get and add operation to increase the counter by 5
    future = counter.getAndAddAsync(5);

    // do some other stuff

    // Obtain the result of the getAndAddAsync call. It will print the value as 2.
    std::cout << "The counter value is " << *future.get() << std::endl;

    // This will print the updated value as 7
    std::cout << "The counter value is " << counter.get() << std::endl;

    // Set the counter value to 100 in an unblocking way
    auto voidFuture = counter.setAsync(100);

    // do some othee stuff

    // wait until the setAsync call is completed
    voidFuture.get();

    // This will print the updated value as 100
    std::cout << "The counter value is " << counter.get() << std::endl;

    auto f = counter.decrementAndGetAsync();
    // Use a callback to write the result of decrement operation in a non-blocking async way
    f.then([=](boost::future<std::shared_ptr<int64_t>> f) {
        try {
            auto response = f.get();
            if (f.get()) {
                std::cout << "Received response is : " << *response << std::endl;
            } else {
                std::cout << "Received null response" << std::endl;
            }
        } catch (hazelcast::client::exception::IException &ie) {
            std::cerr << "A failure occured. The exception is:" << ie << std::endl;
        }
    });

    std::cout << "Finished" << std::endl;

    return 0;
}
