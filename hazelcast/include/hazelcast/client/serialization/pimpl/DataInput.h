/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_DataInput
#define HAZELCAST_DataInput

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"

#include <boost/smart_ptr/shared_ptr.hpp>

#include <vector>
#include <string>
#include <memory>
#include <stdint.h>
#include <sstream>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util{
        class ByteBuffer;
    }
    namespace client {
        namespace serialization {
            namespace pimpl {
                class HAZELCAST_API DataInput {
                public:
                    DataInput(const std::vector<byte> &buffer);

                    DataInput(const std::vector<byte> &buffer, int offset);

                    void readFully(std::vector<byte> &);

                    int skipBytes(int i);

                    bool readBoolean();

                    byte readByte();

                    int16_t readShort();

                    // TODO: change to return 2 bytes char as in java
                    char readChar();

                    int32_t readInt();

                    int64_t readLong();

                    float readFloat();

                    double readDouble();

                    std::auto_ptr<std::string> readUTF();

                    std::auto_ptr<std::vector<byte> > readByteArray();

                    std::auto_ptr<std::vector<bool> > readBooleanArray();

                    std::auto_ptr<std::vector<char> > readCharArray();

                    std::auto_ptr<std::vector<int32_t> > readIntArray();

                    std::auto_ptr<std::vector<int64_t> > readLongArray();

                    std::auto_ptr<std::vector<double> > readDoubleArray();

                    std::auto_ptr<std::vector<float> > readFloatArray();

                    std::auto_ptr<std::vector<int16_t> > readShortArray();

                    std::auto_ptr<std::vector<std::string> > readUTFArray();

                    std::auto_ptr<std::vector<std::string *> > readUTFPointerArray();

                    int position();

                    void position(int position);

                private:
                    const std::vector<byte> &buffer;

                    int pos;

                    template <typename T>
                    inline T read() {
                        throw exception::HazelcastSerializationException("DataInput::read", "Unsupported type");
                    }

                    template <typename T>
                    inline int getSize(T *dummy) {
                        throw exception::HazelcastSerializationException("DataInput::getSize", "Unsupported type");
                    }

                    template <typename T>
                    inline std::auto_ptr<std::vector<T> > readArray() {
                        int32_t len = readInt();
                        if (util::Bits::NULL_ARRAY == len) {
                            return std::auto_ptr<std::vector<T> > (NULL);
                        }

                        if (len == 0) {
                            return std::auto_ptr<std::vector<T> > (new std::vector<T>(0));
                        }

                        if (len < 0) {
                            std::ostringstream out;
                            out << "Incorrect negative array size found in the byte stream. The size is: " << len;
                            throw exception::HazelcastSerializationException("DataInput::readArray", out.str());
                        }

                        std::auto_ptr<std::vector<T> > values(new std::vector<T>((size_t)len));
                        for (int32_t i = 0; i < len; i++) {
                            (*values)[i] = read<T>();
                        }
                        return values;
                    }

                    int getNumBytesForUtf8Char(const byte *start) const;

                    DataInput(const DataInput &);

                    DataInput &operator = (const DataInput &);

                    void checkAvailable(size_t requestedLength);

                    char readCharUnchecked();

                    int16_t readShortUnchecked();

                    int32_t readIntUnchecked();

                    int64_t readLongUnchecked();

                    byte readByteUnchecked();

                    bool readBooleanUnchecked();

                    float readFloatUnchecked();

                    double readDoubleUnchecked();
                };

                template <>
                HAZELCAST_API byte DataInput::read();

                template <>
                HAZELCAST_API char DataInput::read();

                template <>
                HAZELCAST_API bool DataInput::read();

                template <>
                HAZELCAST_API int16_t DataInput::read();

                template <>
                HAZELCAST_API int32_t DataInput::read();

                template <>
                HAZELCAST_API int64_t DataInput::read();

                template <>
                HAZELCAST_API float DataInput::read();

                template <>
                HAZELCAST_API double DataInput::read();
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_DataInput

