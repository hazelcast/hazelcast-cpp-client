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

class MapInterceptor {};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<MapInterceptor> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 1;
                }

                static int32_t get_class_id() noexcept {
                    return 7;
                }

                static void write_data(const MapInterceptor &object, hazelcast::client::serialization::object_data_output &out) {}

                static MapInterceptor read_data(hazelcast::client::serialization::object_data_input &in) {
                    return MapInterceptor{};
                }
            };
        }
    }
}

int main() {
    hazelcast::client::hazelcast_client hz;

    auto map = hz.get_map("themap").get();

    map->add_interceptor(MapInterceptor{}).get();

    map->put<std::string, std::string>("1", "1").get();

    std::cout << "The modified value (modified by the interceptor) at the server:" << *map->get<std::string, std::string>("1").get() << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
