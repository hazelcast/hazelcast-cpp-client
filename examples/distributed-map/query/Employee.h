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

                int32_t getAge() const;

                const std::string &getName() const;

                bool operator<(const Employee &rhs) const;

            private:
                int32_t age;
                std::string name;

                // add all possible types
                byte by;
                bool boolean;
                char c;
                int16_t s;
                int32_t i;
                int64_t  l;
                float f;
                double d;
                std::string str;
                std::string utfStr;

                std::vector<byte> byteVec;
                std::vector<char> cc;
                std::vector<bool> ba;
                std::vector<int16_t> ss;
                std::vector<int32_t> ii;
                std::vector<int64_t > ll;
                std::vector<float> ff;
                std::vector<double> dd;
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
                            const std::pair<const int32_t *, const Employee *> *rhs) const;
            };

            std::ostream &operator<<(std::ostream &out, const Employee &employee);
            
        }
        namespace serialization {
            template<>
            struct hz_serializer<examples::Employee> : public portable_serializer {
                static int32_t getFactoryId();

                static int32_t getClassId();

                static void writePortable(const examples::Employee &object, PortableWriter &writer);

                static examples::Employee readPortable(PortableReader &reader);
            };

            template<>
            struct hz_serializer<examples::EmployeeEntryComparator> : public identified_data_serializer {
                static int32_t getFactoryId();

                static int32_t getClassId();

                static void writeData(const examples::EmployeeEntryComparator &object, ObjectDataOutput &writer);

                static examples::EmployeeEntryComparator readData(ObjectDataInput &reader);
            };

            template<>
            struct hz_serializer<examples::EmployeeEntryKeyComparator> : public identified_data_serializer {
                static int32_t getFactoryId();

                static int32_t getClassId();

                static void writeData(const examples::EmployeeEntryKeyComparator &object, ObjectDataOutput &writer);

                static examples::EmployeeEntryKeyComparator readData(ObjectDataInput &reader);
            };
        }
    }
}


