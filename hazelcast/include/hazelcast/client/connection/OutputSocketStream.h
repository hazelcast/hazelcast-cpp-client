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
                class Data;
            }
        }

        namespace connection {

            class HAZELCAST_API OutputSocketStream {
            public:

                OutputSocketStream(Socket &socket);

                void write(const std::vector<byte> &bytes);

                void writeInt(int i);

                void writeData(const serialization::pimpl::Data &);

            private:

                Socket &socket;

                void writeByte(int i);

            };

        }
    }
}
#endif /* HAZELCAST_OUTPUT_SOCKET_STREAM */

