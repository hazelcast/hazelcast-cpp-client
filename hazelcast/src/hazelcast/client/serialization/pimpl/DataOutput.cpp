//
// Created by sancar koyunlu on 8/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DataOutput::DataOutput()
                : outputStream(new std::vector<byte>()) {
                    outputStream->reserve(DEFAULT_SIZE);
                };


                DataOutput::~DataOutput() {

                };

                DataOutput::DataOutput(DataOutput const &rhs) {
                    //private
                };

                DataOutput &DataOutput::operator = (DataOutput const &rhs) {
                    //private
                    return *this;
                };

                std::auto_ptr< std::vector<byte> > DataOutput::toByteArray() {
                    return outputStream;
                };

                void DataOutput::write(const std::vector<byte> &bytes) {
                    outputStream->insert(outputStream->end(), bytes.begin(), bytes.end());
                };

                void DataOutput::writeBoolean(bool i) {
                    writeByte(i);
                };

                void DataOutput::writeByte(int index, int i) {
                    (*outputStream)[index] = char(0xff & i);
                }

                void DataOutput::writeByte(byte i) {
                    outputStream->push_back(i);
                };

                void DataOutput::writeShort(int v) {
                    writeByte((v >> 8));
                    writeByte(v);
                };

                void DataOutput::writeChar(int i) {
                    writeByte((i >> 8));
                    writeByte(i);
                };

                void DataOutput::writeInt(int v) {
                    writeByte((v >> 24));
                    writeByte((v >> 16));
                    writeByte((v >> 8));
                    writeByte(v);
                };

                void DataOutput::writeLong(long long l) {
                    writeByte((l >> 56));
                    writeByte((l >> 48));
                    writeByte((l >> 40));
                    writeByte((l >> 32));
                    writeByte((l >> 24));
                    writeByte((l >> 16));
                    writeByte((l >> 8));
                    writeByte(l);
                };

                void DataOutput::writeFloat(float x) {
                    union {
                        float f;
                        int i;
                    } u;
                    u.f = x;
                    writeInt(u.i);
                };

                void DataOutput::writeDouble(double v) {
                    union {
                        double d;
                        long long l;
                    } u;
                    u.d = v;
                    writeLong(u.l);
                };

                void DataOutput::writeUTF(const std::string &str) {
                    bool isNull = str.empty();
                    writeBoolean(isNull);
                    if (isNull)
                        return;

                    int length = (int) str.length();
                    writeInt(length);
                    int chunkSize = length / STRING_CHUNK_SIZE + 1;
                    for (int i = 0; i < chunkSize; i++) {
                        int beginIndex = std::max(0, i * STRING_CHUNK_SIZE - 1);
                        int endIndex = std::min((i + 1) * STRING_CHUNK_SIZE - 1, length);
                        writeShortUTF(str.substr(beginIndex, endIndex - beginIndex));
                    }
                };

                void DataOutput::writeInt(int index, int v) {
                    writeByte(index++, (v >> 24));
                    writeByte(index++, (v >> 16));
                    writeByte(index++, (v >> 8));
                    writeByte(index, v);
                };

                void DataOutput::writeByteArray(const std::vector<byte> &data) {
                    writeInt(data.size());
                    outputStream->insert(outputStream->end(), data.begin(), data.end());
                };

                void DataOutput::writeCharArray(const std::vector<char> &data) {
                    int size = data.size();
                    writeInt(size);
                    for (int i = 0; i < size; ++i) {
                        writeChar(data[i]);
                    }
                };

                void DataOutput::writeShortArray(const std::vector<short > &data) {
                    int size = data.size();
                    writeInt(size);
                    for (int i = 0; i < size; ++i) {
                        writeShort(data[i]);
                    }
                };

                void DataOutput::writeIntArray(const std::vector<int> &data) {
                    int size = data.size();
                    writeInt(size);
                    for (int i = 0; i < size; ++i) {
                        writeInt(data[i]);
                    }
                };

                void DataOutput::writeLongArray(const std::vector<long > &data) {
                    int size = data.size();
                    writeInt(size);
                    for (int i = 0; i < size; ++i) {
                        writeLong(data[i]);
                    }
                };

                void DataOutput::writeFloatArray(const std::vector<float > &data) {
                    int size = data.size();
                    writeInt(size);
                    for (int i = 0; i < size; ++i) {
                        writeFloat(data[i]);
                    }
                };

                void DataOutput::writeDoubleArray(const std::vector<double > &data) {
                    int size = data.size();
                    writeInt(size);
                    for (int i = 0; i < size; ++i) {
                        writeDouble(data[i]);
                    }
                };

                int DataOutput::position() {
                    return outputStream->size();
                };

                void DataOutput::position(size_t newPos) {
                    if (outputStream->size() < newPos)
                        outputStream->resize(newPos, 0);
                };

                //private functions

                void DataOutput::writeShortUTF(const std::string &str) {
                    int stringLen = (int) str.length();
                    int utfLength = 0;
                    int count = 0;
                    /* use charAt instead of copying String to char std::vector */
                    for (int i = 0; i < stringLen; i++) {
                        if ((str[i] >= 0x0001) && (str[i] <= 0x007F)) {
                            utfLength++;
//                        } else if (str[i] > 0x07FF) { //MTODO think using wchar
//                            utfLength += 3;
                        } else {
                            utfLength += 2;
                        }
                    }
                    if (utfLength > 65535) {
                        throw exception::IOException("BufferedDataOutput::writeShortUTF", "encoded string too long:" + util::to_string(utfLength) + " bytes");
                    }
                    std::vector<byte> byteArray(utfLength);
                    int i;
                    for (i = 0; i < stringLen; i++) {
                        if (!((str[i] >= 0x0001) && (str[i] <= 0x007F)))
                            break;
                        byteArray[count++] = (byte) str[i];
                    }
                    for (; i < stringLen; i++) {
                        if ((str[i] >= 0x0001) && (str[i] <= 0x007F)) {
                            byteArray[count++] = (byte) str[i];
//                        } else if (str[i] > 0x07FF) {  //MTODO think using wchar
//                            byteArray[count++] = (byte) (0xE0 | ((str[i] >> 12) & 0x0F));
//                            byteArray[count++] = (byte) (0x80 | ((str[i] >> 6) & 0x3F));
//                            byteArray[count++] = (byte) (0x80 | ((str[i]) & 0x3F));
                        } else {
                            byteArray[count++] = (byte) (0xC0 | ((str[i] >> 6) & 0x1F));
                            byteArray[count++] = (byte) (0x80 | ((str[i]) & 0x3F));
                        }
                    }
                    writeShort(utfLength);
                    write(byteArray);
                };
            }

        }
    }
}
