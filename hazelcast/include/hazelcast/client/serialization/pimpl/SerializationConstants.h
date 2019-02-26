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
//
//  SerializationConstants.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_CONSTANTS
#define HAZELCAST_SERIALIZATION_CONSTANTS

#include "hazelcast/util/HazelcastDll.h"
#include <string>
#include <vector>
#include <stdint.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class HAZELCAST_API SerializationConstants {
                public:
                    enum Types {
                        CONSTANT_TYPE_NULL = 0,
                        CONSTANT_TYPE_PORTABLE = -1,
                        CONSTANT_TYPE_DATA = -2,
                        CONSTANT_TYPE_BYTE = -3,
                        CONSTANT_TYPE_BOOLEAN = -4,
                        CONSTANT_TYPE_CHAR = -5,
                        CONSTANT_TYPE_SHORT = -6,
                        CONSTANT_TYPE_INTEGER = -7,
                        CONSTANT_TYPE_LONG = -8,
                        CONSTANT_TYPE_FLOAT = -9,
                        CONSTANT_TYPE_DOUBLE = -10,
                        CONSTANT_TYPE_STRING = -11,
                        CONSTANT_TYPE_BYTE_ARRAY = -12,
                        CONSTANT_TYPE_BOOLEAN_ARRAY = -13,
                        CONSTANT_TYPE_CHAR_ARRAY = -14,
                        CONSTANT_TYPE_SHORT_ARRAY = -15,
                        CONSTANT_TYPE_INTEGER_ARRAY = -16,
                        CONSTANT_TYPE_LONG_ARRAY = -17,
                        CONSTANT_TYPE_FLOAT_ARRAY = -18,
                        CONSTANT_TYPE_DOUBLE_ARRAY = -19,
                        CONSTANT_TYPE_STRING_ARRAY = -20,
                        CONSTANT_TYPE_VOID_POINTER = INT32_MIN
                    };
                    // ------------------------------------------------------------
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_SERIALIZATION_CONSTANTS */

