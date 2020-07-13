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

class MyEntryListener : public hazelcast::client::EntryListener {
public:
    void entryAdded(const hazelcast::client::EntryEvent &event) override {
        std::cout << "[entryAdded] " << event << std::endl;
    }

    void entryRemoved(const hazelcast::client::EntryEvent &event) override {
        std::cout << "[entryRemoved] " << event << std::endl;
    }

    void entryUpdated(const hazelcast::client::EntryEvent &event) override {
        std::cout << "[entryAdded] " << event << std::endl;
    }

    void entryEvicted(const hazelcast::client::EntryEvent &event) override {
        std::cout << "[entryUpdated] " << event << std::endl;
    }

    void entryExpired(const hazelcast::client::EntryEvent &event) override {
        std::cout << "[entryExpired] " << event << std::endl;
    }

    void entryMerged(const hazelcast::client::EntryEvent &event) override {
        std::cout << "[entryMerged] " << event << std::endl;
    }

    void mapEvicted(const hazelcast::client::MapEvent &event) override {
        std::cout << "[mapEvicted] " << event << std::endl;
    }

    void mapCleared(const hazelcast::client::MapEvent &event) override {
        std::cout << "[mapCleared] " << event << std::endl;
    }
};

int main() {
    hazelcast::client::HazelcastClient hz;

    auto map = hz.getMap("somemap");

    MyEntryListener listener;

    std::string listenerId = map->addEntryListener(listener, true).get();

    std::cout << "EntryListener registered" << std::endl;

    // wait for modifymap executable to run
    std::this_thread::sleep_for(std::chrono::seconds(10));

    map->removeEntryListener(listenerId).get();

    // Continuous Query example
    // Register listener with predicate
    // Only listen events for entries with key >= 7
    listenerId = map->addEntryListener(listener, hazelcast::client::query::GreaterLessPredicate(hz,
            hazelcast::client::query::QueryConstants::KEY_ATTRIBUTE_NAME, 7, true, false), true).get();

    // wait for modifymap executable to run
    std::this_thread::sleep_for(std::chrono::seconds(10));

    map->removeEntryListener(listenerId).get();

    std::cout << "Finished" << std::endl;

    return 0;
}
