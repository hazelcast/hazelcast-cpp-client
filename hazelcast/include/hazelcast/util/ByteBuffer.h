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
//
// Created by sancar koyunlu on 31/12/13.
//


#ifndef HAZELCAST_ByteBuffer
#define HAZELCAST_ByteBuffer

#include "hazelcast/util/HazelcastDll.h"
#include <vector>
#include <cstdlib>
#include <memory>

namespace hazelcast {
    namespace util {
        class HAZELCAST_API ByteBuffer {
        public:
            ByteBuffer(char* buffer, size_t size);

            ByteBuffer &flip();

            ByteBuffer &compact();

            ByteBuffer &clear();

            size_t remaining() const;

            bool hasRemaining() const;

            size_t position() const;

            int readInt();

            short readShort();

            void writeInt(int v);

            void writeShort(short v);

            byte readByte();

            /**
            * returns the number of bytes put into target which shall be less or equal to len
            * @param len: requested maximum size to to read
            */
            size_t readBytes(byte *target, size_t len);

            void writeByte(char c);

            void *ix() const;

            void safeIncrementPosition(size_t);

        private:
            size_t pos;
            size_t lim;
            size_t capacity;
            char *buffer;
        };
    }
}

#endif //HAZELCAST_ByteBuffer

