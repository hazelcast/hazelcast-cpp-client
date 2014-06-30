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
            class HAZELCAST_API TestCustomXSerializable {
            public:
                int id;

                int getTypeId() const;
            };

            class HAZELCAST_API TestCustomPerson {
            public:

                std::string getName() const;

                void setName(const std::string & param);

                int getTypeId() const;

            private:
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_TestSimpleXML


