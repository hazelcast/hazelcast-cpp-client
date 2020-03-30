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
//
// Created by sancar koyunlu on 11/11/13.

#ifndef HAZELCAST_Employee
#define HAZELCAST_Employee

#include <string>
#include <stdint.h>
#include "hazelcast/client/query/EntryComparator.h"
#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/util/Comparator.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class Employee : public serialization::Portable {
            public:
                Employee();

                Employee(std::string name, int age);

                bool operator ==(const Employee &employee) const;

                bool operator !=(const Employee &employee) const;

                int32_t getFactoryId() const;

                int32_t getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

                void readPortable(serialization::PortableReader &reader);

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
            class EmployeeEntryComparator
                    : public query::EntryComparator<int32_t, Employee> {

            public:
                int32_t getFactoryId() const;

                virtual int32_t getClassId() const;

                void writeData(serialization::ObjectDataOutput &writer) const;

                void readData(serialization::ObjectDataInput &reader);

                virtual int compare(const std::pair<const int32_t *, const Employee *> *lhs,
                            const std::pair<const int32_t *, const Employee *> *rhs) const;
            };

            // Compares based on the employee age
            class EmployeeEntryKeyComparator
                    : public EmployeeEntryComparator {
            public:
                int compare(const std::pair<const int32_t *, const Employee *> *lhs,
                            const std::pair<const int32_t *, const Employee *> *rhs) const;

                int32_t getClassId() const;
            };

            std::ostream &operator<<(std::ostream &out, const Employee &employee);
        }
    }
}

#endif //HAZELCAST_Employee

