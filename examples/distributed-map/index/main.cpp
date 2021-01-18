/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

struct Person {
    std::string name;
    bool male;
    int32_t age;
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<Person> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 1;
                }

                static int32_t get_class_id() noexcept {
                    return 3;
                }

                static void write_data(const Person &object, hazelcast::client::serialization::object_data_output &out) {
                    out.write(object.name);
                    out.write(object.male);
                    out.write(object.age);
                }

                static Person read_data(hazelcast::client::serialization::object_data_input &in) {
                    return Person{in.read<std::string>(), in.read<bool>(), in.read<int32_t>()};
                }
            };
        }
    }
}

int main() {
    auto hz = hazelcast::new_client().get();

    auto map = hz.get_map("personsWithIndex").get();

    map->add_index(hazelcast::client::config::index_config::index_type::SORTED, "name").get();

    const int mapSize = 200000;

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < mapSize; ++i) {
        map->put<std::string, Person>(std::string("person-") + std::to_string(i),
                                      {std::string("myname-") + std::to_string(i), (i % 2 == 0), (i % 100)}).get();
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "Put " << mapSize << " entries into the map in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds" << '\n';

    start = std::chrono::steady_clock::now();
    auto entries = map->entry_set<std::string, Person>(
            hazelcast::client::query::sql_predicate(hz, "name == 'myname-30'")).get();
    end = std::chrono::steady_clock::now();
    std::cout << "The query resulted in " << entries.size() << " entries in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds" << '\n';

    std::cout << "Finished" << std::endl;

    return 0;
}
