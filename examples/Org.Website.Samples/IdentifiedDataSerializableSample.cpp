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

struct employee {
    friend std::ostream &operator<<(std::ostream &os, const employee &person);

    int32_t id;
    std::string name;
};

std::ostream &operator<<(std::ostream &os, const employee &person) {
    os << "id: " << person.id << " name: " << person.name;
    return os;
}

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<employee> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 100;
                }

                static int32_t get_class_id() noexcept {
                    return 1000;
                }

                static void write_data(const employee &object, hazelcast::client::serialization::object_data_output &out) {
                    out.write(object.id);
                    out.write(object.name);
                }

                static employee read_data(hazelcast::client::serialization::object_data_input &in) {
                    return employee{in.read<int32_t>(), in.read<std::string>()};
                }
            };
        }
    }
}

int main() {
    hazelcast::client::hazelcast_client hz;
    //Employee can be used here
    hz.shutdown();

    return 0;
}
