/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by İhsan Demir on 21/12/15.
//
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/query/GreaterLessPredicate.h>
#include <hazelcast/client/query/QueryConstants.h>

class MyEntryListener : public hazelcast::client::EntryListener<int, int> {

public:
    void entryAdded(const hazelcast::client::EntryEvent<int, int> &event) {
        std::cout << "[entryAdded] " << event << std::endl;
    }

    void entryRemoved(const hazelcast::client::EntryEvent<int, int> &event) {
        std::cout << "[entryRemoved] " << event << std::endl;
    }

    void entryUpdated(const hazelcast::client::EntryEvent<int, int> &event) {
        std::cout << "[entryAdded] " << event << std::endl;
    }

    void entryEvicted(const hazelcast::client::EntryEvent<int, int> &event) {
        std::cout << "[entryUpdated] " << event << std::endl;
    }

    void entryExpired(const hazelcast::client::EntryEvent<int, int> &event) {
        std::cout << "[entryExpired] " << event << std::endl;
    }

    void entryMerged(const hazelcast::client::EntryEvent<int, int> &event) {
        std::cout << "[entryMerged] " << event << std::endl;
    }

    void mapEvicted(const hazelcast::client::MapEvent &event) {
        std::cout << "[mapEvicted] " << event << std::endl;
    }

    void mapCleared(const hazelcast::client::MapEvent &event) {
        std::cout << "[mapCleared] " << event << std::endl;
    }
};

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<int, int> map = hz.getMap<int, int>("somemap");

    MyEntryListener listener;

    std::string listenerId = map.addEntryListener(listener, true);

    std::cout << "EntryListener registered" << std::endl;

    // wait for modifymap executable to run
    hazelcast::util::sleep(10);

    map.removeEntryListener(listenerId);

    // Continuous Query example
    // Register listener with predicate
    // Only listen events for entries with key >= 7
    listenerId = map.addEntryListener(listener, hazelcast::client::query::GreaterLessPredicate<int>(
            hazelcast::client::query::QueryConstants::getKeyAttributeName(), 7, true, false), true);

    // wait for modifymap executable to run
    hazelcast::util::sleep(10);

    map.removeEntryListener(listenerId);

    std::cout << "Finished" << std::endl;

    return 0;
}
