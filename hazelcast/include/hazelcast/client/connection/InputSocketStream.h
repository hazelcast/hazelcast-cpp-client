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
            namespace pimpl {
                class PortableContext;

                class Data;

                class Packet;
            }
        }
        namespace connection {
            class InputSocketStream {
            public:

                InputSocketStream(Socket &socket);

                void setPortableContext(serialization::pimpl::PortableContext *context);

                int readInt();

                void readPacket(serialization::pimpl::Packet &data);

            private:

                Socket &socket;
                serialization::pimpl::PortableContext *context;

                void readData(serialization::pimpl::Data &data);

                void readFully(std::vector<byte> &);

                int readShort();

                int readByte();

                int skipBytes(int i);

            };

        }
    }
}


#endif //HAZELCAST_INPUT_SOCKET_STREAM

