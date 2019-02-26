/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#ifndef __Server__TestNamedPortableV2__
#define __Server__TestNamedPortableV2__


#include "TestNamedPortable.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class TestNamedPortableV2 : public serialization::Portable {
            public:
                TestNamedPortableV2();

                TestNamedPortableV2(std::string name, int v);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

                std::string name;
                int k;
                int v;

            };

        }
    }
}


#endif

