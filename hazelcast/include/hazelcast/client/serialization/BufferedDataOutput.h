//
//  BufferedDataOutput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA_OUTPUT
#define HAZELCAST_DATA_OUTPUT

#include "../HazelcastException.h"
#include <vector>
#include <iosfwd>

namespace hazelcast {
    namespace client {
        namespace serialization {

            typedef unsigned char byte;

            class BufferedDataOutput {
            public:

                BufferedDataOutput();

                virtual ~BufferedDataOutput();

                std::auto_ptr< std::vector<byte> > toByteArray();

                void write(const std::vector<byte>& bytes);

                void writeBoolean(bool b);

                void writeByte(int i);

                void writeShort(int i);

                void writeChar(int i);

                void writeInt(int i);

                void writeLong(long l);

                void writeFloat(float v);

                void writeDouble(double v);

                void writeUTF(const std::string& s);

                void writeByteArray(const std::vector<byte>&  data);

                void writeCharArray(const std::vector<char>& bytes);

                void writeShortArray(const std::vector<short >&  data);

                void writeIntArray(const std::vector<int>&  data);

                void writeLongArray(const std::vector<long >&  data);

                void writeFloatArray(const std::vector<float >&  data);

                void writeDoubleArray(const std::vector<double >&  data);

                void writeByte(int index, int i);

                void writeInt(int index, int v);

                int position();

                void position(int newPos);

                void reset();

                static int const STRING_CHUNK_SIZE = 16 * 1024;
                static int const DEFAULT_SIZE = 4 * 1024;

            private:
                std::auto_ptr< std::vector<byte> > outputStream;

                void writeShortUTF(const std::string&);

                BufferedDataOutput(const BufferedDataOutput& rhs);

                BufferedDataOutput& operator = (const BufferedDataOutput& rhs);

            };
        }
    }
}
#endif /* HAZELCAST_DATA_OUTPUT */
