/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
    auto hz = hazelcast::new_client().get();

    // Get an fenced_lock named 'my-lock'
    auto lock = hz.get_cp_subsystem().get_lock("my-lock").get();

    // Acquire the lock
    lock->lock().get();
    try {
        // Your guarded code goes here
    } catch (...) {
        // Make sure to release the lock
        lock->unlock().get();
    }

    // Try to acquire the lock
    auto success = lock->try_lock().get();
    // Check for valid fencing token
    if (success) {
        try {
            // Your guarded code goes here
        } catch (...) {
            // Make sure to release the lock
            lock->unlock().get();
        }
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
