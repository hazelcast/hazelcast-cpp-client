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
//  TestInnerPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Hazelcast_TestInnerPortable_h
#define Hazelcast_TestInnerPortable_h

#include "TestNamedPortable.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace test {
            class TestInnerPortable : public serialization::Portable {
            public:
                TestInnerPortable();

                TestInnerPortable(const TestInnerPortable& rhs);

                TestInnerPortable(std::vector<byte> b, std::vector<char> c, std::vector<short> s, std::vector<int> i, std::vector<long> l, std::vector<float> f, std::vector<double> d, std::vector<TestNamedPortable> n);

                TestInnerPortable& operator = (const TestInnerPortable& rhs);

                int getClassId() const;

                int getFactoryId() const;

                ~TestInnerPortable();

                bool operator ==(const TestInnerPortable& m) const;

                bool operator !=(const TestInnerPortable& m) const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::vector<int> ii;
                std::vector<byte> bb;
                std::vector<char> cc;
                std::vector<short> ss;
                std::vector<long> ll;
                std::vector<float> ff;
                std::vector<double> dd;
                std::vector< TestNamedPortable > nn;

            };
        }
    }
}


#endif

