/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 5/12/13.




#ifndef __TestMobile_H_
#define __TestMobile_H_

#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class TestDataSerializable : public serialization::IdentifiedDataSerializable {
            public:
                TestDataSerializable();

                TestDataSerializable(int i, char c);

                bool operator ==(const TestDataSerializable & rhs) const;

                bool operator !=(const TestDataSerializable& m) const;

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput& writer) const;

                void readData(serialization::ObjectDataInput& reader);

                int i;
                char c;
            };

        }
    }
}
#endif //__TestMobile_H_

