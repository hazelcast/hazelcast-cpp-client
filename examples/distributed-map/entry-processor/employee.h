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
#pragma once

#include <hazelcast/client/serialization/serialization.h>

struct employee {
    int32_t salary;
};

struct employee_raise_entry_processor {};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<employee> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 1;
                }

                static int32_t get_class_id() noexcept {
                    return 5;
                }

                static void write_data(const employee &object, hazelcast::client::serialization::object_data_output &out) {
                    out.write(object.salary);
                }

                static employee read_data(hazelcast::client::serialization::object_data_input &in) {
                    return employee{in.read<int32_t>()};
                }
            };

            template<>
            struct hz_serializer<employee_raise_entry_processor> : identified_data_serializer {
                static int32_t get_factory_id() noexcept {
                    return 1;
                }

                static int32_t get_class_id() noexcept {
                    return 6;
                }

                static void write_data(const employee_raise_entry_processor &object, hazelcast::client::serialization::object_data_output &out) {
                }

                employee_raise_entry_processor read_data(hazelcast::client::serialization::object_data_input &in) {
                    return employee_raise_entry_processor{};
                }
            };
        }
    }
}

