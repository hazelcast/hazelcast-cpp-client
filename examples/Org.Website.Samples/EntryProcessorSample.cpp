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
#include <hazelcast/client/hazelcast.h>

using namespace hazelcast::client;

struct IdentifiedEntryProcessor {
    std::string name;
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<IdentifiedEntryProcessor> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 66;
                }

                static int32_t get_class_id() noexcept {
                    return 1;
                }

                static void
                write_data(const IdentifiedEntryProcessor &object, hazelcast::client::serialization::object_data_output &out) {
                    out.write(object.name);
                }

                static IdentifiedEntryProcessor read_data(hazelcast::client::serialization::object_data_input &in) {
                    // no-need to implement here, since we do not expect to receive this object but we only send it to server
                    assert(0);
                    return IdentifiedEntryProcessor{};
                }
            };
        }
    }
}

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    hazelcast::client::hazelcast_client hz;
    // Get the Distributed Map from Cluster.
    auto map = hz.get_map("my-distributed-map").get();
    // Put the integer value of 0 into the Distributed Map
    map->put("key", 0).get();
    // Run the IncEntryProcessor class on the Hazelcast Cluster Member holding the key called "key",
    // The entry processor returns in32_t.
    auto returnValueFromIncEntryProcessor = map->execute_on_key<std::string, int32_t, IdentifiedEntryProcessor>(
            "key", IdentifiedEntryProcessor{"my_name"});
    // Show that the IncEntryProcessor updated the value.
    std::cout << "new value:" << *map->get<std::string, std::string>("key").get();
    // Shutdown this Hazelcast Client
    hz.shutdown();

    return 0;
}
