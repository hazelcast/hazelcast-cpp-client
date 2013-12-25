//
// Created by sancar koyunlu on 5/10/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_INPUT_SOCKET_STREAM
#define HAZELCAST_INPUT_SOCKET_STREAM

#include "hazelcast/util/HazelcastDll.h"
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        class Socket;

        namespace serialization {

            class SerializationService;

            class SerializationContext;



            class HAZELCAST_API InputSocketStream {
            public:

                InputSocketStream(Socket& socket);

                void setSerializationContext(SerializationContext *context);

                SerializationContext *getSerializationContext();

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
                Socket& socket;
                SerializationContext *context;

                static int const STRING_CHUNK_SIZE = 16 * 1024;

                std::string readShortUTF();

                InputSocketStream& operator = (const InputSocketStream&);

            };

        }
    }
}


#endif //HAZELCAST_INPUT_SOCKET_STREAM
