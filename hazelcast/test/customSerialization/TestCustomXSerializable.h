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
// Created by sancar koyunlu on 7/1/13.


#ifndef HAZELCAST_TestSimpleXML
#define HAZELCAST_TestSimpleXML

#include "hazelcast/util/HazelcastDll.h"
#include <string>
#include "hazelcast/client/serialization/Serializer.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class TestCustomXSerializable : public serialization::SerializerBase {
            public:
                TestCustomXSerializable();

                TestCustomXSerializable(int id);

                int getTypeId() const;

                bool operator ==(const TestCustomXSerializable & rhs) const;

                bool operator !=(const TestCustomXSerializable& m) const;

                int id;
            };

            class TestCustomPerson : public serialization::SerializerBase {
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


