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
// Created by sancar koyunlu on 11/11/13.

#include "Employee.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {
            Employee::Employee():age(-1), name("") {

            }

            Employee::Employee(std::string name, int age)
                    :age(age)
                    , name(name) {

            }

            bool Employee::operator==(const Employee &rhs) const {
                return age == rhs.getAge() && name == rhs.getName();
            }

            bool Employee::operator !=(const Employee &employee) const {
                return !(*this == employee);
            }

            int Employee::getFactoryId() const {
                return 666;
            }

            int Employee::getClassId() const {
                return 2;
            }

            void Employee::writePortable(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", &name);
                writer.writeInt("a", age);
            }

            void Employee::readPortable(serialization::PortableReader &reader) {
                name = *reader.readUTF("n");
                age = reader.readInt("a");
            }

            int Employee::getAge() const {
                return age;
            }

            const std::string &Employee::getName() const {
                return name;
            }

            bool Employee::operator<(const Employee &rhs) const {
                return age < rhs.getAge();
            }

            int EmployeeEntryComparator::getFactoryId() const {
                return 666;
            }

            int EmployeeEntryComparator::getClassId() const {
                return 4;
            }

            void EmployeeEntryComparator::writeData(serialization::ObjectDataOutput &writer) const {
            }

            void EmployeeEntryComparator::readData(serialization::ObjectDataInput &reader) {
            }

            int EmployeeEntryComparator::compare(const std::pair<const int *, const Employee *> *lhs,
                        const std::pair<const int *, const Employee *> *rhs) const {
                const Employee *lv = lhs->second;
                const Employee *rv = rhs->second;

                if (NULL == lv && NULL == rv) {
                    // order by key
                    const int leftKey = *lhs->first;
                    const int rightKey = *rhs->first;

                    if (leftKey == rightKey) {
                        return 0;
                    }

                    if (leftKey < rightKey) {
                        return -1;
                    }

                    return 1;
                }

                if (NULL == lv) {
                    return -1;
                }

                if (NULL == rv) {
                    return 1;
                }

                int la = lv->getAge();
                int ra = rv->getAge();

                if (la == ra) {
                    return 0;
                }

                if (la < ra) {
                    return -1;
                }

                return 1;
            }


            int EmployeeEntryKeyComparator::compare(const std::pair<const int *, const Employee *> *lhs,
                                                    const std::pair<const int *, const Employee *> *rhs) const {
                const int *key1 = lhs->first;
                const int *key2 = rhs->first;

                if (NULL == key1) {
                    return -1;
                }

                if (NULL == key2) {
                    return 1;
                }

                if (*key1 == *key2) {
                    return 0;
                }

                if (*key1 < *key2) {
                    return -1;
                }

                return 1;
            }

            int EmployeeEntryKeyComparator::getClassId() const {
                return 5;
            }

            std::ostream &operator<<(std::ostream &out, const Employee &employee) {
                out << "Employee:[" << employee.getName() << ", " << employee.getAge() << "]";
                return out;
            }
        }
    }
}

