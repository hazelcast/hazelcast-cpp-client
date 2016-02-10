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
// Created by sancar koyunlu on 5/6/13.

//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef __RawDataPortable_H_
#define __RawDataPortable_H_

#include "TestNamedPortable.h"
#include "TestDataSerializable.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace test {

            class TestRawDataPortable : public serialization::Portable {
            public:
                TestRawDataPortable();

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

                void readPortable(serialization::PortableReader &reader);

                TestRawDataPortable(long l, std::vector<char> c, TestNamedPortable p, int k, std::string s, TestDataSerializable ds);

                bool operator ==(const TestRawDataPortable &m) const;

                bool operator !=(const TestRawDataPortable &m) const;

                long l;
                std::vector<char> c;
                TestNamedPortable p;
                int k;
                std::string s;
                TestDataSerializable ds;
            };
        }
    }
}


#endif //__RawDataPortable_H_


