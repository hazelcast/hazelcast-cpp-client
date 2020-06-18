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
#pragma once

#include <hazelcast/client/serialization/serialization.h>

struct Employee {
    int32_t salary;
};

struct EmployeeRaiseEntryProcessor {};

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<Employee> : identified_data_serializer {
                static int32_t getFactoryId() noexcept {
                    return 1;
                }

                static int32_t getClassId() noexcept {
                    return 5;
                }

                static void writeData(const Employee &object, hazelcast::client::serialization::ObjectDataOutput &out) {
                    out.write(object.salary);
                }

                static Employee readData(hazelcast::client::serialization::ObjectDataInput &in) {
                    return Employee{in.read<int32_t>()};
                }
            };

            template<>
            struct hz_serializer<EmployeeRaiseEntryProcessor> : identified_data_serializer {
                static int32_t getFactoryId() noexcept {
                    return 1;
                }

                static int32_t getClassId() noexcept {
                    return 6;
                }

                static void writeData(const EmployeeRaiseEntryProcessor &object, hazelcast::client::serialization::ObjectDataOutput &out) {
                }

                EmployeeRaiseEntryProcessor readData(hazelcast::client::serialization::ObjectDataInput &in) {
                    return EmployeeRaiseEntryProcessor{};
                }
            };
        }
    }
}

