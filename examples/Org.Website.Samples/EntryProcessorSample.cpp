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
#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

class IncEntryProcessor {
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<IncEntryProcessor> : identified_data_serializer {
                static int32_t getFactoryId() noexcept {
                    return 66;
                }

                static int32_t getClassId() noexcept {
                    return 1;
                }

                static void
                writeData(const IncEntryProcessor &object, hazelcast::client::serialization::ObjectDataOutput &out) {
                }

                static IncEntryProcessor readData(hazelcast::client::serialization::ObjectDataInput &in) {
                    return IncEntryProcessor{};
                }
            };
        }
    }
}

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    HazelcastClient hz;
    // Get the Distributed Map from Cluster.
    auto map = hz.getMap("my-distributed-map");
    // Put the integer value of 0 into the Distributed Map
    map->put("key", 0).get();
    // Run the IncEntryProcessor class on the Hazelcast Cluster Member holding the key called "key",
    // The entry processor returns in32_t.
    auto returnValueFromIncEntryProcessor = map->executeOnKey<std::string, int32_t, IncEntryProcessor>(
            "key", IncEntryProcessor());
    // Show that the IncEntryProcessor updated the value.
    std::cout << "new value:" << map->get<std::string, int32_t>("key").get();
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}
