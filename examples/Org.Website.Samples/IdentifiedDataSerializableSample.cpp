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

struct Employee {
    friend std::ostream &operator<<(std::ostream &os, const Employee &person);

    int32_t id;
    std::string name;
};

std::ostream &operator<<(std::ostream &os, const Employee &person) {
    os << "id: " << person.id << " name: " << person.name;
    return os;
}

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<Employee> : identified_data_serializer {
                static int32_t getFactoryId() noexcept {
                    return 100;
                }

                static int32_t getClassId() noexcept {
                    return 1000;
                }

                static void writeData(const Employee &object, hazelcast::client::serialization::ObjectDataOutput &out) {
                    out.write(object.id);
                    out.write(object.name);
                }

                static Employee readData(hazelcast::client::serialization::ObjectDataInput &in) {
                    return Employee{in.read<int32_t>(), in.read<std::string>()};
                }
            };
        }
    }
}

int main() {
    HazelcastClient hz;
    //Employee can be used here
    hz.shutdown();

    return 0;
}
