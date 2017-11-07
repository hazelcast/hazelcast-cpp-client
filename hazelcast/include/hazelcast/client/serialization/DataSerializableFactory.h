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

#ifndef HAZELCAST_CLIENT_SERIALIZATION_DATASERIALIZABLEFACTORY_H_
#define HAZELCAST_CLIENT_SERIALIZATION_DATASERIALIZABLEFACTORY_H_

#include <stdint.h>
#include <memory>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class IdentifiedDataSerializable;

            /**
             * DataSerializableFactory is used to create IdentifiedDataSerializable instances during de-serialization.
             *
             * You should implement this interface as copyable since it will be copied into the SerializationConfig.
             *
             * @see IdentifiedDataSerializable
             */
            class HAZELCAST_API DataSerializableFactory {
            public:
                /**
                 * Creates an IdentifiedDataSerializable instance using given class ID
                 *
                 * @param classId IdentifiedDataSerializable type ID
                 * @return IdentifiedDataSerializable instance or {@code null} if type ID is not known by this factory
                 */
                virtual std::auto_ptr<IdentifiedDataSerializable> create(int32_t classId) = 0;
            };
        }
    }
}


#endif //HAZELCAST_CLIENT_SERIALIZATION_DATASERIALIZABLEFACTORY_H_
