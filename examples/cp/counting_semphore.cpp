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

#include <hazelcast/client/hazelcast_client.h>

int
main()
{
    auto hz = hazelcast::new_client().get();

    // Get counting_semaphore named 'my-semaphore'
    auto semaphore = hz.get_cp_subsystem().get_semaphore("my-semaphore").get();
    // Try to initialize the semaphore
    // (does nothing if the semaphore is already initialized)
    semaphore->init(3).get();
    // Acquire 3 permits out of 3
    semaphore->acquire(3).get();
    // Release 2 permits
    semaphore->release(2).get();
    // Check available permits
    auto available = semaphore->available_permits().get();
    std::cout << "Available:" << available << "\n";
    // Prints:
    // Available: 1

    // Try to acquire 1 permit
    auto success = semaphore->try_acquire(1).get();
    // Check for valid fencing token
    if (success) {
        try {
            // Your guarded code goes here
        } catch (...) {
            // Make sure to release the permits
            semaphore->release(1).get();
        }
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
