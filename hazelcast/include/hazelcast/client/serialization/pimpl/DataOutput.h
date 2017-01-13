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
//
// Created by sancar koyunlu on 8/7/13.

#ifndef HAZELCAST_DataOutput
#define HAZELCAST_DataOutput

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/ByteBuffer.h"
#include <memory>
#include <vector>
#include <string>
#include <stdint.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class HAZELCAST_API DataOutput {
                public:

                    DataOutput();

                    virtual ~DataOutput();

                    std::auto_ptr< std::vector<byte> > toByteArray();

                    void write(const std::vector<byte> &bytes);

                    void writeBoolean(bool b);

                    void writeByte(int i);

                    void writeShort(int i);

                    void writeChar(int i);

                    void writeInt(int i);

                    void writeLong(int64_t l);

                    void writeFloat(float v);

                    void writeDouble(double v);

                    void writeUTF(const std::string *s);

                    void writeBytes(const byte *bytes, unsigned int len);

                    void writeByteArray(const std::vector<byte> *data);

                    void writeCharArray(const std::vector<char> *bytes);

                    void writeBooleanArray(const std::vector<bool> *bytes);

                    void writeShortArray(const std::vector<short> *data);

                    void writeIntArray(const std::vector<int> *data);

                    void writeLongArray(const std::vector<long> *data);

                    void writeFloatArray(const std::vector<float> *data);

                    void writeDoubleArray(const std::vector<double> *data);

                    void writeUTFArray(const std::vector<std::string> *data);

                    void writeByte(int index, int i);

                    void writeInt(int index, int v);

                    void writeZeroBytes(int numberOfBytes);

                    size_t position();

                    void position(size_t newPos);

                    static size_t const DEFAULT_SIZE;

                private:
                    std::auto_ptr< std::vector<byte> > outputStream;

                    DataOutput(const DataOutput &rhs);

                    DataOutput &operator = (const DataOutput &rhs);

                    int getUTF8CharCount(const std::string &str);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_DataOutput

