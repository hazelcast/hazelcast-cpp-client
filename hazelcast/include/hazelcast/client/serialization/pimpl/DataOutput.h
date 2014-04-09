//
// Created by sancar koyunlu on 8/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DataOutput
#define HAZELCAST_DataOutput


#include "hazelcast/util/HazelcastDll.h"
#include <memory>
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class HAZELCAST_API DataOutput {
                public:

                    DataOutput();

                    virtual ~DataOutput();

                    std::auto_ptr< std::vector<byte> > toByteArray();

                    void write(const std::vector<byte> &bytes);

                    void writeBoolean(bool b);

                    void writeByte(byte i);

                    void writeShort(int i);

                    void writeChar(int i);

                    void writeInt(int i);

                    void writeLong(long long l);

                    void writeFloat(float v);

                    void writeDouble(double v);

                    void writeUTF(const std::string &s);

                    void writeByteArray(const std::vector<byte> &data);

                    void writeCharArray(const std::vector<char> &bytes);

                    void writeShortArray(const std::vector<short > &data);

                    void writeIntArray(const std::vector<int> &data);

                    void writeLongArray(const std::vector<long > &data);

                    void writeFloatArray(const std::vector<float > &data);

                    void writeDoubleArray(const std::vector<double > &data);

                    void writeByte(int index, int i);

                    void writeInt(int index, int v);

                    int position();

                    void position(size_t newPos);

                    static int const STRING_CHUNK_SIZE = 16 * 1024;
                    static int const DEFAULT_SIZE = 4 * 1024;

                private:
                    std::auto_ptr< std::vector<byte> > outputStream;

                    void writeShortUTF(const std::string &);

                    DataOutput(const DataOutput &rhs);

                    DataOutput &operator = (const DataOutput &rhs);

                };
            }
        }
    }
}

#endif //HAZELCAST_DataOutput

