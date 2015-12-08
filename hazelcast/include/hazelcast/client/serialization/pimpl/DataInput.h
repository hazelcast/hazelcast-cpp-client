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




#ifndef HAZELCAST_DataInput
#define HAZELCAST_DataInput

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"
#include <vector>
#include <string>
#include <memory>
#include <boost/smart_ptr/shared_ptr.hpp>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util{
        class ByteBuffer;
    }
    namespace client {
        namespace serialization {
            namespace pimpl {
                class HAZELCAST_API DataInput {
                public:
                    DataInput(const std::vector<byte> &buffer);

                    DataInput(const std::vector<byte> &buffer, int offset);

                    void readFully(std::vector<byte> &);

                    void readFully(std::vector<char> &);

                    int skipBytes(int i);

                    bool readBoolean();

                    byte readByte();

                    short readShort();

                    // TODO: change to return 2 bytes char as in java
                    char readChar();

                    int readInt();

                    long long readLong();

                    float readFloat();

                    double readDouble();

                    std::auto_ptr<std::string> readUTF();

                    std::auto_ptr<std::vector<byte> > readByteArray();

                    std::auto_ptr<std::vector<bool> > readBooleanArray();

                    std::auto_ptr<std::vector<char> > readCharArray();

                    std::auto_ptr<std::vector<int> > readIntArray();

                    std::auto_ptr<std::vector<long> > readLongArray();

                    std::auto_ptr<std::vector<double> > readDoubleArray();

                    std::auto_ptr<std::vector<float> > readFloatArray();

                    std::auto_ptr<std::vector<short> > readShortArray();

                    std::auto_ptr<std::vector<std::string> > readUTFArray();

                    int position();

                    void position(int position);

                private:
                    const std::vector<byte> &buffer;

                    int pos;

                    template <typename T>
                    inline T read() {
                        throw exception::HazelcastSerializationException("DataInput::read", "Unsupported type");
                    }

                    template <typename T>
                    inline int getSize(T *dummy) {
                        throw exception::HazelcastSerializationException("DataInput::getSize", "Unsupported type");
                    }

                    template <typename T>
                    inline std::auto_ptr<std::vector<T> > readArray() {
                        int len = readInt();
                        if (util::Bits::NULL_ARRAY == len) {
                            return std::auto_ptr<std::vector<T> > (NULL);
                        }

                        if (len > 0) {
                            checkAvailable((size_t)len * getSize((T *)NULL));
                            std::auto_ptr<std::vector<T> > values(new std::vector<T>((size_t)len));
                            for (int i = 0; i < len; i++) {
                                (*values)[i] = read<T>();
                            }
                            return values;
                        }

                        return std::auto_ptr<std::vector<T> > (new std::vector<T>(0));
                    }

                    int getNumBytesForUtf8Char(const byte *start) const;

                    DataInput(const DataInput &);

                    DataInput &operator = (const DataInput &);

                    void checkAvailable(size_t requestedLength);

                    int getSize(byte *dummy);

                    int getSize(char *dummy);

                    int getSize(bool *dummy);

                    int getSize(short *dummy);

                    int getSize(int *dummy);

                    int getSize(long *dummy);

                    int getSize(float *dummy);

                    int getSize(double *dummy);
                };

                template <>
                HAZELCAST_API byte DataInput::read();

                template <>
                HAZELCAST_API char DataInput::read();

                template <>
                HAZELCAST_API bool DataInput::read();

                template <>
                HAZELCAST_API short DataInput::read();

                template <>
                HAZELCAST_API int DataInput::read();

                template <>
                HAZELCAST_API long DataInput::read();

                template <>
                HAZELCAST_API float DataInput::read();

                template <>
                HAZELCAST_API double DataInput::read();
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_DataInput

