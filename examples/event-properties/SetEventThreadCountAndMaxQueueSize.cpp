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
#include <hazelcast/client/entry_listener.h>
#include <hazelcast/client/hazelcast_client.h>

int main() {
    hazelcast::client::client_config config;

    /**
     * Number of the threads to handle the incoming event packets.
     * The default is 5.
     *
     * We force the clisnt use only a single thread for events in this example.
     */
    config.set_property("hazelcast.client.event.thread.count", "1");

    /**
     * Capacity of the executor that handles the incoming event packets.
     *
     * The default value is 1000000.
     *
     * This example sets the max capacity to 50000.
     */
    config.set_property("hazelcast.client.event.queue.capacity", "50000");

    hazelcast::client::hazelcast_client hz(std::move(config));
    hz.start().get();

    auto map = hz.get_map("MyMap").get();

    hazelcast::client::entry_listener listener;

    listener.
        on_added([](hazelcast::client::entry_event &&event) {
            std::cout << "Entry added:" << event.get_key().get<int>().value();
        }).
        on_removed([](hazelcast::client::entry_event &&event) {
            std::cout << "Entry removed:" << event.get_key().get<int>().value();
        }).
        on_updated([](hazelcast::client::entry_event &&event) {
            std::cout << "Entry updated:" << event.get_key().get<int>().value();
        }).
        on_evicted([](hazelcast::client::entry_event &&event) {
            std::cout << "Entry evicted:" << event.get_key().get<int>().value();
        }).
        on_expired([](hazelcast::client::entry_event &&event) {
            std::cout << "Entry expired:" << event.get_key().get<int>().value();
        }).
        on_merged([](hazelcast::client::entry_event &&event) {
            std::cout << "Entry merged:" << event.get_key().get<int>().value();
        }).
        on_map_evicted([](hazelcast::client::map_event &&event) {
            std::cout << "Map evicted:" << event.get_name();
        }).
        on_map_cleared([](hazelcast::client::map_event &&event) {
            std::cout << "Map cleared:" << event.get_name();
        });

    map->add_entry_listener(std::move(listener), false).get();

    // Now we put two entries, and since there is only one event thread, they will be delivered to the entry listener,
    // from within the same thread, hence it will be a sequential delivery. Hence we should see that "Entry added:100"
    // is printed before "Entry added:200"
    map->put(1, 100).get();
    map->put(2, 200).get();

    std::cout << "Finished" << std::endl;

    hz.shutdown().get();

    return 0;
}
