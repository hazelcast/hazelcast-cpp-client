/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan demir on 18 Apr 2016.

#ifndef HAZELCAST_Employee
#define HAZELCAST_Employee

#include "hazelcast/client/serialization/Portable.h"
#include <string>
#include <hazelcast/util/Comparator.h>
#include <hazelcast/client/serialization/IdentifiedDataSerializable.h>

namespace hazelcast {
    namespace client {
        namespace examples {
            namespace criteriaapi {
                class Employee : public serialization::Portable {
                public:
                    Employee();

                    Employee(std::string name, int age);

                    bool operator==(const Employee &employee) const;

                    bool operator!=(const Employee &employee) const;

                    int getFactoryId() const;

                    int getClassId() const;

                    void writePortable(serialization::PortableWriter &writer) const;

                    void readPortable(serialization::PortableReader &reader);

                    int getAge() const;

                    const std::string &getName() const;

                    bool operator<(const Employee &rhs) const;
                private:
                    int age;
                    std::string name;
                };

                // Compares based on the employee age
                class EmployeeEntryComparator
                        : public util::Comparator<std::pair<const int *, const Employee *> >,
                          public serialization::IdentifiedDataSerializable {

                public:
                    int getFactoryId() const;

                    int getClassId() const;

                    void writeData(serialization::ObjectDataOutput &writer) const;

                    void readData(serialization::ObjectDataInput &reader);

                    int compare(const std::pair<const int *, const Employee *> &lhs,
                                const std::pair<const int *, const Employee *> &rhs) const;
                };
            }
        }
    }
}

#endif //HAZELCAST_Employee

