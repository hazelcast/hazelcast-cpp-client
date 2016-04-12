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

            bool Employee::operator ==(const Employee &employee) const {
                if (age != employee.age)
                    return false;
                else if (name.compare(employee.name))
                    return false;
                else
                    return true;
            }

            bool Employee::operator !=(const Employee &employee) const {
                return !(*this == employee);
            }

            int Employee::getFactoryId() const {
                return TestSerializationConstants::EMPLOYEE_FACTORY;
            }

            int Employee::getClassId() const {
                return TestSerializationConstants::EMPLOYEE;
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

        }
    }
}

