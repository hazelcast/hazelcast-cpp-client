//
// Created by sancar koyunlu on 11/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "Employee.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace test {
            Employee::Employee():age(-1), name("") {

            }

            Employee::Employee(std::string name, int age)
            :age(age)
            , name(name) {

            };

            int Employee::getFactoryId() const {
                return 666;
            };

            int Employee::getClassId() const {
                return 2;
            };

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

            void Employee::writePortable(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeInt("a", age);
            };

            void Employee::readPortable(serialization::PortableReader &reader) {
                name = reader.readUTF("n");
                age = reader.readInt("a");
            };

        }
    }
}
