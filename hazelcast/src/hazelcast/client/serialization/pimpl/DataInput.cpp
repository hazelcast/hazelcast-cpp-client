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
// Created by sancar koyunlu on 8/7/13.

#include <string.h>
#include <memory>


#include "hazelcast/util/Util.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/util/IOUtil.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                DataInput::DataInput(const std::vector<byte> &buf) : buffer(buf), pos(0) {
                }

                DataInput::DataInput(const std::vector<byte> &buf, int offset)
                        : buffer(buf), pos(offset) {
                }

                DataInput::~DataInput() {
                }

                void DataInput::readFully(std::vector<byte> &bytes) {
                    size_t length = bytes.size();
                    checkAvailable(length);
                    memcpy(&(bytes[0]), &(buffer[pos]) , length);
                    pos += length;
                }

                int DataInput::skipBytes(int i) {
                    checkAvailable(i);
                    pos += i;
                    return i;
                }

                bool DataInput::readBoolean() {
                    checkAvailable(1);
                    return readBooleanUnchecked();
                }

                bool DataInput::readBooleanUnchecked() { return readByteUnchecked() != 0; }

                byte DataInput::readByte() {
                    checkAvailable(1);
                    return readByteUnchecked();
                }

                byte DataInput::readByteUnchecked() { return buffer[pos++]; }

                int16_t DataInput::readShort() {
                    checkAvailable(util::Bits::SHORT_SIZE_IN_BYTES);
                    return readShortUnchecked();
                }

                int16_t DataInput::readShortUnchecked() {
                    int16_t result;
                    util::Bits::bigEndianToNative2(&buffer[pos], &result);
                    pos += util::Bits::SHORT_SIZE_IN_BYTES;
                    return result;
                }

                char DataInput::readChar() {
                    checkAvailable(util::Bits::CHAR_SIZE_IN_BYTES);
                    return readCharUnchecked();
                }

                char DataInput::readCharUnchecked() {
                    // skip the first byte
                    byte b = buffer[pos + 1];
                    pos += util::Bits::CHAR_SIZE_IN_BYTES;
                    return b;
                }

                int32_t DataInput::readInt() {
                    checkAvailable(util::Bits::INT_SIZE_IN_BYTES);
                    return readIntUnchecked();
                }

                int32_t DataInput::readIntUnchecked() {
                    int32_t result;
                    util::Bits::bigEndianToNative4(&buffer[pos], &result);
                    pos += util::Bits::INT_SIZE_IN_BYTES;
                    return result;
                }

                int64_t DataInput::readLong() {
                    checkAvailable(util::Bits::LONG_SIZE_IN_BYTES);
                    return readLongUnchecked();
                }

                int64_t DataInput::readLongUnchecked() {
                    int64_t result;
                    util::Bits::bigEndianToNative8(&buffer[pos], &result);
                    pos += util::Bits::LONG_SIZE_IN_BYTES;
                    return result;
                }

                float DataInput::readFloat() {
                    checkAvailable(util::Bits::FLOAT_SIZE_IN_BYTES);
                    return readFloatUnchecked();
                }

                float DataInput::readFloatUnchecked() {
                    union {
                        int32_t i;
                        float f;
                    } u;
                    u.i = readIntUnchecked();
                    return u.f;
                }

                double DataInput::readDouble() {
                    checkAvailable(util::Bits::DOUBLE_SIZE_IN_BYTES);
                    return readDoubleUnchecked();
                }

                double DataInput::readDoubleUnchecked() {
                    union {
                        double d;
                        int64_t l;
                    } u;
                    u.l = readLongUnchecked();
                    return u.d;
                }

                std::unique_ptr<std::string> DataInput::readUTF() {
                    int32_t charCount = readInt();
                    if (util::Bits::NULL_ARRAY == charCount) {
                        return std::unique_ptr<std::string>();
                    } else {
                        utfBuffer.clear();
                        utfBuffer.reserve((size_t) MAX_UTF_CHAR_SIZE * charCount);
                        byte b;
                        for (int i = 0; i < charCount; ++i) {
                            b = readByte();
                            util::UTFUtil::readUTF8Char(*this, b, utfBuffer);
                        }

                        return std::unique_ptr<std::string>(new std::string(utfBuffer.begin(), utfBuffer.end()));
                    }
                }

                int DataInput::position() {
                    return pos;
                }

                void DataInput::position(int position) {
                    if(position > pos){
                        checkAvailable((size_t)(position - pos));
                    }
                    pos = position;
                }
                //private functions

                std::unique_ptr<std::vector<byte> > DataInput::readByteArray() {
                    return readArray<byte>();
                }

                std::unique_ptr<std::vector<bool> > DataInput::readBooleanArray() {
                    return readArray<bool>();
                }

                std::unique_ptr<std::vector<char> > DataInput::readCharArray() {
                    return readArray<char>();
                }

                std::unique_ptr<std::vector<int32_t> > DataInput::readIntArray() {
                    return readArray<int32_t>();
                }

                std::unique_ptr<std::vector<int64_t> > DataInput::readLongArray() {
                    return readArray<int64_t>();
                }

                std::unique_ptr<std::vector<double> > DataInput::readDoubleArray() {
                    return readArray<double>();
                }

                std::unique_ptr<std::vector<float> > DataInput::readFloatArray() {
                    return readArray<float>();
                }

                std::unique_ptr<std::vector<int16_t> > DataInput::readShortArray() {
                    return readArray<int16_t>();
                }

                std::unique_ptr<std::vector<std::string> > DataInput::readUTFArray() {
                    int32_t len = readInt();
                    if (util::Bits::NULL_ARRAY == len) {
                        return std::unique_ptr<std::vector<std::string> >();
                    }

                    std::unique_ptr<std::vector<std::string> > values(
                            new std::vector<std::string>());
                    for (int32_t i = 0; i < len; ++i) {
                        std::unique_ptr<std::string> value = readUTF();
                        // handle null pointer possibility
                        if ((std::string *)NULL == value.get()) {
                            values->push_back(std::string(""));
                        } else {
                            values->push_back(*value);
                        }
                    }
                    return values;
                }

                std::unique_ptr<std::vector<std::string *> > DataInput::readUTFPointerArray() {
                    int32_t len = readInt();
                    if (util::Bits::NULL_ARRAY == len) {
                        return std::unique_ptr<std::vector<std::string *> >();
                    }

                    std::unique_ptr<std::vector<std::string *> > values(
                            new std::vector<std::string *>());
                    try {
                        for (int32_t i = 0; i < len; ++i) {
                            values->push_back(readUTF().release());
                        }
                    } catch (exception::IException &) {
                        // clean resources to avoid any leaks
                        typedef std::vector<std::string *> STRING_ARRAY;
                        for (STRING_ARRAY::value_type value  : *values) {
                                        delete value;
                                    }
                        throw;
                    }
                    return values;
                }

                void DataInput::checkAvailable(size_t requestedLength) {
                    size_t available = buffer.size() - pos;

                    if (requestedLength > available) {
                        char msg[100];
                        util::hz_snprintf(msg, 100,
                                          "Not enough bytes in internal buffer. Available:%lu bytes but needed %lu bytes",
                                          (unsigned long) available, (unsigned long) requestedLength);
                        throw exception::IOException("DataInput::checkBoundary", msg);
                    }
                }

                template <>
                byte DataInput::read() {
                    return readByteUnchecked();
                }

                template <>
                char DataInput::read() {
                    return readCharUnchecked();
                }

                template <>
                bool DataInput::read() {
                    return readBooleanUnchecked();
                }

                template <>
                int16_t DataInput::read() {
                    return readShortUnchecked();
                }

                template <>
                int32_t DataInput::read() {
                    return readIntUnchecked();
                }

                template <>
                int64_t DataInput::read() {
                    return readLongUnchecked();
                }

                template <>
                float DataInput::read() {
                    return readFloatUnchecked();
                }

                template <>
                double DataInput::read() {
                    return readDoubleUnchecked();
                }
            }
        }
    }
}
