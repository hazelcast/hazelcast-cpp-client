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

#include <hazelcast/client/hazelcast_client.h>

int main() {
    hazelcast_client hz;

    // Get a latch called 'my-latch'
    auto latch = hz.get_cp_subsystem().get_latch("my-latch'").get();

    // Try to initialize the latch
    // (does nothing if the count is not zero)
    auto initialized = latch->try_set_count(1).get();
    std::cout << "Initialized:" << initialized << "\n";
    // Check count
    auto count = latch->get_count().get();
    std::cout << "Count:" << count << "\n";
    // Prints:
    // Count: 1
    // Bring the count down to zero after 10ms
    auto f = std::async([=] () {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        latch->count_down().get();
    });
    // Wait up to 1 second for the count to become zero up
    auto status = latch->wait_for(std::chrono::seconds(1)).get();
    if (status == std::cv_status::no_timeout) {
        std::cout << "latch is count down\n";
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
