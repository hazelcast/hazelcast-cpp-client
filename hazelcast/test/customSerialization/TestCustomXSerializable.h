//
// Created by sancar koyunlu on 7/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_TestSimpleXML
#define HAZELCAST_TestSimpleXML

#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            class TestCustomXSerializable {
            public:
                int id;

                int getSerializerId() const;
            };

            class TestCustomPerson {
            public:

                std::string getName() const;

                void setName(const std::string & param);

                int getSerializerId() const;

            private:
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_TestSimpleXML

