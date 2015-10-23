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
// Created by sancar koyunlu on 8/7/13.



#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/Bits.h"

#include <algorithm>

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

                void DataOutput::writeByte(int index, int i) {
                    (*outputStream)[index] = byte(0xff & i);
                }

                void DataOutput::writeByte(int i) {
                    outputStream->push_back(byte(0xff & i));
                }

                void DataOutput::writeShort(int v) {
                    writeByte((byte)(v >> 8));
                    writeByte((byte)v);
                }

                void DataOutput::writeChar(int i) {
                    writeByte((byte)(i >> 8));
                    writeByte((byte)i);
                }

                void DataOutput::writeInt(int v) {
                    writeByte((byte)(v >> 24));
                    writeByte((byte)(v >> 16));
                    writeByte((byte)(v >> 8));
                    writeByte((byte)v);
                }

                void DataOutput::writeLong(long long l) {
                    writeByte((byte)(l >> 56));
                    writeByte((byte)(l >> 48));
                    writeByte((byte)(l >> 40));
                    writeByte((byte)(l >> 32));
                    writeByte((byte)(l >> 24));
                    writeByte((byte)(l >> 16));
                    writeByte((byte)(l >> 8));
                    writeByte((byte)l);
                }

                void DataOutput::writeFloat(float x) {
                    union {
                        float f;
                        int i;
                    } u;
                    u.f = x;
                    writeInt(u.i);
                }

                void DataOutput::writeDouble(double v) {
                    union {
                        double d;
                        long long l;
                    } u;
                    u.d = v;
                    writeLong(u.l);
                }

                void DataOutput::writeUTF(const std::string *str) {
                    int len = (NULL != str) ? getUTF8CharCount(*str) : util::Bits::NULL_ARRAY;
                    writeInt(len);
                    if (len > 0) {
                        outputStream->insert(outputStream->end(), str->begin(), str->end());
                    }
                }

                void DataOutput::writeInt(int index, int v) {
                    writeByte(index++, (v >> 24));
                    writeByte(index++, (v >> 16));
                    writeByte(index++, (v >> 8));
                    writeByte(index, v);
                }

                void DataOutput::writeBytes(const byte *bytes, unsigned int len) {
                    outputStream->insert(outputStream->end(), bytes, bytes + len);
                }

                void DataOutput::writeByteArray(const std::vector<byte> *data) {
                    int len = (NULL == data ? util::Bits::NULL_ARRAY : (int) data->size());
                    writeInt(len);
                    if (len > 0) {
                        outputStream->insert(outputStream->end(),  data->begin(),  data->end());
                    }
                }

                void DataOutput::writeCharArray(const std::vector<char> *data) {
                    int len = (NULL == data ? util::Bits::NULL_ARRAY : (int) data->size());
                    writeInt(len);
                    if (len > 0) {
                        for (int i = 0; i < len; ++i) {
                            writeChar((*data)[i]);
                        }
                    }
                }

                void DataOutput::writeBooleanArray(const std::vector<bool> *data) {
                    int len = (NULL == data ? util::Bits::NULL_ARRAY : (int) data->size());
                    writeInt(len);
                    if (len > 0) {
                        for (int i = 0; i < len; ++i) {
                            writeBoolean((*data)[i]);
                        }
                    }
                }

                void DataOutput::writeShortArray(const std::vector<short> *data) {
                    int len = (NULL == data ? util::Bits::NULL_ARRAY : (int) data->size());
                    writeInt(len);
                    if (len > 0) {
                        for (int i = 0; i < len; ++i) {
                            writeShort((*data)[i]);
                        }
                    }
                }

                void DataOutput::writeIntArray(const std::vector<int> *data) {
                    int len = (NULL == data ? util::Bits::NULL_ARRAY : (int) data->size());
                    writeInt(len);
                    if (len > 0) {
                        for (int i = 0; i < len; ++i) {
                            writeInt((*data)[i]);
                        }
                    }
                }

                void DataOutput::writeLongArray(const std::vector<long> *data) {
                    int len = (NULL == data ? util::Bits::NULL_ARRAY : (int) data->size());
                    writeInt(len);
                    if (len > 0) {
                        for (int i = 0; i < len; ++i) {
                            writeLong((*data)[i]);
                        }
                    }
                }

                void DataOutput::writeFloatArray(const std::vector<float> *data) {
                    int len = (NULL == data ? util::Bits::NULL_ARRAY : (int) data->size());
                    writeInt(len);
                    if (len > 0) {
                        for (int i = 0; i < len; ++i) {
                            writeFloat((*data)[i]);
                        }
                    }
                }

                void DataOutput::writeDoubleArray(const std::vector<double> *data) {
                    int len = (NULL == data ? util::Bits::NULL_ARRAY : (int) data->size());
                    writeInt(len);
                    if (len > 0) {
                        for (int i = 0; i < len; ++i) {
                            writeDouble((*data)[i]);
                        }
                    }
                }

                void DataOutput::writeUTFArray(const std::vector<const std::string *> *data) {
                    int len = (NULL != data) ? (int)data->size() : util::Bits::NULL_ARRAY;
                    writeInt(len);
                    if (len > 0) {
                        for (int i = 0; i < len; ++i) {
                            writeUTF((*data)[i]);
                        }
                    }
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
                    for (std::string::const_iterator it = str.begin();it != str.end();) {
                        byte buffer[3];
                        buffer[0] = (unsigned char)(*it);
                        if (buffer[0] <= 0x7F) {
                            ++it;
                            ++size;
                        } else {
                            ++it;
                            if (it != str.end()) {
                                buffer[1] = (unsigned char)(*it);
                                unsigned short twoByteChar = ((buffer[0] << 8) | (buffer[1]));
                                if (twoByteChar > 0x07FF) {
                                    it += 2;
                                    ++size;
                                } else {
                                    ++it;
                                    ++size;
                                }
                            }
                        }
                    }
                    return size;
                }
            }

        }
    }
}

