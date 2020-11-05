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

#include <string>
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/Comparator.h"
#include "hazelcast/client/query/EntryComparator.h"

namespace hazelcast {
    namespace client {
        namespace examples {
            class Employee {
                friend serialization::hz_serializer<Employee>;
            public:
                Employee();

                Employee(std::string name, int age);

                bool operator ==(const Employee &employee) const;

                bool operator !=(const Employee &employee) const;

                int32_t get_age() const;

                const std::string &get_name() const;

                bool operator<(const Employee &rhs) const;

            private:
                int32_t age_;
                std::string name_;

                // add all possible types
                byte by_;
                bool boolean_;
                char c_;
                int16_t s_;
                int32_t i_;
                int64_t  l_;
                float f_;
                double d_;
                std::string str_;
                std::string utfStr_;

                std::vector<byte> byteVec_;
                std::vector<char> cc_;
                std::vector<bool> ba_;
                std::vector<int16_t> ss_;
                std::vector<int32_t> ii_;
                std::vector<int64_t > ll_;
                std::vector<float> ff_;
                std::vector<double> dd_;
            };

            // Compares based on the employee age
            class EmployeeEntryComparator : public query::EntryComparator<int32_t, Employee> {
            public:
                int compare(const std::pair<const int32_t *, const Employee *> *lhs,
                            const std::pair<const int32_t *, const Employee *> *rhs) const override;
            };

            // Compares based on the employee age
            class EmployeeEntryKeyComparator : public EmployeeEntryComparator {
            public:
                int compare(const std::pair<const int32_t *, const Employee *> *lhs,
                            const std::pair<const int32_t *, const Employee *> *rhs) const override;
            };

            std::ostream &operator<<(std::ostream &out, const Employee &employee);
            
        }
        namespace serialization {
            template<>
            struct hz_serializer<examples::Employee> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const examples::Employee &object, PortableWriter &writer);

                static examples::Employee read_portable(PortableReader &reader);
            };

            template<>
            struct hz_serializer<examples::EmployeeEntryComparator> : public identified_data_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_data(const examples::EmployeeEntryComparator &object, ObjectDataOutput &writer);

                static examples::EmployeeEntryComparator read_data(ObjectDataInput &reader);
            };

            template<>
            struct hz_serializer<examples::EmployeeEntryKeyComparator> : public identified_data_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_data(const examples::EmployeeEntryKeyComparator &object, ObjectDataOutput &writer);

                static examples::EmployeeEntryKeyComparator read_data(ObjectDataInput &reader);
            };
        }
    }
}


