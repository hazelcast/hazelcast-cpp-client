//
// Created by sancar koyunlu on 5/10/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_INPUT_SOCKET_STREAM
#define HAZELCAST_INPUT_SOCKET_STREAM

#include <vector>
#include "Socket.h"

namespace hazelcast {
    namespace client {

        namespace serialization {

            class SerializationService;

            class SerializationContext;

            typedef unsigned char byte;

            class InputSocketStream {
            public:

                InputSocketStream(hazelcast::client::protocol::Socket& socket);

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

            private:
                hazelcast::client::protocol::Socket& socket;

                static int const STRING_CHUNK_SIZE = 16 * 1024;

                std::string readShortUTF();

                InputSocketStream& operator = (const InputSocketStream&);

            };

        }
    }
}


#endif //HAZELCAST_INPUT_SOCKET_STREAM
