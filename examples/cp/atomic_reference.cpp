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

    // Get an atomic_reference named 'my-ref'
    auto ref = hz.get_cp_subsystem().get_atomic_reference("my-ref").get();

    // Set the value atomically
    ref->set(42).get();
    // Read the value
    auto value = ref->get<int>().get();
    std::cout << "Value:" << value << "\n";
    // Prints:
    // Value: 42
    // Try to replace the value with 'value'
    // with a compare-and-set atomic operation
    auto result = ref->compare_and_set(42, "value").get();
    std::cout << "CAS result:" << result << "\n";
    // Prints:
    // CAS result: 1

    std::cout << "Finished" << std::endl;

    return 0;
}
