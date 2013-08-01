//
//  ObjectDataInput.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "ObjectDataInput.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            ObjectDataInput::ObjectDataInput(const std::vector<byte>& rhsBuffer )
            :buffer(rhsBuffer)
            , pos(0)
            , serializationContext(NULL)
            , serializerHolder(NULL) {
            };

            ObjectDataInput::ObjectDataInput(const std::vector<byte>& rhsBuffer, SerializerHolder& serializerHolder, SerializationContext& context)
            :buffer(rhsBuffer)
            , pos(0)
            , serializationContext(&context)
            , serializerHolder(&serializerHolder){
            };

            ObjectDataInput::ObjectDataInput(ObjectDataInput const & param)
            :buffer(param.buffer){
                //private
            };

            ObjectDataInput& ObjectDataInput::operator = (const ObjectDataInput&) {
                //private
                return *this;
            };

            void ObjectDataInput::setSerializationContext(SerializationContext *context) {
                this->serializationContext = context;
            };

            SerializationContext *ObjectDataInput::getSerializationContext() {
                return serializationContext;
            };

            void ObjectDataInput::readFully(std::vector<byte>& bytes) {
                bytes = std::vector<byte >(buffer.begin() + pos, buffer.begin() + pos + bytes.size());
                pos += bytes.size();
            };

            int ObjectDataInput::skipBytes(int i) {
                pos += i;
                return i;
            };

            bool ObjectDataInput::readBoolean() {
                return readByte();
            };

            byte ObjectDataInput::readByte() {
                return buffer[pos++];
            };

            short ObjectDataInput::readShort() {
                byte a = readByte();
                byte b = readByte();
                return (0xff00 & (a << 8)) |
                        (0x00ff & b);
            };

            char ObjectDataInput::readChar() {
                readByte();
                byte b = readByte();
                return b;
            };

            int ObjectDataInput::readInt() {
                byte a = readByte();
                byte b = readByte();
                byte c = readByte();
                byte d = readByte();
                return (0xff000000 & (a << 24)) |
                        (0x00ff0000 & (b << 16)) |
                        (0x0000ff00 & (c << 8)) |
                        (0x000000ff & d);
            };

            long ObjectDataInput::readLong() {
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

            float ObjectDataInput::readFloat() {
                union {
                    int i;
                    float f;
                } u;
                u.i = readInt();
                return u.f;
            };

            double ObjectDataInput::readDouble() {
                union {
                    double d;
                    long l;
                } u;
                u.l = readLong();
                return u.d;
            };

            std::string ObjectDataInput::readUTF() {
                bool isNull = readBoolean();
                if (isNull)
                    return "";
                int length = readInt();
                std::string result = "";
                int chunkSize = (length / STRING_CHUNK_SIZE) + 1;
                while (chunkSize > 0) {
                    result += readShortUTF();
                    chunkSize--;
                }
                return result;
            };

            int ObjectDataInput::position() {
                return pos;
            };

            void ObjectDataInput::position(int newPos) {
                pos = newPos;
            };
            //private functions

            std::string ObjectDataInput::readShortUTF() {
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
                                throw exception::IOException("ObjectDataInput::readShortUTF", "malformed input: partial character at end");
                            char2 = bytearr[count - 1];
                            if ((char2 & 0xC0) != 0x80) {
                                throw exception::IOException("ObjectDataInput::readShortUTF", "malformed input around byte" + util::to_string(count));
                            }
                            chararr[chararr_count++] = (char) (((c & 0x1F) << 6) | (char2 & 0x3F));
                            break;
                        case 14:
                            /* 1110 xxxx 10xx xxxx 10xx xxxx */
                            count += 3;
                            if (count > utflen)
                                throw exception::IOException("ObjectDataInput::readShortUTF", "malformed input: partial character at end");
                            char2 = bytearr[count - 2];
                            char3 = bytearr[count - 1];
                            if (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80)) {
                                throw exception::IOException("ObjectDataInput::readShortUTF", "malformed input around byte" + util::to_string(count - 1));
                            }
                            chararr[chararr_count++] = (char) (((c & 0x0F) << 12) | ((char2 & 0x3F) << 6) | ((char3 & 0x3F) << 0));
                            break;
                        default:
                            /* 10xx xxxx, 1111 xxxx */

                            throw exception::IOException("ObjectDataInput::readShortUTF", "malformed input around byte" + util::to_string(count));

                    }
                }
                chararr[chararr_count] = '\0';
                return std::string(chararr.data());
            };

            std::vector <byte> ObjectDataInput::readByteArray() {
                int len = readInt();
                std::vector <byte> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readByte();
                }
                return values;
            };

            std::vector<char> ObjectDataInput::readCharArray() {
                int len = readInt();
                std::vector<char> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readChar();
                }
                return values;
            };

            std::vector<int> ObjectDataInput::readIntArray() {
                int len = readInt();
                std::vector<int> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readInt();
                }
                return values;
            };

            std::vector<long> ObjectDataInput::readLongArray() {
                int len = readInt();
                std::vector<long> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readLong();
                }
                return values;
            };

            std::vector<double> ObjectDataInput::readDoubleArray() {
                int len = readInt();
                std::vector<double> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readDouble();
                }
                return values;
            };

            std::vector<float> ObjectDataInput::readFloatArray() {
                int len = readInt();
                std::vector<float> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = readFloat();
                }
                return values;
            };

            std::vector<short> ObjectDataInput::readShortArray() {
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
