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
// Created by sancar koyunlu on 8/7/13.

#ifndef HAZELCAST_DataOutput
#define HAZELCAST_DataOutput

#include <memory>
#include <vector>
#include <string>
#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"

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

                    /**
                     *
                     * @return a deep copy of the bytes by constructing a new byte array.
                     */
                    std::unique_ptr<std::vector<byte> > toByteArray();

                    void write(const std::vector<byte> &bytes);

                    void writeBoolean(bool b);

                    void writeByte(int32_t i);

                    void writeShort(int32_t i);

                    void writeChar(int32_t i);

                    void writeInt(int32_t i);

                    void writeLong(int64_t l);

                    void writeFloat(float v);

                    void writeDouble(double v);

                    void writeUTF(const std::string *s);

                    void writeBytes(const byte *bytes, size_t len);

                    void writeByteArray(const std::vector<byte> *data);

                    void writeCharArray(const std::vector<char> *bytes);

                    void writeBooleanArray(const std::vector<bool> *bytes);

                    void writeShortArray(const std::vector<int16_t> *data);

                    void writeIntArray(const std::vector<int32_t> *data);

                    void writeLongArray(const std::vector<int64_t> *data);

                    void writeFloatArray(const std::vector<float> *data);

                    void writeDoubleArray(const std::vector<double> *data);

                    void writeUTFArray(const std::vector<std::string> *data);

                    void writeByte(int index, int32_t i);

                    void writeInt(int index, int32_t v);

                    void writeZeroBytes(int numberOfBytes);

                    size_t position();

                    void position(size_t newPos);

                    static size_t const DEFAULT_SIZE;

                private:
                    std::unique_ptr< std::vector<byte> > outputStream;

                    DataOutput(const DataOutput &rhs);

                    DataOutput &operator = (const DataOutput &rhs);

                    int getUTF8CharCount(const std::string &str);
                    
                    template <typename T>
                    void write(const T &value) {
                        BOOST_THROW_EXCEPTION(
                                exception::HazelcastSerializationException("DataOutput::write", "Unsupported type"));
                    }
                    
                    template <typename T>
                    void writeArray(const std::vector<T> *data) {
                        int32_t len = (NULL == data ? util::Bits::NULL_ARRAY : (int32_t) data->size());
                        writeInt(len);

                        if (len > 0) {
                            for (int32_t i = 0; i < len; ++i) {
                                write<T>((*data)[i]);
                            }
                        }
                    }
                };

                template <>
                HAZELCAST_API void DataOutput::write(const byte &value);

                template <>
                HAZELCAST_API void DataOutput::write(const char &value);

                template <>
                HAZELCAST_API void DataOutput::write(const bool &value);

                template <>
                HAZELCAST_API void DataOutput::write(const int16_t &value);

                template <>
                HAZELCAST_API void DataOutput::write(const int32_t &value);

                template <>
                HAZELCAST_API void DataOutput::write(const int64_t &value);

                template <>
                HAZELCAST_API void DataOutput::write(const float &value);

                template <>
                HAZELCAST_API void DataOutput::write(const double &value);

                template <>
                HAZELCAST_API void DataOutput::write(const std::string &value);
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_DataOutput

