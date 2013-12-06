//
// Created by sancar koyunlu on 8/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DataInput
#define HAZELCAST_DataInput

#include "hazelcast/util/HazelcastDll.h"
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {

            typedef unsigned char byte;

            class HAZELCAST_API DataInput {
                friend class ObjectDataInput;

            public:
                DataInput(const std::vector<byte>& rhsBuffer);

                void readFully(std::vector<byte>&);

                int skipBytes(int i);

                bool readBoolean();

                byte readByte();

                short readShort();

                char readChar();

                int readInt();

                long readLong();

                float readFloat();

                double readDouble();

                std::string readUTF();

                std::vector<byte> readByteArray();

                std::vector<char> readCharArray();

                std::vector<int> readIntArray();

                std::vector<long> readLongArray();

                std::vector<double> readDoubleArray();

                std::vector<float> readFloatArray();

                std::vector<short> readShortArray();

                int position();

                void position(int newPos);

            private:
                const std::vector<byte>& buffer;
                int pos;

                static int const STRING_CHUNK_SIZE = 16 * 1024;

                std::string readShortUTF();

                DataInput(const DataInput&);

                DataInput& operator = (const DataInput&);

            };
        }
    }
}

#endif //HAZELCAST_DataInput
