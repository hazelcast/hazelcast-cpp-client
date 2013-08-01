//
//  BufferedDataOutput.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "ObjectDataOutput.h"
#include "IOException.h"
#include "Util.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            ObjectDataOutput::ObjectDataOutput()
            : outputStream(new std::vector<byte>()) {
                outputStream->reserve(DEFAULT_SIZE);
            };


            ObjectDataOutput::~ObjectDataOutput() {

            };

            ObjectDataOutput::ObjectDataOutput(ObjectDataOutput const & rhs) {
                //private
            };

            ObjectDataOutput & ObjectDataOutput::operator = (ObjectDataOutput const & rhs) {
                //private
                return *this;
            };

            std::auto_ptr< std::vector<byte> > ObjectDataOutput::toByteArray() {
                return outputStream;
            };

            void ObjectDataOutput::write(const std::vector<byte>& bytes) {
                outputStream->insert(outputStream->end(), bytes.begin(), bytes.end());
            };

            void ObjectDataOutput::writeBoolean(bool i) {
                writeByte(i);
            };

            void ObjectDataOutput::writeByte(int index, int i) {
                (*outputStream)[index] = char(0xff & i);
            }

            void ObjectDataOutput::writeByte(int i) {
                outputStream->push_back(char(0xff & i));
            };

            void ObjectDataOutput::writeShort(int v) {
                writeByte((v >> 8));
                writeByte(v);
            };

            void ObjectDataOutput::writeChar(int i) {
                writeByte((i >> 8));
                writeByte(i);
            };

            void ObjectDataOutput::writeInt(int v) {
                writeByte((v >> 24));
                writeByte((v >> 16));
                writeByte((v >> 8));
                writeByte(v);
            };

            void ObjectDataOutput::writeLong(long l) {
                writeByte((l >> 56));
                writeByte((l >> 48));
                writeByte((l >> 40));
                writeByte((l >> 32));
                writeByte((l >> 24));
                writeByte((l >> 16));
                writeByte((l >> 8));
                writeByte((int) l);
            };

            void ObjectDataOutput::writeFloat(float x) {
                union {
                    float f;
                    int i;
                } u;
                u.f = x;
                writeInt(u.i);
            };

            void ObjectDataOutput::writeDouble(double v) {
                union {
                    double d;
                    long l;
                } u;
                u.d = v;
                writeLong(u.l);
            };

            void ObjectDataOutput::writeUTF(const std::string& str) {
                bool isNull = str.empty();
                writeBoolean(isNull);
                if (isNull)
                    return;

                int length = (int) str.length();
                writeInt(length);
                int chunkSize = length / STRING_CHUNK_SIZE + 1;
                for (int i = 0; i < chunkSize; i++) {
                    int beginIndex = std::max(0, i * STRING_CHUNK_SIZE - 1);
                    int endIndex = std::max((i + 1) * STRING_CHUNK_SIZE - 1, length);
                    writeShortUTF(str.substr(beginIndex, endIndex - beginIndex));
                }
            };

            void ObjectDataOutput::writeInt(int index, int v) {
                writeByte(index++, (v >> 24));
                writeByte(index++, (v >> 16));
                writeByte(index++, (v >> 8));
                writeByte(index, v);
            };

            void ObjectDataOutput::writeByteArray(const std::vector<byte>&  data) {
                writeInt(data.size());
                outputStream->insert(outputStream->end(), data.begin(), data.end());
            };

            void ObjectDataOutput::writeCharArray(const std::vector<char>& data) {
                writeInt(data.size());
//                outputStream->insert(outputStream->end(), data.begin(), data.end());//TODO
                for (int i = 0; i < data.size(); ++i) {
                    writeChar(data[i]);
                }
            };

            void ObjectDataOutput::writeShortArray(const std::vector<short >&  data) {
                int size = data.size();
                writeInt(size);
                for (int i = 0; i < size; ++i) {
                    writeShort(data[i]);
                }
            };

            void ObjectDataOutput::writeIntArray(const std::vector<int>&  data) {
                int size = data.size();
                writeInt(size);
                for (int i = 0; i < size; ++i) {
                    writeInt(data[i]);
                }
            };

            void ObjectDataOutput::writeLongArray(const std::vector<long >&  data) {
                int size = data.size();
                writeInt(size);
                for (int i = 0; i < size; ++i) {
                    writeLong(data[i]);
                }
            };

            void ObjectDataOutput::writeFloatArray(const std::vector<float >&  data) {
                int size = data.size();
                writeInt(size);
                for (int i = 0; i < size; ++i) {
                    writeFloat(data[i]);
                }
            };

            void ObjectDataOutput::writeDoubleArray(const std::vector<double >&  data) {
                int size = data.size();
                writeInt(size);
                for (int i = 0; i < size; ++i) {
                    writeDouble(data[i]);
                }
            };

            int ObjectDataOutput::position() {
                return outputStream->size();
            };

            void ObjectDataOutput::position(int newPos) {
                if (outputStream->size() < newPos)
                    outputStream->resize(newPos, 0);
            };

            void ObjectDataOutput::reset() {
                outputStream->clear();
            };

            //private functions

            void ObjectDataOutput::writeShortUTF(const std::string& str) {
                int stringLen = (int) str.length();
                int utfLength = 0;
                int count = 0;
                /* use charAt instead of copying String to char std::vector */
                for (int i = 0; i < stringLen; i++) {
                    if ((str[i] >= 0x0001) && (str[i] <= 0x007F)) {
                        utfLength++;
                    } else if (str[i] > 0x07FF) {
                        utfLength += 3;
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
                    } else if (str[i] > 0x07FF) {
                        byteArray[count++] = (byte) (0xE0 | ((str[i] >> 12) & 0x0F));
                        byteArray[count++] = (byte) (0x80 | ((str[i] >> 6) & 0x3F));
                        byteArray[count++] = (byte) (0x80 | ((str[i]) & 0x3F));
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
