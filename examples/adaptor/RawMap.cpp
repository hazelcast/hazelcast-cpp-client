/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/adaptor/RawPointerMap.h>
#include <hazelcast/client/query/GreaterLessPredicate.h>
#include <hazelcast/client/query/QueryConstants.h>

class MyEntryListener : public hazelcast::client::EntryListener<std::string, std::string> {

public:
    void entryAdded(const hazelcast::client::EntryEvent<std::string, std::string> &event) {
        std::cout << "[entryAdded] " << event << std::endl;
    }

    void entryRemoved(const hazelcast::client::EntryEvent<std::string, std::string> &event) {
        std::cout << "[entryRemoved] " << event << std::endl;
    }

    void entryUpdated(const hazelcast::client::EntryEvent<std::string, std::string> &event) {
        std::cout << "[entryAdded] " << event << std::endl;
    }

    void entryEvicted(const hazelcast::client::EntryEvent<std::string, std::string> &event) {
        std::cout << "[entryUpdated] " << event << std::endl;
    }

    void entryExpired(const hazelcast::client::EntryEvent<std::string, std::string> &event) {
        std::cout << "[entryExpired] " << event << std::endl;
    }

    void entryMerged(const hazelcast::client::EntryEvent<std::string, std::string> &event) {
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
    hazelcast::client::HazelcastClient hz;

    hazelcast::client::IMap<std::string, std::string> m = hz.getMap<std::string, std::string>("map");
    hazelcast::client::adaptor::RawPointerMap<std::string, std::string> map(m);
    map.put("1", "Tokyo");
    map.put("2", "Paris");
    map.put("3", "New York");
    std::cout << "Finished loading map" << std::endl;

    std::auto_ptr<hazelcast::client::DataArray<std::string> > vals = map.values();
    std::auto_ptr<hazelcast::client::EntryArray<std::string, std::string> > entries = map.entrySet();

    std::cout << "There are " << vals->size() << " values in the map" << std::endl;
    std::cout << "There are " << entries->size() << " entries in the map" << std::endl;

    for (size_t i = 0; i < entries->size(); ++i) {
        const std::string * key = entries->getKey(i);
        if ((std::string *) NULL == key) {
            std::cout << "The key at index " << i << " is NULL" << std::endl;
        } else {
            std::auto_ptr<std::string> val = entries->releaseValue(i);
            std::cout << "(Key, Value) for index " << i << " is: (" << *key << ", " <<
                (val.get() == NULL ? "NULL" : *val) << ")" << std::endl;
        }
    }

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
