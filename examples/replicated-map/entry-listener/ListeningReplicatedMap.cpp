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
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    boost::shared_ptr<hazelcast::client::ReplicatedMap<std::string, std::string> > map = hz.getReplicatedMap<std::string, std::string>(
            "map");

    boost::shared_ptr<hazelcast::client::EntryListener<std::string, std::string> > listener(new MyEntryListener());

    std::string listenerId = map->addEntryListener(listener);

    std::cout << "EntryListener registered with id " << listenerId << std::endl;

    return 0;
}
