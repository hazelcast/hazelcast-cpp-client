//
//  BufferedDataOutput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA_OUTPUT
#define HAZELCAST_DATA_OUTPUT

#include <vector>
#include <iosfwd>
#include "HazelcastException.h"
#include "ConstantSerializers.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            typedef unsigned char byte;

            class BufferedDataOutput {
            public:

                BufferedDataOutput();

                BufferedDataOutput& operator [](std::string);

                virtual std::vector<byte> toByteArray();

                virtual void write(const std::vector<byte>& bytes);

                virtual void write(char const *bytes, int length);

                virtual void writeBoolean(bool b);

                virtual void writeByte(int i);

                virtual void writeByte(int n, int i);

                virtual void writeShort(int i);

                virtual void writeChar(int i);

                virtual void writeInt(int i);

                virtual void writeLong(long l);

                virtual void writeFloat(float v);

                virtual void writeDouble(double v);

                virtual void writeUTF(std::string s);

                virtual void writeInt(int index, int v);

                virtual int position();

                virtual void position(int newPos);

                virtual void reset();

                static int const STRING_CHUNK_SIZE = 16 * 1024;

            private:
                std::vector<byte> outputStream;

                void writeShortUTF(std::string);

            };


            template<typename T>
            inline void writePortable(BufferedDataOutput& out, const std::vector<T>& data) {
                //TODO i probably need to add more here
                //........
                throw hazelcast::client::HazelcastException("template<typename T>\n"
                        "            inline void writePortable(BufferedDataOutput& out, std::vector<T>& data) >> Not supported");
            };

            template<typename T>
            inline void operator <<(BufferedDataOutput& dataOutput, const T& data) {
                //TODO some control stuff can be added here : not sure what is needed right now
                //........
                writePortable(dataOutput, data);
            };
        }
    }
}
#endif /* HAZELCAST_DATA_OUTPUT */
