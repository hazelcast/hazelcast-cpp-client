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

class MapChangeListener : public hazelcast::client::EntryListener {
public:
    void entryAdded(const hazelcast::client::EntryEvent &event) override {
            std::cout << "Entry added:" << event.getKey().get<int>().value();
    }

    void entryRemoved(const hazelcast::client::EntryEvent &event) override {
        std::cout << "Entry removed:" << event.getKey().get<int>().value();
    }

    void entryUpdated(const hazelcast::client::EntryEvent &event) override {
        std::cout << "Entry updated:" << event.getKey().get<int>().value();
    }

    void entryEvicted(const hazelcast::client::EntryEvent &event) override {
        std::cout << "Entry evicted:" << event.getKey().get<int>().value();
    }

    void entryExpired(const hazelcast::client::EntryEvent &event) override {
        std::cout << "Entry expired:" << event.getKey().get<int>().value();
    }

    void entryMerged(const hazelcast::client::EntryEvent &event) override {
        std::cout << "Entry merged:" << event.getKey().get<int>().value();
    }

    void mapEvicted(const hazelcast::client::MapEvent &event) override {
        std::cout << "Map evicted:" << event.getName();
    }

    void mapCleared(const hazelcast::client::MapEvent &event) override {
        std::cout << "Map cleared:" << event.getName();
    }
};

int main() {
    hazelcast::client::ClientConfig config;

    /**
     * Number of the threads to handle the incoming event packets.
     * The default is 5.
     *
     * We force the clisnt use only a single thread for events in this example.
     */
    config.setProperty("hazelcast.client.event.thread.count", "1");

    /**
     * Capacity of the executor that handles the incoming event packets.
     *
     * The default value is 1000000.
     *
     * This example sets the max capacity to 50000.
     */
    config.setProperty("hazelcast.client.event.queue.capacity", "50000");

    hazelcast::client::HazelcastClient hz(config);

    auto map = hz.getMap("MyMap");

    map->addEntryListener(MapChangeListener(), false).get();

    // Now we put two entries, and since there is only one event thread, they will be delivered to the entry listener,
    // from within the same thread, hence it will be a sequential delivery. Hence we should see that "Entry added:100"
    // is printed before "Entry added:200"
    map->put(1, 100).get();
    map->put(2, 200).get();

    std::cout << "Finished" << std::endl;

    hz.shutdown();

    return 0;
}
