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

    // Get an AtomicLong called 'my-atomic-long'
    auto atomic_counter = hz.get_cp_subsystem().get_atomic_long("my-atomic-long").get();

    // also present the future continuation capability. you can use sync version as well.
    // Get current value (returns a int64_t)
    auto future =
      atomic_counter->get().then(boost::launch::deferred, [=](boost::future<int64_t> f) {
          // Prints:
          // Value: 0
          std::cout << "Value: " << f.get() << "\n";

          // Increment by 42
          auto value = atomic_counter->add_and_get(42).get();
          std::cout << "New value: " << value << "\n";

          // Set to 0 atomically if the current value is 42
          auto result = atomic_counter->compare_and_set(42, 0).get();
          std::cout << "result: " << result << "\n";
          // Prints:
          // CAS operation result: 1
      });

    future.get();

    std::cout << "Finished" << std::endl;

    return 0;
}
