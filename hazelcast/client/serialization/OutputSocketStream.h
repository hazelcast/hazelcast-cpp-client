//
//  Portable.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


#ifndef HAZELCAST_OUTPUT_SOCKET_STREAM
#define HAZELCAST_OUTPUT_SOCKET_STREAM

#include "OutputStream.h"
#include "Socket.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace serialization {


            class OutputSocketStream : public OutputStream {
            public:

                OutputSocketStream(hazelcast::client::protocol::Socket& socket);

                ~OutputSocketStream();

                void put(char c);

                void put(int n, char c);

                void write(char const *buffer, int size);

                std::vector<byte> toByteArray();

                void reset();

                void resize(int size);

                int size();

            private:
                hazelcast::client::protocol::Socket& socket;
            };

        }
    }
}
#endif /* HAZELCAST_OUTPUT_SOCKET_STREAM */

