/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
    try {
        hazelcast::client::ClientConfig config;
        hazelcast::client::HazelcastClient hz(config);

        hazelcast::client::IMap<std::string, std::string> map = hz.getMap<std::string, std::string>("somemap");

        MyEntryListener listener;

        map.addEntryListener(listener, true);

        std::cout << "EntryListener registered" << std::endl;

        // wait for modify map executable to run
        hazelcast::util::sleep(10);
    } catch (hazelcast::client::exception::IException &e) {
        std::cerr << "Test failed !!! " << e.what() << std::endl;
        exit(-1);
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
