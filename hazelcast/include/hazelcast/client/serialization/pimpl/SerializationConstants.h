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
                    int const CONSTANT_TYPE_NULL;
                    int const CONSTANT_TYPE_PORTABLE;
                    int const CONSTANT_TYPE_DATA;
                    int const CONSTANT_TYPE_BYTE;
                    int const CONSTANT_TYPE_BOOLEAN;
                    int const CONSTANT_TYPE_CHAR;
                    int const CONSTANT_TYPE_SHORT;
                    int const CONSTANT_TYPE_INTEGER;
                    int const CONSTANT_TYPE_LONG;
                    int const CONSTANT_TYPE_FLOAT;
                    int const CONSTANT_TYPE_DOUBLE;
                    int const CONSTANT_TYPE_STRING;
                    int const CONSTANT_TYPE_BYTE_ARRAY;
                    int const CONSTANT_TYPE_BOOLEAN_ARRAY;
                    int const CONSTANT_TYPE_CHAR_ARRAY;
                    int const CONSTANT_TYPE_SHORT_ARRAY;
                    int const CONSTANT_TYPE_INTEGER_ARRAY;
                    int const CONSTANT_TYPE_LONG_ARRAY;
                    int const CONSTANT_TYPE_FLOAT_ARRAY;
                    int const CONSTANT_TYPE_DOUBLE_ARRAY;
                    int const CONSTANT_TYPE_STRING_ARRAY;
                    // ------------------------------------------------------------

                    std::string typeIdToName(int typeId);

                    static SerializationConstants *getInstance();
                private:
                    const int size;
                    std::vector<std::string> typeIdNameVector;

                    static SerializationConstants *instance;

                    SerializationConstants();

                    int idToIndex(int id);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_SERIALIZATION_CONSTANTS */

