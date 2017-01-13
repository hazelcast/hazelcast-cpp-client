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
// Created by sancar koyunlu on 6/28/13.




#ifndef HAZELCAST_Portable
#define HAZELCAST_Portable

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class PortableWriter;

            class PortableReader;

            /**
            * Classes that will be used with hazelcast data structures like IMap, IQueue etc should either inherit from
            * one of the following classes : Portable , IdentifiedDataSerializable or it should be custom serializable.
            * For custom serializable see Serializer
            *
            * Portable serialization that have the following advantages:
            * <ul>
            *     <li>Support multiversion of the same object type.
            *     See SerializationConfig#setPortableVersion(int)}</li>
            *     <li>Fetching individual fields without having to rely on reflection.</li>
            *     <li>Querying and indexing support without de-serialization and/or reflection.</li>
            * </ul>
            *
            * Note that: factoryId and classId is to the same class in find server side, hence they should be consistent
            * with each other.
            *
            * @see IdentifiedDataSerializable
            */
            class HAZELCAST_API Portable {
            public:
                /**
                * Destructor
                */
                virtual ~Portable(){

                }

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
                * @param writer PortableWriter
                */
                virtual void writePortable(PortableWriter& writer) const = 0;

                /**
                *Defines how this class will be read.
                * @param reader PortableReader
                */
                virtual void readPortable(PortableReader& reader) = 0;

            };
        }
    }


}


#endif //HAZELCAST_Portable

