/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//  TestMainPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestMainPortable_h
#define Server_TestMainPortable_h

#include "TestInnerPortable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            class TestMainPortable : public serialization::Portable {
            public:

                TestMainPortable();

                TestMainPortable(byte b, bool boolean, char c, short s, int i, long l, float f, double d, std::string str, TestInnerPortable p);

                bool operator ==(const TestMainPortable &m) const;

                bool operator !=(const TestMainPortable &m) const;

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

                void readPortable(serialization::PortableReader &reader);

                TestInnerPortable p;
                int i;
            private:
                bool null;
                byte b;
                bool boolean;
                char c;
                short s;
                long l;
                float f;
                double d;
                std::string str;
            };

        }
    }
}
#endif

