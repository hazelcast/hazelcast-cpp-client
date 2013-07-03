//
// Created by sancar koyunlu on 5/10/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_INPUT_SOCKET_STREAM
#define HAZELCAST_INPUT_SOCKET_STREAM

#include "../connection/Socket.h"
#include <vector>

namespace hazelcast {
    namespace client {

        namespace serialization {

            class SerializationService;

            class SerializationContext;

            typedef unsigned char byte;

            class InputSocketStream {
            public:

                InputSocketStream(hazelcast::client::connection::Socket& socket);

                void setSerializationContext(SerializationContext* context);

                SerializationContext * getSerializationContext();

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

                void close();

            private:
                connection::Socket& socket;
                SerializationContext* context;

                static int const STRING_CHUNK_SIZE = 16 * 1024;

                std::string readShortUTF();

                InputSocketStream& operator = (const InputSocketStream&);

            };

        }
    }
}


#endif //HAZELCAST_INPUT_SOCKET_STREAM
