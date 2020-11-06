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
#include <hazelcast/client/serialization/serialization.h>
#include <memory>
#include <map>
#include <ostream>

struct Car {
    Car() = default;

    explicit Car(const char *name) {
        attributes["name"] = name;
        attributes["tripStart"] = "0";
        attributes["tripStop"] = "0";
    }

    Car(const char *name, int break_horse_power, int mileage) {
        attributes["name"] = name;
        attributes["tripStart"] = "0";
        attributes["tripStop"] = "0";
        attributes["bhp"] = std::to_string(break_horse_power);
        attributes["mileage"] = std::to_string(mileage);
    }

    friend std::ostream &operator<<(std::ostream &os, const Car &car) {
        os << "attributes: {";
        for (auto &entry : car.attributes) {
            os << "(" << entry.first << ": " << entry.second << "), ";
        }
        os << "}";
        return os;
    }

    std::map<std::string, std::string> attributes;
};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<Car> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 1;
                }

                static int32_t get_class_id() noexcept {
                    return 4;
                }

                static void write_data(const Car &object, hazelcast::client::serialization::object_data_output &out) {
                    out.write(static_cast<int32_t>(object.attributes.size()));
                    for (auto &entry : object.attributes) {
                        out.write(entry.first);
                        out.write(entry.second);
                    }
                }

                static Car read_data(hazelcast::client::serialization::object_data_input &in) {
                    Car object;
                    int32_t size = in.read<int32_t>();
                    if (size > 0) {
                        for (int32_t i = 0; i < size; ++i) {
                            object.attributes[in.read<std::string>()] = in.read<std::string>();
                        }
                    }
                    return object;
                }
            };
        }
    }
}

int main() {
    hazelcast::client::hazelcast_client hz;

    auto map = hz.get_map("cars");

    map->put(1, Car("Audi Q7", 250, 22000)).get();
    map->put(2, Car("BMW X5", 312, 34000)).get();
    map->put(3, Car("Porsche Cayenne", 408, 57000)).get();

    // we're using a custom attribute called 'attribute' which is provided by the 'CarAttributeExtractor'
    // we are also passing an argument 'mileage' to the extractor
    hazelcast::client::query::SqlPredicate criteria(hz, "attribute[mileage] < 30000");
    auto cars = map->values<Car>(criteria).get();

    for (const auto &car : cars) {
        std::cout << car << '\n';
    }

    std::cout << "Finished" << std::endl;

    return 0;
}

