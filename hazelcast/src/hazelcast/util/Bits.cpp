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

#include "hazelcast/util/Bits.h"

namespace hazelcast {
    namespace util {
        // Bits class implementation
        Bits::Bits() {
        }

        Bits::~Bits() {
        }

        int Bits::readIntB(std::vector<byte> &buffer, unsigned long pos) {
            #ifdef HZ_BIG_ENDIAN
                return *((int *) (&buffer[0] + pos));
            #else
                int result;
                swap_4(&(buffer[0]) + pos, &result);
                return result;
            #endif
        }


        void Bits::swap_4(void *orig, void *target) {
            // TODO: Change with implementation at CDR_BASE.inl from ACE library
            register int x = * reinterpret_cast<const int*> (orig);
            x = (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
            * reinterpret_cast<int *> (target) = x;
        }
    }
}
