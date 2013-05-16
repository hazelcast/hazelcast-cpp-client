//
//  BufferedDataInput.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "BufferedDataInput.h"
#include "HazelcastException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            BufferedDataInput::BufferedDataInput(const std::vector<byte>& rhsBuffer) {
                int size = rhsBuffer.size();
                try {
                    beg = new byte[size];
                    ptr = beg;
                    for (int i = 0; i < size; i++)
                        beg[i] = rhsBuffer[i];
                } catch (std::exception& e) {
                    delete [] beg;
                };
            };

            BufferedDataInput& BufferedDataInput::operator [](std::string string) {
                throw hazelcast::client::HazelcastException("BufferedDataInput::operator [](std::string string) > not supported!!");
            };

            BufferedDataInput::~BufferedDataInput() {
                delete [] beg;
            };

            BufferedDataInput& BufferedDataInput::operator = (const BufferedDataInput&) {
                return *this;
            };

            void BufferedDataInput::readFully(std::vector<byte>& bytes) {
                byte temp[bytes.size()];
                readFully(temp, 0, bytes.size());
                bytes = std::vector<byte >(temp, temp + bytes.size());
            };

            void BufferedDataInput::readFully(byte *bytes, int off, int len) {
                memcpy(bytes + off, ptr, sizeof (byte) * len);
                ptr += sizeof (byte) * len;
            };

            int BufferedDataInput::skipBytes(int i) {
                ptr += i;
                return i;
            };

            bool BufferedDataInput::readBoolean() {
                return readByte();
            };

            byte BufferedDataInput::readByte() {
                byte b;
                memcpy(&b, ptr, sizeof (byte));
                ptr += sizeof (byte);
                return b;
            };

            short BufferedDataInput::readShort() {
                byte a = readByte();
                byte b = readByte();
                return (0xff00 & (a << 8)) |
                        (0x00ff & b);
            };

            char BufferedDataInput::readChar() {
                readByte();
                byte b = readByte();
                return b;
            };

            int BufferedDataInput::readInt() {
                byte a = readByte();
                byte b = readByte();
                byte c = readByte();
                byte d = readByte();
                return (0xff000000 & (a << 24)) |
                        (0x00ff0000 & (b << 16)) |
                        (0x0000ff00 & (c << 8)) |
                        (0x000000ff & d);
            };

            long BufferedDataInput::readLong() {
                byte a = readByte();
                byte b = readByte();
                byte c = readByte();
                byte d = readByte();
                byte e = readByte();
                byte f = readByte();
                byte g = readByte();
                byte h = readByte();
                return (0xff00000000000000 & (long(a) << 56)) |
                        (0x00ff000000000000 & (long(b) << 48)) |
                        (0x0000ff0000000000 & (long(c) << 40)) |
                        (0x000000ff00000000 & (long(d) << 32)) |
                        (0x00000000ff000000 & (e << 24)) |
                        (0x0000000000ff0000 & (f << 16)) |
                        (0x000000000000ff00 & (g << 8)) |
                        (0x00000000000000ff & h);
            };

            float BufferedDataInput::readFloat() {

                union {
                    int i;
                    float f;
                } u;
                u.i = readInt();
                return u.f;
            };

            double BufferedDataInput::readDouble() {

                union {
                    double d;
                    long l;
                } u;
                u.l = readLong();
                return u.d;
            };

            std::string BufferedDataInput::readUTF() {
                bool isNull = readBoolean();
                if (isNull)
                    return "";
                int length = readInt();
                std::string result;
                int chunkSize = (length / STRING_CHUNK_SIZE) + 1;
                while (chunkSize > 0) {
                    result += readShortUTF();
                    chunkSize--;
                }
                return result;
            };

            int BufferedDataInput::position() {
                return int(ptr - beg);
            };

            void BufferedDataInput::position(int newPos) {
                ptr = beg + newPos;
            };
            //private functions

            std::string BufferedDataInput::readShortUTF() {
                short utflen = readShort();
                byte bytearr[utflen];
                char chararr[utflen];
                int c, char2, char3;
                int count = 0;
                int chararr_count = 0;
                readFully(bytearr, 0, utflen);

                while (count < utflen) {
                    c = bytearr[count] & 0xff;
                    switch (0x0f & (c >> 4)) {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                            /* 0xxxxxxx */
                            count++;
                            chararr[chararr_count++] = (char) c;
                            break;
                        case 12:
                        case 13:
                            /* 110x xxxx 10xx xxxx */
                            count += 2;
                            if (count > utflen)
                                throw hazelcast::client::HazelcastException("BufferedDataInput::readShortUTF : malformed input: partial character at end");
                            char2 = bytearr[count - 1];
                            if ((char2 & 0xC0) != 0x80) {
                                std::string error = "malformed input around byte";
                                error += count;
                                throw hazelcast::client::HazelcastException(error);
                            }
                            chararr[chararr_count++] = (char) (((c & 0x1F) << 6) | (char2 & 0x3F));
                            break;
                        case 14:
                            /* 1110 xxxx 10xx xxxx 10xx xxxx */
                            count += 3;
                            if (count > utflen)
                                throw hazelcast::client::HazelcastException("BufferedDataInput::readShortUTF : malformed input: partial character at end");
                            char2 = bytearr[count - 2];
                            char3 = bytearr[count - 1];
                            if (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80)) {
                                std::string error = "BufferedDataInput::readShortUTF : malformed input around byte";
                                error += count - 1;
                                throw hazelcast::client::HazelcastException(error);
                            }
                            chararr[chararr_count++] = (char) (((c & 0x0F) << 12) | ((char2 & 0x3F) << 6) | ((char3 & 0x3F) << 0));
                            break;
                        default:
                            /* 10xx xxxx, 1111 xxxx */

                            std::string error = "malformed input around byte";
                            error += count;
                            throw hazelcast::client::HazelcastException(error);

                    }
                }
                chararr[chararr_count] = '\0';
                return std::string(chararr);
            };

            std::vector <byte> BufferedDataInput::readByteArray() {
                int len = readInt();
                std::vector <byte> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readByte();
                }
                return values;
            };

            std::vector<char> BufferedDataInput::readCharArray() {

                int len = readInt();
                std::vector<char> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readChar();
                }
                return values;
            };

            std::vector<int> BufferedDataInput::readIntArray() {

                int len = readInt();
                std::vector<int> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readInt();
                }
                return values;
            };

            std::vector<long> BufferedDataInput::readLongArray() {

                int len = readInt();
                std::vector<long> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readLong();
                }
                return values;
            };

            std::vector<double> BufferedDataInput::readDoubleArray() {

                int len = readInt();
                std::vector<double> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readDouble();
                }
                return values;
            };

            std::vector<float> BufferedDataInput::readFloatArray() {

                int len = readInt();
                std::vector<float> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readFloat();
                }
                return values;
            };

            std::vector<short> BufferedDataInput::readShortArray() {

                int len = readInt();
                std::vector<short> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readShort();
                }
                return values;
            };

        }
    }
}
