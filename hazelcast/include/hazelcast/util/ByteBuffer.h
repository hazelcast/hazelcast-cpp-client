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

#include "hazelcast/util/byte.h"
#include "hazelcast/util/export.h"
#include <vector>
#include <cstdlib>
#include <memory>

namespace hazelcast {
namespace util {
class HAZELCAST_API ByteBuffer
{
public:
    ByteBuffer(char* buffer, size_t size);

    ByteBuffer& flip();

    ByteBuffer& compact();

    ByteBuffer& clear();

    size_t remaining() const;

    bool has_remaining() const;

    size_t position() const;

    int read_int();

    short read_short();

    void write_int(int v);

    void write_short(short v);

    byte read_byte();

    /**
     * returns the number of bytes put into target which shall be less or equal to len
     * @param len: requested maximum size to to read
     */
    template<typename T>
    size_t read_bytes(T& target, size_t len)
    {
        size_t numBytesToCopy = std::min<size_t>(lim_ - pos_, len);
        target.insert(target.end(), buffer_ + pos_, buffer_ + pos_ + numBytesToCopy);
        pos_ += numBytesToCopy;
        return numBytesToCopy;
    }

    inline void read_bytes(byte* dest, size_t len)
    {
        std::memcpy(dest, buffer_ + pos_, len);
        pos_ += len;
    }

    void write_byte(char c);

    void* ix() const;

    void safe_increment_position(size_t);

private:
    size_t pos_;
    size_t lim_;
    size_t capacity_;
    char* buffer_;
};
} // namespace util
} // namespace hazelcast
