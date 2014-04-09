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
                class SerializationContext;

                class Data;
            }
        }
        namespace connection {
            class HAZELCAST_API InputSocketStream {
            public:

                InputSocketStream(Socket &socket);

                void setSerializationContext(serialization::pimpl::SerializationContext *context);

                int readInt();

                void readData(serialization::pimpl::Data &data);

            private:
                Socket &socket;
                serialization::pimpl::SerializationContext *context;

                void readFully(std::vector<byte> &);

                int skipBytes(int i);

            };

        }
    }
}


#endif //HAZELCAST_INPUT_SOCKET_STREAM

