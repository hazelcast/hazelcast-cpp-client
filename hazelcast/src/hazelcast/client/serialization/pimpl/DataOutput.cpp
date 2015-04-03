//
// Created by sancar koyunlu on 8/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/IOUtil.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                size_t const DataOutput::STRING_CHUNK_SIZE = 16 * 1024;
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

                void DataOutput::writeByte(byte i) {
                    outputStream->push_back(i);
                }

                void DataOutput::writeShort(short v) {
                    writeByte((byte)(v >> 8));
                    writeByte((byte)v);
                }

                void DataOutput::writeChar(short i) {
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

                void DataOutput::writeUTF(const std::string& str) {
                    bool isNull = str.empty();
                    writeBoolean(isNull);
                    if (isNull)
                        return;

                    size_t length = str.length();
                    writeInt((int)length);
                    writeInt((int)length);
                    size_t chunkSize = length / STRING_CHUNK_SIZE + 1;
                    for (size_t i = 0; i < chunkSize; i++) {
                        using namespace std;
                        size_t beginIndex = (size_t)max((int)0, (int)STRING_CHUNK_SIZE * (int)i - 1);
                        size_t endIndex = min(STRING_CHUNK_SIZE * (i + 1) - 1, length);
                        writeShortUTF(str.substr(beginIndex, endIndex - beginIndex));
                    }
                }

                void DataOutput::writeInt(int index, int v) {
                    writeByte(index++, (v >> 24));
                    writeByte(index++, (v >> 16));
                    writeByte(index++, (v >> 8));
                    writeByte(index, v);
                }

                void DataOutput::writeBytes(const byte *bytes, unsigned int len) {
                    for (unsigned int i = 0; i < len; i++) {
                        writeByte(bytes[i]);
                    }
                }

                void DataOutput::writeByteArray(const std::vector<byte>& data) {
                    writeInt((int)data.size());
                    outputStream->insert(outputStream->end(), data.begin(), data.end());
                }

                void DataOutput::writeCharArray(const std::vector<char>& data) {
                    size_t size = data.size();
                    writeInt((int)size);
                    for (int i = 0; i < size; ++i) {
                        writeChar(data[i]);
                    }
                }

                void DataOutput::writeShortArray(const std::vector<short>& data) {
                    size_t size = data.size();
                    writeInt((int)size);
                    for (int i = 0; i < size; ++i) {
                        writeShort(data[i]);
                    }
                }

                void DataOutput::writeIntArray(const std::vector<int>& data) {
                    size_t size = data.size();
                    writeInt((int)size);
                    for (int i = 0; i < size; ++i) {
                        writeInt(data[i]);
                    }
                }

                void DataOutput::writeLongArray(const std::vector<long>& data) {
                    size_t size = data.size();
                    writeInt((int)size);
                    for (int i = 0; i < size; ++i) {
                        writeLong(data[i]);
                    }
                }

                void DataOutput::writeFloatArray(const std::vector<float>& data) {
                    size_t size = data.size();
                    writeInt((int)size);
                    for (int i = 0; i < size; ++i) {
                        writeFloat(data[i]);
                    }
                }

                void DataOutput::writeDoubleArray(const std::vector<double>& data) {
                    size_t size = data.size();
                    writeInt((int)size);
                    for (int i = 0; i < size; ++i) {
                        writeDouble(data[i]);
                    }
                }

                size_t DataOutput::position() {
                    return outputStream->size();
                }

                void DataOutput::position(size_t newPos) {
                    if (outputStream->size() < newPos)
                        outputStream->resize(newPos, 0);
                }

                //private functions

                void DataOutput::writeShortUTF(const std::string& str) {
                    int stringLen = (int)str.length();
//                    int utfLength = 0;
                    int utfLength = stringLen;
                    int count = 0;
                    /* use charAt instead of copying String to char std::vector */
//                    for (int i = 0; i < stringLen; i++) {
//                        if ((str[i] >= 0x0001) && (str[i] <= 0x007F)) {
//                            utfLength++;
//                        } else if (str[i] > 0x07FF) { //MTODO think using wchar
//                            utfLength += 3;
//                        } else {
//                            utfLength += 2;
//                        }
                    //                   }
                    if (utfLength > 65535) {
                        const std::string& message = "encoded string too long:" +
                        util::IOUtil::to_string(utfLength) + " bytes";
                        throw exception::IOException("BufferedDataOutput::writeShortUTF", message);
                    }
                    std::vector<byte> byteArray((size_t)utfLength);
                    int i;
                    for (i = 0; i < stringLen; i++) {
//                        if (!((str[i] >= 0x0001) && (str[i] <= 0x007F)))
//                            break;
                        byteArray[count++] = (byte)str[i];
                    }
//                    for (; i < stringLen; i++) {
//                        if ((str[i] >= 0x0001) && (str[i] <= 0x007F)) {
//                            byteArray[count++] = (byte) str[i];
//                        } else if (str[i] > 0x07FF) {  //MTODO think using wchar
//                            byteArray[count++] = (byte) (0xE0 | ((str[i] >> 12) & 0x0F));
//                            byteArray[count++] = (byte) (0x80 | ((str[i] >> 6) & 0x3F));
//                            byteArray[count++] = (byte) (0x80 | ((str[i]) & 0x3F));
//                        } else {
//                            byteArray[count++] = (byte) (0xC0 | ((str[i] >> 6) & 0x1F));
//                            byteArray[count++] = (byte) (0x80 | ((str[i]) & 0x3F));
//                        }
//                    }
                    writeShort((short)utfLength);
                    write(byteArray);
                }

            }

        }
    }
}

