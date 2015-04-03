//
// Created by sancar koyunlu on 8/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DataInput
#define HAZELCAST_DataInput

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/ByteBuffer.h"

#include <vector>
#include <string>

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
            /**
             * TODO: Make reads safe by checking on the array boundary, throw HazelcastSerializationException
             */
                class HAZELCAST_API DataInput {
                public:
                    DataInput(const std::vector<byte> &buffer);

                    DataInput(const std::vector<byte> &buffer, int offset);

                    void readFully(std::vector<byte> &);

                    int skipBytes(int i);

                    bool readBoolean();

                    byte readByte();

                    short readShort();

                    char readChar();

                    int readInt();

                    long long readLong();

                    float readFloat();

                    double readDouble();

                    std::string readUTF();

                    std::auto_ptr<std::vector<byte> > readByteArrayAsPtr();

                    std::vector<byte> readByteArray();

                    std::vector<char> readCharArray();

                    std::vector<int> readIntArray();

                    std::vector<long> readLongArray();

                    std::vector<double> readDoubleArray();

                    std::vector<float> readFloatArray();

                    std::vector<short> readShortArray();

                    int position();

                    void position(int newPos);

                    int readInt(int newPositionToReadFrom);

                    void readByteArray(char buffer[], short length);

                private:
                    const std::vector<byte> &buffer;

                    int pos;

                    static int const STRING_CHUNK_SIZE;

                    std::string readShortUTF();

                    DataInput(const DataInput &);

                    DataInput &operator = (const DataInput &);

                    void checkBoundary(size_t requestedLength);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_DataInput

