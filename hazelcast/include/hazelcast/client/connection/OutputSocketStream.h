//
//  Portable.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


#ifndef HAZELCAST_OUTPUT_SOCKET_STREAM
#define HAZELCAST_OUTPUT_SOCKET_STREAM

#include "hazelcast/util/HazelcastDll.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {
        class Socket;
        namespace serialization {
            namespace pimpl {
                class Packet;

                class Data;
            }
        }

        namespace connection {

            class HAZELCAST_API OutputSocketStream {
            public:

                OutputSocketStream(Socket& socket);

                void write(const std::vector<byte>& bytes);

                void writeInt(int i);

                void writePacket(const serialization::pimpl::Packet&);

            private:
                void writeData(const serialization::pimpl::Data&);

                Socket& socket;

                void writeShort(int v);

                void writeByte(int i);

            };

        }
    }
}
#endif /* HAZELCAST_OUTPUT_SOCKET_STREAM */


