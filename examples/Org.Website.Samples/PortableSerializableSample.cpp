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

struct PortableSerializableSample {
    std::string name;
    int32_t id;
    int64_t last_order;
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<PortableSerializableSample> : portable_serializer {
                static int32_t get_factory_id() noexcept {
                    return 1;
                }

                static int32_t get_class_id() noexcept {
                    return 1;
                }

                static void write_portable(const PortableSerializableSample &object,
                                          hazelcast::client::serialization::portable_writer &out) {
                    out.write("name", object.name);
                    out.write("id", object.id);
                    out.write("lastOrder", object.last_order);
                }

                static PortableSerializableSample read_portable(hazelcast::client::serialization::portable_reader &in) {
                    return PortableSerializableSample{in.read<std::string>("name"), in.read<int32_t>("id"),
                                                      in.read<int64_t>("lastOrder")};
                }
            };
        }
    }
}

int main() {
    hazelcast_client hz;
    //PortableSerializableSample can be used here
    hz.shutdown();

    return 0;
}
