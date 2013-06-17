//
//  Portable.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


#ifndef HAZELCAST_OUTPUT_SOCKET_STREAM
#define HAZELCAST_OUTPUT_SOCKET_STREAM

#include <string>
#include <vector>
#include "../connection/Socket.h"

namespace hazelcast {
    namespace client {

        namespace serialization {

            class OutputSocketStream {
            public:

                OutputSocketStream(hazelcast::client::connection::Socket& socket);

                void write(const std::vector<byte>& bytes);

                void write(char const *bytes, int length);

                void write(unsigned char const *bytes, int length);

                void writeBoolean(bool b);

                void writeByte(int i);

                void writeShort(int i);

                void writeChar(int i);

                void writeInt(int i);

                void writeLong(long l);

                void writeFloat(float v);

                void writeDouble(double v);

                void writeUTF(std::string s);

                static int const STRING_CHUNK_SIZE = 16 * 1024;

                void close();

            private:
                hazelcast::client::connection::Socket& socket;

                void writeShortUTF(std::string);
            };

        }
    }
}
#endif /* HAZELCAST_OUTPUT_SOCKET_STREAM */

