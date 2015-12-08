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




#ifndef __TestInvalidReadPortable_H_
#define __TestInvalidReadPortable_H_

#include "hazelcast/client/serialization/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            class TestInvalidReadPortable : public serialization::Portable {
            public:

                TestInvalidReadPortable();

                TestInvalidReadPortable(long l, int i, std::string s);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);
            private:
                long l;
                int i;
                std::string s;
            };
        }
    }
}
#endif //__TestInvalidReadPortable_H_



