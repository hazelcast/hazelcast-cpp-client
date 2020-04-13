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

    hazelcast::client::IMap<std::string, std::string> map =
            hz.getMap<std::string, std::string>("themap");

    // initiate map put in an unblocking way
    auto future = map.putAsync("key", "value");

    // later on get the result of the put operation
    std::shared_ptr<std::string> result = future.get();
    if (result.get()) {
        std::cout << "There was a previous value for key. The value was:" << *result << std::endl;
    } else {
        std::cout << "There was no previous value for key." << std::endl;
    }

    // Get the value for key unblocking
    future = map.getAsync("key");

    // Let the callback handle the response when received and print the appropriate message
    // The callback will be called using the user executor thread.
    future.then([=](boost::future<std::shared_ptr<std::string>> f) {
        try {
            std::cout << "Response was received. ";
            auto result = f.get();
            if (result) {
                std::cout << "Received response is : " << *result << std::endl;
            } else {
                std::cout << "Received null response" << std::endl;
            }
        } catch (hazelcast::client::exception::IException &e) {

        }
    });

    // Set the value to a new value in an unblocking manner
    map.setAsync("key", "value2", 5, hazelcast::util::concurrent::TimeUnit::SECONDS());

    // Remove the key unblocking
    future = map.removeAsync("key");

    std::cout << "Finished" << std::endl;

    return 0;
}
