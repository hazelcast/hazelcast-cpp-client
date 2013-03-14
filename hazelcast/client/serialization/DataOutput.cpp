//
//  DataOutput.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "DataOutput.h"
#include "SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            DataOutput::DataOutput(SerializationService *service, OutputStream *outputStream)
            : offset(0), outputStream(outputStream) {
                this->service = service;
            };

            std::vector<byte> DataOutput::toByteArray() {
                return outputStream->toByteArray();
            };

            std::string DataOutput::toString() {
                std::vector<byte> bytes = outputStream->toByteArray();
                std::string s;
                for (int i = 0; i < bytes.size(); i++) {
                    s.push_back(bytes[i]);
                }
                return s;
            };

            int DataOutput::getSize() {
                return outputStream->size();
            };

            //Inherited from DataOutput

            void DataOutput::write(const std::vector<byte>& bytes) {
                for (int i = 0; i < bytes.size(); i++)
                    outputStream->put(bytes[i]);
            };

            void DataOutput::write(char const *bytes, int length) {
                outputStream->write(bytes, sizeof (char) * length);
            };

            void DataOutput::writeBoolean(bool i) {
                writeByte(i);
            };

            void DataOutput::writeByte(int n, int i) {
                outputStream->put(n, 0xff & i);
            }

            void DataOutput::writeByte(int i) {
                outputStream->put(0xff & i);
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

            void DataOutput::writeLong(long l) {
                writeByte((l >> 56));
                writeByte((l >> 48));
                writeByte((l >> 40));
                writeByte((l >> 32));
                writeByte((l >> 24));
                writeByte((l >> 16));
                writeByte((l >> 8));
                writeByte((int) l);
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
                    long l;
                } u;
                u.d = v;
                writeLong(u.l);
            };

            void DataOutput::writeUTF(std::string str) {
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

            //Inherited from BufferObjectDataOutput

//            void DataOutput::write(int index, int b) {
//                int pos = position();
//                position(index);
//                writeByte(b);
//                position(pos);
//            };
//
//            void DataOutput::write(int index, char *b, int off, int len) {
//                int pos = position();
//                position(index);
//                write(b, off, len);
//                position(pos);
//            };

            void DataOutput::writeInt(int index, int v) {
//                int pos = position();
//                position(index);
//                writeInt(v);
//                position(pos);
                writeByte(index++, (v >> 24));
                writeByte(index++, (v >> 16));
                writeByte(index++, (v >> 8));
                writeByte(index, v);
            };

//            void DataOutput::writeLong(int index, const long v) {
//                int pos = position();
//                position(index);
//                writeLong(v);
//                position(pos);
//            };

//            void DataOutput::writeBoolean(int index, const bool v) {
//                int pos = position();
//                position(index);
//                writeBoolean(v);
//                position(pos);
//            };

//            void DataOutput::writeByte(int index, const int v) {
//                int pos = position();
//                position(index);
//                writeByte(v);
//                position(pos);
//            };
//
//            void DataOutput::writeChar(int index, const int v) {
//                int pos = position();
//                position(index);
//                writeChar(v);
//                position(pos);
//            };
//
//            void DataOutput::writeDouble(int index, const double v) {
//                int pos = position();
//                position(index);
//                writeDouble(v);
//                position(pos);
//            };
//
//            void DataOutput::writeFloat(int index, const float v) {
//                int pos = position();
//                position(index);
//                writeFloat(v);
//                position(pos);
//            };
//
//            void DataOutput::writeShort(int index, const int v) {
//                int pos = position();
//                position(index);
//                writeShort(v);
//                position(pos);
//            };

            int DataOutput::position() {
                return outputStream->size();
            };

            void DataOutput::position(int newPos) {
                outputStream->resize(newPos);
            };

            void DataOutput::reset() {
                outputStream->reset();
            };

            //private functions

            void DataOutput::writeShortUTF(std::string str) {
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
                    std::string error = "encoded string too long:";
                    error += utfLength;
                    error += " bytes";
                    throw hazelcast::client::HazelcastException(error);
                }
                char byteArray[utfLength];
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
                write(byteArray, utfLength);
            };

        }
    }
}
