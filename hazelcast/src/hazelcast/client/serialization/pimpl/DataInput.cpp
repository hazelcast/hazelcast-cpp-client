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

#include "hazelcast/util/Util.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/exception/UTFDataFormatException.h"

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
                    return readByte() != 0;
                }

                byte DataInput::readByte() {
                    checkAvailable(1);
                    return buffer[pos++];
                }

                int16_t DataInput::readShort() {
                    byte a = readByte();
                    byte b = readByte();
                    return (int16_t)((0xff00 & (a << 8)) |
                            (0x00ff & b));
                }

                char DataInput::readChar() {
                    readByte();
                    byte b = readByte();
                    return b;
                }

                int32_t DataInput::readInt() {
                    byte a = readByte();
                    byte b = readByte();
                    byte c = readByte();
                    byte d = readByte();
                    return (0xff000000 & (a << 24)) |
                            (0x00ff0000 & (b << 16)) |
                            (0x0000ff00 & (c << 8)) |
                            (0x000000ff & d);
                }

                int64_t DataInput::readLong() {
                    byte a = readByte();
                    byte b = readByte();
                    byte c = readByte();
                    byte d = readByte();
                    byte e = readByte();
                    byte f = readByte();
                    byte g = readByte();
                    byte h = readByte();
                    return (0xff00000000000000LL & ((int64_t) (a) << 56)) |
                            (0x00ff000000000000LL & ((int64_t) (b) << 48)) |
                            (0x0000ff0000000000LL & ((int64_t) (c) << 40)) |
                            (0x000000ff00000000LL & ((int64_t) (d) << 32)) |
                            (0x00000000ff000000LL & (e << 24)) |
                            (0x0000000000ff0000LL & (f << 16)) |
                            (0x000000000000ff00LL & (g << 8)) |
                            (0x00000000000000ffLL & h);
                }

                float DataInput::readFloat() {
                    union {
                        int32_t i;
                        float f;
                    } u;
                    u.i = readInt();
                    return u.f;
                }

                double DataInput::readDouble() {
                    union {
                        double d;
                        int64_t l;
                    } u;
                    u.l = readLong();
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
                        values->push_back(*readUTF());
                    }
                    return values;
                }

                std::auto_ptr<std::vector<std::string *> > DataInput::readStringArray() {
                    int32_t len = readInt();
                    if (util::Bits::NULL_ARRAY == len) {
                        return std::auto_ptr<std::vector<std::string *> >();
                    }

                    std::auto_ptr<std::vector<std::string *> > values(
                            new std::vector<std::string *>());
                    for (int32_t i = 0; i < len; ++i) {
                        values->push_back(readUTF().release());
                    }
                    return values;
                }

                void DataInput::checkAvailable(size_t requestedLength) {
                    size_t available = buffer.size() - pos;

                    if (requestedLength > available) {
                        char msg[100];
                        util::snprintf(msg, 100, "Not enough bytes in internal buffer. Available:%lu bytes but needed %lu bytes", (unsigned long)available, (unsigned long)requestedLength);
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
                    return readByte();
                }

                template <>
                char DataInput::read() {
                    return readChar();
                }

                template <>
                bool DataInput::read() {
                    return readBoolean();
                }

                template <>
                int16_t DataInput::read() {
                    return readShort();
                }

                template <>
                int32_t DataInput::read() {
                    return readInt();
                }

                template <>
                int64_t DataInput::read() {
                    return readLong();
                }

                template <>
                float DataInput::read() {
                    return readFloat();
                }

                template <>
                double DataInput::read() {
                    return readDouble();
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
