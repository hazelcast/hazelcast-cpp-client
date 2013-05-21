//
// Created by sancar koyunlu on 5/10/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "InputSocketStream.h"
#include "../HazelcastException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            InputSocketStream::InputSocketStream(hazelcast::client::connection::Socket& socket):socket(socket) {
            };

            InputSocketStream& InputSocketStream::operator = (const InputSocketStream&) {
                return *this;
            };

            void InputSocketStream::readFully(std::vector<byte>& bytes) {
                byte temp[bytes.size()];
                socket.receive(temp, bytes.size());
                bytes = std::vector<byte >(temp, temp + bytes.size());
            };

            int InputSocketStream::skipBytes(int i) {
                byte temp[i];
                socket.receive(temp, i);
                return i;
            };

            bool InputSocketStream::readBoolean() {
                return readByte();
            };

            byte InputSocketStream::readByte() {
                byte b;
                socket.receive(&b, sizeof(byte));
                return b;
            };

            short InputSocketStream::readShort() {
                byte a = readByte();
                byte b = readByte();
                return (0xff00 & (a << 8)) |
                        (0x00ff & b);
            };

            char InputSocketStream::readChar() {
                readByte();
                byte b = readByte();
                return b;
            };

            int InputSocketStream::readInt() {
                byte a = readByte();
                byte b = readByte();
                byte c = readByte();
                byte d = readByte();
                return (0xff000000 & (a << 24)) |
                        (0x00ff0000 & (b << 16)) |
                        (0x0000ff00 & (c << 8)) |
                        (0x000000ff & d);
            };

            long InputSocketStream::readLong() {
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

            float InputSocketStream::readFloat() {

                union {
                    int i;
                    float f;
                } u;
                u.i = readInt();
                return u.f;
            };

            double InputSocketStream::readDouble() {

                union {
                    double d;
                    long l;
                } u;
                u.l = readLong();
                return u.d;
            };

            std::string InputSocketStream::readUTF() {
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

            std::string InputSocketStream::readShortUTF() {
                short utflen = readShort();
                std::vector<byte> bytearr(utflen);
                char chararr[utflen];
                int c, char2, char3;
                int count = 0;
                int chararr_count = 0;
                readFully(bytearr);

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
                                throw hazelcast::client::HazelcastException("InputSocketStream::readShortUTF : malformed input: partial character at end");
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
                                throw hazelcast::client::HazelcastException("InputSocketStream::readShortUTF : malformed input: partial character at end");
                            char2 = bytearr[count - 2];
                            char3 = bytearr[count - 1];
                            if (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80)) {
                                std::string error = "InputSocketStream::readShortUTF : malformed input around byte";
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

        }
    }
}
