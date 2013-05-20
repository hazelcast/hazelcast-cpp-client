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
#include "Socket.h"

namespace hazelcast {
    namespace client {

        namespace serialization {

            class OutputSocketStream {
            public:

                OutputSocketStream(hazelcast::client::protocol::Socket& socket);

                virtual void write(const std::vector<byte>& bytes);

                virtual void write(char const *bytes, int length);

                virtual void writeBoolean(bool b);

                virtual void writeByte(int i);

                virtual void writeShort(int i);

                virtual void writeChar(int i);

                virtual void writeInt(int i);

                virtual void writeLong(long l);

                virtual void writeFloat(float v);

                virtual void writeDouble(double v);

                virtual void writeUTF(std::string s);

                static int const STRING_CHUNK_SIZE = 16 * 1024;

            private:
                hazelcast::client::protocol::Socket& socket;

                void writeShortUTF(std::string);
            };

        }
    }
}
#endif /* HAZELCAST_OUTPUT_SOCKET_STREAM */

