/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#include "hazelcast/util/export.h"

namespace hazelcast {
    namespace util {
        struct HAZELCAST_API Bits {
            /**
            * Char size in bytes
            */
            static constexpr int CHAR_SIZE_IN_BYTES = 2;
            /**
            * Short size in bytes
            */
            static constexpr int SHORT_SIZE_IN_BYTES = 2;
            /**
            * Integer size in bytes
            */
            static constexpr int INT_SIZE_IN_BYTES = 4;

            /**
            * Float size in bytes
            */
            static constexpr int FLOAT_SIZE_IN_BYTES = 4;

            /**
            * Long size in bytes
            */
            static constexpr int LONG_SIZE_IN_BYTES = 8;

            /**
            * uuid size in bytes
            */
            static constexpr int UUID_SIZE_IN_BYTES = 16;

            /**
             * for null arrays, this value writen to stream to represent null array size.
             */
            static constexpr int NULL_ARRAY = -1;

            /**
             * Length of the data blocks used by the CPU cache sub-system in bytes.
             */
            static constexpr int CACHE_LINE_LENGTH = 64;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


