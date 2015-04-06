//
// Created by sancar koyunlu on 7/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_TestSimpleXML
#define HAZELCAST_TestSimpleXML

#include "hazelcast/util/HazelcastDll.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            class TestCustomXSerializable {
            public:
                TestCustomXSerializable();

                TestCustomXSerializable(int id);

                int getTypeId() const;

                bool operator ==(const TestCustomXSerializable & rhs) const;

                bool operator !=(const TestCustomXSerializable& m) const;

                int id;
            };

            class TestCustomPerson {
            public:
                TestCustomPerson();

                TestCustomPerson(const std::string & param);

                std::string getName() const;

                void setName(const std::string & name);

                int getTypeId() const;

                bool operator ==(const TestCustomPerson & rhs) const;

                bool operator !=(const TestCustomPerson& m) const;


            private:
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_TestSimpleXML


