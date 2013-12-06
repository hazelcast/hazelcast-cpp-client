//
// Created by sancar koyunlu on 11/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_Employee
#define HAZELCAST_Employee

#include "hazelcast/client/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            class HAZELCAST_API Employee : public Portable {
            public:
                Employee();

                Employee(std::string name, int age);

                int getFactoryId() const;

                int getClassId() const;

                bool operator ==(const Employee &employee) const;

                bool operator !=(const Employee &employee) const;

                void writePortable(serialization::PortableWriter &writer) const;

                void readPortable(serialization::PortableReader &reader);

            private:
                int age;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_Employee
