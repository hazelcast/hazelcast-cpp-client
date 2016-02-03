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
// Created by sancar koyunlu on 6/28/13.


#ifndef HAZELCAST_IdentifiedDataSerializable
#define HAZELCAST_IdentifiedDataSerializable

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;

            /**
             * Classes that will be used with hazelcast data structures like IMap, IQueue etc should either inherit from
             * one of the following classes : Portable , IdentifiedDataSerializable or it should be custom serializable.
             * For custom serializable see Serializer
             *
             * Note that: factoryId and classId of the same class in server side should be consistent
             * with each other.
             *
             * @see Portable
             */
            class HAZELCAST_API IdentifiedDataSerializable {
            public:
                /**
                 * Destructor
                 */
                virtual ~IdentifiedDataSerializable();

                /**
                 * @return factory id
                 */
                virtual int getFactoryId() const = 0;

                /**
                 * @return class id
                 */
                virtual int getClassId() const = 0;

                /**
                 * Defines how this class will be written.
                 * @param writer ObjectDataOutput
                 */
                virtual void writeData(ObjectDataOutput &writer) const = 0;

                /**
                 *Defines how this class will be read.
                 * @param reader ObjectDataInput
                 */
                virtual void readData(ObjectDataInput &reader) = 0;

                /**
                 * Not public api. Do not override this method.
                 * @return type id
                 */
                virtual int getTypeId() const;

            };
        }

    }
}

#endif //HAZELCAST_DataSerializable

