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
#include <hazelcast/client/entry_listener.h>

hazelcast::client::entry_listener
make_listener()
{
    return hazelcast::client::entry_listener()
      .on_added([](hazelcast::client::entry_event&& event) {
          std::cout << "[added] " << event << std::endl;
      })
      .on_removed([](hazelcast::client::entry_event&& event) {
          std::cout << "[removed] " << event << std::endl;
      })
      .on_updated([](hazelcast::client::entry_event&& event) {
          std::cout << "[added] " << event << std::endl;
      })
      .on_evicted([](hazelcast::client::entry_event&& event) {
          std::cout << "[updated] " << event << std::endl;
      })
      .on_expired([](hazelcast::client::entry_event&& event) {
          std::cout << "[expired] " << event << std::endl;
      })
      .on_merged([](hazelcast::client::entry_event&& event) {
          std::cout << "[merged] " << event << std::endl;
      })
      .on_map_evicted([](hazelcast::client::map_event&& event) {
          std::cout << "[map_evicted] " << event << std::endl;
      })
      .on_map_cleared([](hazelcast::client::map_event&& event) {
          std::cout << "[map_cleared] " << event << std::endl;
      });
}

int
main()
{
    auto hz = hazelcast::new_client().get();

    auto map = hz.get_map("somemap").get();

    auto listenerId = map->add_entry_listener(make_listener(), true).get();

    std::cout << "EntryListener registered" << std::endl;

    // wait for modifymap executable to run
    std::this_thread::sleep_for(std::chrono::seconds(10));

    map->remove_entry_listener(listenerId).get();

    // Continuous Query example
    // Register listener with predicate
    // Only listen events for entries with key >= 7
    listenerId =
      map
        ->add_entry_listener(
          make_listener(),
          hazelcast::client::query::greater_less_predicate(
            hz,
            hazelcast::client::query::query_constants::KEY_ATTRIBUTE_NAME,
            7,
            true,
            false),
          true)
        .get();

    // wait for modifymap executable to run
    std::this_thread::sleep_for(std::chrono::seconds(10));

    map->remove_entry_listener(listenerId).get();

    std::cout << "Finished" << std::endl;

    return 0;
}
