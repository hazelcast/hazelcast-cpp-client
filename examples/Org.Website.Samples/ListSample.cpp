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
#include <hazelcast/client/hazelcast.h>

using namespace hazelcast::client;
int
main()
{
    // Start the Hazelcast Client and connect to an already running Hazelcast
    // Cluster on 127.0.0.1
    auto hz = hazelcast::new_client().get();
    // Get the Distributed List from Cluster.
    auto list = hz.get_list("my-distributed-list").get();
    // Add elements to the list
    list->add("item1").get();
    // Using future continuation here so that the calls are not blocking
    auto f = list->add("item2").then(
      boost::launch::deferred, [=](boost::future<bool> f) {
          if (!f.get()) {
              std::cerr << "Element 2 could not be added !!!\n";
              return;
          }

          std::cout << std::boolalpha;
          // Remove the first element
          std::cout << "Removed: " << list->remove(0).get();
          // There is only one element left
          std::cout << "Current size is " << list->size().get() << '\n';
          // Clear the list
          list->clear().get();
      });

    // make the deferred future execute
    f.get();

    // Shutdown this Hazelcast Client
    hz.shutdown().get();

    return 0;
}
