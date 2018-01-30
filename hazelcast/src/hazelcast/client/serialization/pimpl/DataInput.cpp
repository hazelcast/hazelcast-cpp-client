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

#include <string.h>
#include <memory>
#include <boost/foreach.hpp>

#include "hazelcast/util/Util.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/util/IOUtil.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                DataInput::DataInput(const std::vector<byte> &buf)
                :buffer(buf)
                , pos(0) {
                }

                DataInput::DataInput(const std::vector<byte> &buf, int offset)
                        : buffer(buf), pos(offset) {
                }

                void DataInput::readFully(std::vector<byte> &bytes) {
                    size_t length = bytes.size();
                    checkAvailable(length);
                    memcpy(&(bytes[0]), &(buffer[pos]) , length);
                    pos += length;
                }

                void DataInput::readFully(std::vector<char> &chars) {
                    size_t length = chars.size();
                    checkAvailable(length);
                    memcpy(&(chars[0]), &(buffer[pos]) , length);
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

                std::auto_ptr<std::string> DataInput::readUTF() {
                    int32_t len = readInt();
                    if (util::Bits::NULL_ARRAY == len) {
                        return std::auto_ptr<std::string>(NULL);
                    } else {
                        int numBytesToRead = 0;
                        for (int i = 0; i < len ; ++i) {
                            checkAvailable(1);
                            int numBytesForChar = getNumBytesForUtf8Char(&buffer[pos] + numBytesToRead);
                            numBytesToRead += numBytesForChar;
                            checkAvailable(numBytesToRead);
                        }

                        const std::vector<unsigned char>::const_iterator start = buffer.begin() + pos;
                        std::auto_ptr<std::string> result(new std::string(start, start + numBytesToRead));
                        pos += numBytesToRead;
                        return result;
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

                std::auto_ptr<std::vector<byte> > DataInput::readByteArray() {
                    return readArray<byte>();
                }

                std::auto_ptr<std::vector<bool> > DataInput::readBooleanArray() {
                    return readArray<bool>();
                }

                std::auto_ptr<std::vector<char> > DataInput::readCharArray() {
                    return readArray<char>();
                }

                std::auto_ptr<std::vector<int32_t> > DataInput::readIntArray() {
                    return readArray<int32_t>();
                }

                std::auto_ptr<std::vector<int64_t> > DataInput::readLongArray() {
                    return readArray<int64_t>();
                }

                std::auto_ptr<std::vector<double> > DataInput::readDoubleArray() {
                    return readArray<double>();
                }

                std::auto_ptr<std::vector<float> > DataInput::readFloatArray() {
                    return readArray<float>();
                }

                std::auto_ptr<std::vector<int16_t> > DataInput::readShortArray() {
                    return readArray<int16_t>();
                }

                std::auto_ptr<std::vector<std::string> > DataInput::readUTFArray() {
                    int32_t len = readInt();
                    if (util::Bits::NULL_ARRAY == len) {
                        return std::auto_ptr<std::vector<std::string> >();
                    }

                    std::auto_ptr<std::vector<std::string> > values(
                            new std::vector<std::string>());
                    for (int32_t i = 0; i < len; ++i) {
                        std::auto_ptr<std::string> value = readUTF();
                        // handle null pointer possibility
                        if ((std::string *)NULL == value.get()) {
                            values->push_back(std::string(""));
                        } else {
                            values->push_back(*value);
                        }
                    }
                    return values;
                }

                std::auto_ptr<std::vector<std::string *> > DataInput::readUTFPointerArray() {
                    int32_t len = readInt();
                    if (util::Bits::NULL_ARRAY == len) {
                        return std::auto_ptr<std::vector<std::string *> >();
                    }

                    std::auto_ptr<std::vector<std::string *> > values(
                            new std::vector<std::string *>());
                    try {
                        for (int32_t i = 0; i < len; ++i) {
                            values->push_back(readUTF().release());
                        }
                    } catch (exception::IException &) {
                        // clean resources to avoid any leaks
                        typedef std::vector<std::string *> STRING_ARRAY;
                        BOOST_FOREACH(STRING_ARRAY::value_type value , *values) {
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

                int DataInput::getSize(byte *dummy) {
                    return util::Bits::BYTE_SIZE_IN_BYTES;
                }

                int DataInput::getSize(char *dummy) {
                    return util::Bits::CHAR_SIZE_IN_BYTES;
                }

                int DataInput::getSize(bool *dummy) {
                    return util::Bits::BOOLEAN_SIZE_IN_BYTES;
                }

                int DataInput::getSize(int16_t *dummy) {
                    return util::Bits::SHORT_SIZE_IN_BYTES;
                }

                int DataInput::getSize(int32_t *dummy) {
                    return util::Bits::INT_SIZE_IN_BYTES;
                }

                int DataInput::getSize(int64_t *dummy) {
                    return util::Bits::LONG_SIZE_IN_BYTES;
                }

                int DataInput::getSize(float *dummy) {
                    return util::Bits::FLOAT_SIZE_IN_BYTES;
                }

                int DataInput::getSize(double *dummy) {
                    return util::Bits::DOUBLE_SIZE_IN_BYTES;
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

                int DataInput::getNumBytesForUtf8Char(const byte *start) const {
                    char first = *start;
                    int b = first & 0xFF;
                    switch (b >> 4) {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                            return 1;
                        case 12:
                        case 13: {
                            return 2;
                        }
                        case 14: {
                            return 3;
                        }
                        default:
                            throw exception::UTFDataFormatException("DataInput::getNumBytesForUtf8Char",
                                                                    "Malformed byte sequence");
                    }
                }
            }
        }
    }
}
