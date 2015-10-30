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
// Created by İhsan Demir on 26/10/15.
//

#ifndef HAZELCASTCLIENT_MULTIPLIERENTRYPROCESSOR_H
#define HAZELCASTCLIENT_MULTIPLIERENTRYPROCESSOR_H

#include "hazelcast/client/serialization/Portable.h"

using namespace hazelcast::client;

namespace hazelcast {
    namespace client {
        namespace serialization {
            class PortableWriter;
            class PortableReader;
        }

        namespace test {
            class MultiplierPortableEntryProcessor : public serialization::Portable {
            public:
                MultiplierPortableEntryProcessor(int multiplier);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

                void readPortable(serialization::PortableReader &reader);

            private:
                int multiplier;
            };

        }
    }
}

#endif //HAZELCASTCLIENT_MULTIPLIERENTRYPROCESSOR_H

