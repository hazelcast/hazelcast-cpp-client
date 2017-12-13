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

#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/util/IOUtil.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                size_t const DataOutput::DEFAULT_SIZE = 4 * 1024;

                DataOutput::DataOutput()
                : outputStream(new std::vector<byte>()) {
                    outputStream->reserve(DEFAULT_SIZE);
                }


                DataOutput::~DataOutput() {
                }

                DataOutput::DataOutput(DataOutput const& rhs)
                {
                    //private
                }

                DataOutput& DataOutput::operator=(DataOutput const& rhs) {
                    //private
                    return *this;
                }

                std::auto_ptr<std::vector<byte> > DataOutput::toByteArray() {
                    std::auto_ptr<std::vector<byte> > byteArrayPtr(new std::vector<byte>(*outputStream));
                    return byteArrayPtr;
                }

                void DataOutput::write(const std::vector<byte>& bytes) {
                    outputStream->insert(outputStream->end(), bytes.begin(), bytes.end());
                }

                void DataOutput::writeBoolean(bool i) {
                    writeByte((byte)i);
                }

                void DataOutput::writeByte(int index, int32_t i) {
                    (*outputStream)[index] = byte(0xff & i);
                }

                void DataOutput::writeByte(int32_t i) {
                    outputStream->push_back(byte(0xff & i));
                }

                void DataOutput::writeShort(int32_t v) {
                    int16_t value = (int16_t) v;
                    int16_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian2(&value, &result);
                    outputStream->insert(outputStream->end(), target, target + util::Bits::SHORT_SIZE_IN_BYTES);
                }

                void DataOutput::writeChar(int32_t i) {
                    writeByte((byte)(i >> 8));
                    writeByte((byte)i);
                }

                void DataOutput::writeInt(int32_t v) {
                    int32_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian4(&v, &result);
                    outputStream->insert(outputStream->end(), target, target + util::Bits::INT_SIZE_IN_BYTES);
                }

                void DataOutput::writeLong(int64_t l) {
                    int64_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian8(&l, &result);
                    outputStream->insert(outputStream->end(), target, target + util::Bits::LONG_SIZE_IN_BYTES);
                }

                void DataOutput::writeFloat(float x) {
                    union {
                        float f;
                        int32_t i;
                    } u;
                    u.f = x;
                    writeInt(u.i);
                }

                void DataOutput::writeDouble(double v) {
                    union {
                        double d;
                        int64_t l;
                    } u;
                    u.d = v;
                    writeLong(u.l);
                }

                void DataOutput::writeUTF(const std::string *str) {
                    int32_t len = (NULL != str) ? getUTF8CharCount(*str) : util::Bits::NULL_ARRAY;
                    writeInt(len);
                    if (len > 0) {
                        outputStream->insert(outputStream->end(), str->begin(), str->end());
                    }
                }

                void DataOutput::writeInt(int index, int32_t v) {
                    int32_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian4(&v, &result);
                    (*outputStream)[index++] = *target++;
                    (*outputStream)[index++] = *target++;
                    (*outputStream)[index++] = *target++;
                    (*outputStream)[index] = *target;
                }

                void DataOutput::writeBytes(const byte *bytes, size_t len) {
                    outputStream->insert(outputStream->end(), bytes, bytes + len);
                }

                void DataOutput::writeByteArray(const std::vector<byte> *data) {
                    writeArray<byte>(data);
                }

                void DataOutput::writeCharArray(const std::vector<char> *data) {
                    writeArray<char>(data);
                }

                void DataOutput::writeBooleanArray(const std::vector<bool> *data) {
                    writeArray<bool>(data);
                }

                void DataOutput::writeShortArray(const std::vector<int16_t> *data) {
                    writeArray<int16_t>(data);
                }

                void DataOutput::writeIntArray(const std::vector<int32_t> *data) {
                    writeArray<int32_t>(data);
                }

                void DataOutput::writeLongArray(const std::vector<int64_t> *data) {
                    writeArray<int64_t>(data);
                }

                void DataOutput::writeFloatArray(const std::vector<float> *data) {
                    writeArray<float>(data);
                }

                void DataOutput::writeDoubleArray(const std::vector<double> *data) {
                    writeArray<double>(data);
                }

                void DataOutput::writeUTFArray(const std::vector<std::string> *data) {
                    writeArray<std::string>(data);
                }

                void DataOutput::writeZeroBytes(int numberOfBytes) {
                    for (int k = 0; k < numberOfBytes; k++) {
                        writeByte(0);
                    }
                }

                size_t DataOutput::position() {
                    return outputStream->size();
                }

                void DataOutput::position(size_t newPos) {
                    if (outputStream->size() < newPos)
                        outputStream->resize(newPos, 0);
                }

                int DataOutput::getUTF8CharCount(const std::string &str) {
                    int size = 0;
                    for (std::string::const_iterator it = str.begin();it != str.end();++it) {
                        // Any additional byte for an UTF character has a bit mask of 10xxxxxx
                        size += (*it & 0xC0) != 0x80;
                    }

                    return size;
                }

                template<>
                void DataOutput::write(const byte &value) {
                    writeByte(value);
                }

                template<>
                void DataOutput::write(const char &value) {
                    writeChar(value);
                }

                template<>
                void DataOutput::write(const bool &value) {
                    writeBoolean(value);
                }

                template<>
                void DataOutput::write(const int16_t &value) {
                    writeShort(value);
                }

                template<>
                void DataOutput::write(const int32_t &value) {
                    writeInt(value);
                }

                template<>
                void DataOutput::write(const int64_t &value) {
                    writeLong(value);
                }

                template<>
                void DataOutput::write(const float &value) {
                    writeFloat(value);
                }

                template<>
                void DataOutput::write(const double &value) {
                    writeDouble(value);
                }

                template<>
                void DataOutput::write(const std::string &value) {
                    writeUTF(&value);
                }
            }
        }
    }
}

