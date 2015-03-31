//
// Created by sancar koyunlu on 8/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include <string.h>
#include <stdio.h>
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/util/IOUtil.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                int const DataInput::STRING_CHUNK_SIZE = 16 * 1024;

                DataInput::DataInput(const std::vector<byte> &buf)
                :buffer(buf)
                , pos(0) {
                }

                DataInput::DataInput(const std::vector<byte> &buf, int offset)
                        : buffer(buf), pos(offset) {
                }

                void DataInput::readFully(std::vector<byte> &bytes) {
                    bytes = std::vector<byte >(buffer.begin() + pos, buffer.begin() + pos + bytes.size());
                    pos += bytes.size();
                }

                bool DataInput::readBoolean() {
                    return readByte() != 0;
                }

                byte DataInput::readByte() {
                    return buffer[pos++];
                }

                short DataInput::readShort() {
                    byte a = readByte();
                    byte b = readByte();
                    return (0xff00 & (a << 8)) |
                            (0x00ff & b);
                }

                char DataInput::readChar() {
                    readByte();
                    byte b = readByte();
                    return b;
                }

                int DataInput::readInt() {
                    byte a = readByte();
                    byte b = readByte();
                    byte c = readByte();
                    byte d = readByte();
                    return (0xff000000 & (a << 24)) |
                            (0x00ff0000 & (b << 16)) |
                            (0x0000ff00 & (c << 8)) |
                            (0x000000ff & d);
                }

                long long DataInput::readLong() {
                    byte a = readByte();
                    byte b = readByte();
                    byte c = readByte();
                    byte d = readByte();
                    byte e = readByte();
                    byte f = readByte();
                    byte g = readByte();
                    byte h = readByte();
                    return (0xff00000000000000LL & ((long long) (a) << 56)) |
                            (0x00ff000000000000LL & ((long long) (b) << 48)) |
                            (0x0000ff0000000000LL & ((long long) (c) << 40)) |
                            (0x000000ff00000000LL & ((long long) (d) << 32)) |
                            (0x00000000ff000000LL & (e << 24)) |
                            (0x0000000000ff0000LL & (f << 16)) |
                            (0x000000000000ff00LL & (g << 8)) |
                            (0x00000000000000ffLL & h);
                }

                float DataInput::readFloat() {
                    union {
                        int i;
                        float f;
                    } u;
                    u.i = readInt();
                    return u.f;
                }

                double DataInput::readDouble() {
                    union {
                        double d;
                        long long l;
                    } u;
                    u.l = readLong();
                    return u.d;
                }

                std::string DataInput::readUTF() {
                    bool isNull = readBoolean();
                    if (isNull)
                        return "";
                    int length = readInt();
                    int lengthCheck = readInt();
                    if (length != lengthCheck) {
                        throw exception::IOException("DataInput::readShortUTF", "Length check failed, maybe broken bytestream or wrong stream position");
                    }
                    std::string result = "";
                    int chunkSize = (length / STRING_CHUNK_SIZE) + 1;
                    while (chunkSize > 0) {
                        result += readShortUTF();
                        chunkSize--;
                    }
                    return result;
                }

                int DataInput::position() {
                    return pos;
                }

                void DataInput::position(int newPos) {
                    pos = newPos;
                }
                //private functions

                std::string DataInput::readShortUTF() {
                    short utflen = readShort();
                    std::vector<byte> bytearr(utflen);
                    std::vector<char> chararr(utflen + 1);
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
                                    throw exception::IOException("DataInput::readShortUTF", "malformed input: partial character at end");
                                char2 = bytearr[count - 1];
                                if ((char2 & 0xC0) != 0x80) {
                                    throw exception::IOException("DataInput::readShortUTF", "malformed input around byte" + util::IOUtil::to_string(count));
                                }
                                chararr[chararr_count++] = (char) (((c & 0x1F) << 6) | (char2 & 0x3F));
                                break;
                            case 14:
                                /* 1110 xxxx 10xx xxxx 10xx xxxx */
                                count += 3;
                                if (count > utflen)
                                    throw exception::IOException("DataInput::readShortUTF", "malformed input: partial character at end");
                                char2 = bytearr[count - 2];
                                char3 = bytearr[count - 1];
                                if (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80)) {
                                    throw exception::IOException("DataInput::readShortUTF", "malformed input around byte" + util::IOUtil::to_string(count - 1));
                                }
                                chararr[chararr_count++] = (char) (((c & 0x0F) << 12) | ((char2 & 0x3F) << 6) | ((char3 & 0x3F) << 0));
                                break;
                            default:
                                /* 10xx xxxx, 1111 xxxx */

                                throw exception::IOException("DataInput::readShortUTF", "malformed input around byte" + util::IOUtil::to_string(count));

                        }
                    }
                    chararr[chararr_count] = '\0';
                    return std::string(&chararr[0]);
                }

                hazelcast::util::ByteVector_ptr DataInput::readByteArrayAsPtr() {
                    int len = readInt();
                    checkBoundary(len);
                    util::ByteVector_ptr values(
                            new util::ByteVector(buffer.begin() + pos, buffer.begin() + pos + len));
                    pos += len;
                    return values;
                }

                hazelcast::util::ByteVector DataInput::readByteArray() {
                    int len = readInt();
                    checkBoundary(len);
                    util::ByteVector result(buffer.begin() + pos, buffer.begin() + pos + len);
                    pos += len;
                    return result;
                }

                void DataInput::readByteArray(char destinationBuffer[], short length) {
                    checkBoundary(length);
                    memcpy(destinationBuffer, &buffer[0] + pos, length);
                    pos += length;
                }

                hazelcast::util::CharVector_ptr DataInput::readCharArrayAsPtr() {
                    int len = readInt();
                    util::CharVector_ptr values(new util::CharVector(len));
                    for (int i = 0; i < len; i++) {
                        (*values)[i] = readChar();
                    }
                    return values;
                }

                hazelcast::util::CharVector DataInput::readCharArray() {
                    int len = readInt();
                    util::CharVector values(len);
                    for (int i = 0; i < len; i++) {
                        values[i] = readChar();
                    }
                    return values;
                }

                std::vector<int> DataInput::readIntArray() {
                    int len = readInt();
                    std::vector<int> values(len);
                    for (int i = 0; i < len; i++) {
                        values[i] = readInt();
                    }
                    return values;
                }

                std::vector<long> DataInput::readLongArray() {
                    int len = readInt();
                    std::vector<long> values(len);
                    for (int i = 0; i < len; i++) {
                        values[i] = (long)readLong();
                    }
                    return values;
                }

                std::vector<double> DataInput::readDoubleArray() {
                    int len = readInt();
                    std::vector<double> values(len);
                    for (int i = 0; i < len; i++) {
                        values[i] = readDouble();
                    }
                    return values;
                }

                std::vector<float> DataInput::readFloatArray() {
                    int len = readInt();
                    std::vector<float> values(len);
                    for (int i = 0; i < len; i++) {
                        values[i] = readFloat();
                    }
                    return values;
                }

                std::vector<short> DataInput::readShortArray() {
                    int len = readInt();
                    std::vector<short> values(len);
                    for (int i = 0; i < len; i++) {
                        values[i] = readShort();
                    }
                    return values;
                }

                int DataInput::readInt(int newPositionToReadFrom) {
                    position(newPositionToReadFrom);
                    return readInt();
                }

                void DataInput::checkBoundary(short requestedLength) throw (IOException) {
                    int available = buffer.size() - pos;

                    if (requestedLength > available) {
                        char msg[100];
                        snprintf(msg, 100, "Not enough bytes in internal buffer. Available:%d bytes but needed %d bytes", available, requestedLength);
                        throw IOException("DataInput::checkBoundary", msg);
                    }
                }
            }
        }
    }
}
