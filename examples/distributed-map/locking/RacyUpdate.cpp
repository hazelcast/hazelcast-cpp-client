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

#include <hazelcast/client/hazelcast_client.h>

struct Value {
    int32_t amount;
    int32_t version;
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<Value> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 1;
                }

                static int32_t get_class_id() noexcept {
                    return 6;
                }

                static void write_data(const Value &object, hazelcast::client::serialization::object_data_output &out) {
                    out.write(object.amount);
                    out.write(object.version);
                }

                static Value read_data(hazelcast::client::serialization::object_data_input &in) {
                    return Value{in.read<int32_t>(), in.read<int32_t>()};
                }
            };
        }
    }
}

int main() {
    auto hz = hazelcast::new_client().get();

    auto map = hz.get_map("map").get();

    std::string key("1");
    Value v;
    map->put(key, v).get();
    std::cout << "Starting" << std::endl;
    for (int k = 0; k < 1000; k++) {
        if (k % 100 == 0) {
            std::cout << "At: " << k << std::endl;
        }

        auto oldValue = map->get<std::string, Value>(key).get();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        map->put(key, *oldValue).get();
    }
    std::cout << "Finished! Result = " << map->get<std::string, Value>(key).get()->amount << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
