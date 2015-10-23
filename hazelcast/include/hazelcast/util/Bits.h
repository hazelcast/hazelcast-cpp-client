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
// Created by sancar koyunlu on 18/11/14.
//


#ifndef HAZELCAST_Bits
#define HAZELCAST_Bits

#include "hazelcast/util/HazelcastDll.h"
#include <vector>
#include <memory>

namespace hazelcast {
    namespace util {
        HAZELCAST_API class Bits {
        public:
            /**
            * Byte size in bytes
            */
            static const unsigned  int BYTE_SIZE_IN_BYTES = 1;
            /**
            * Boolean size in bytes
            */
            static const unsigned  int BOOLEAN_SIZE_IN_BYTES = 1;
            /**
            * Char size in bytes
            */
            static const unsigned  int CHAR_SIZE_IN_BYTES = 2;
            /**
            * Short size in bytes
            */
            static const unsigned int SHORT_SIZE_IN_BYTES = 2;
            /**
            * Integer size in bytes
            */
            static const unsigned int INT_SIZE_IN_BYTES = 4;
            /**
            * Float size in bytes
            */
            static const unsigned int FLOAT_SIZE_IN_BYTES = 4;
            /**
            * Long size in bytes
            */
            static const unsigned int LONG_SIZE_IN_BYTES = 8;
            /**
            * Double size in bytes
            */
            static const unsigned int DOUBLE_SIZE_IN_BYTES = 8;

            /**
             * for null arrays, this value writen to stream to represent null array size.
             */
            static const int NULL_ARRAY = -1;

            static int readIntB(std::vector<byte> &buffer, unsigned long pos);

        private :
            /**
            * Make the default constructor and the destructor inaccessible from public.
            */
            Bits();

            virtual ~Bits();

            static void swap_4(void *orig, void *target);
        };
    }
}


#endif //HAZELCAST_Bits
