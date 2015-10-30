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
// Created by İhsan Demir on 27/10/15.
//

#ifndef HAZELCASTCLIENT_MULTIPLIERDATASERIALIZABLEENTRYPROCESSOR_H
#define HAZELCASTCLIENT_MULTIPLIERDATASERIALIZABLEENTRYPROCESSOR_H

#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;
        }

        namespace test {
            class MultiplierDataSerializableEntryProcessor : public serialization::IdentifiedDataSerializable {
            public:
                MultiplierDataSerializableEntryProcessor(int multiplier);

                /**
                 * @return factory id
                 */
                int getFactoryId() const;

                /**
                 * @return class id
                 */
                int getClassId() const;

                /**
                 * Defines how this class will be written.
                 * @param writer ObjectDataOutput
                 */
                void writeData(serialization::ObjectDataOutput &writer) const;

                /**
                 *Defines how this class will be read.
                 * @param reader ObjectDataInput
                 */
                void readData(serialization::ObjectDataInput &reader);

            private:
                int multiplier;
            };
        }
    }
}


#endif //HAZELCASTCLIENT_MULTIPLIERDATASERIALIZABLEENTRYPROCESSOR_H
