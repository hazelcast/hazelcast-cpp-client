/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#pragma once
#include <stdint.h>
#include <memory>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/serialization.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            /**
             * PortableFactory is used to create Portable instances during de-serialization.
             *
             * You should implement this interface as copyable since it will be copied into the SerializationConfig.
             *
             * @see Portable
             * @see VersionedPortable
             */
            class HAZELCAST_API PortableFactory {
            public:
                virtual ~PortableFactory() {
                }

                /**
                 * Creates a Portable instance using given class ID
                 * @param classId portable class ID
                 * @return portable instance or null if class ID is not known by this factory
                 */
                virtual std::unique_ptr<Portable> create(int32_t classId) const = 0;
            };
        }
    }
}


