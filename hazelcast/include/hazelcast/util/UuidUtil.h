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
#include <string>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/UUID.h"

namespace hazelcast {
    namespace util {
        class HAZELCAST_API UuidUtil {
        public:
            /**
             * Static factory to retrieve a type 4 (pseudo randomly generated) UUID based string.
             *
             * The {@code UUID} string is generated using a cryptographically weak pseudo random number generator.
             *
             * @return A randomly generated {@code UUID} base string
             */
            static std::string newUnsecureUuidString();

            /**
             * Static factory to retrieve a type 4 (pseudo randomly generated) UUID.
             *
             * The {@code UUID} is generated using a cryptographically weak pseudo random number generator.
             *
             * @return A randomly generated {@code UUID}
             */
            static UUID newUnsecureUUID();
        };
    }
}


