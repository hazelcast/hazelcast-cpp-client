//
// Created by sancar koyunlu on 5/10/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_INPUT_SOCKET_STREAM
#define HAZELCAST_INPUT_SOCKET_STREAM

#include "hazelcast/util/HazelcastDll.h"
#include <vector>
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

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
            class HAZELCAST_API InputSocketStream {
            public:

                InputSocketStream(Socket &socket);

                void setPortableContext(serialization::pimpl::PortableContext *context);

                int readInt();

                void readPacket(serialization::pimpl::Packet &data);

            private:

                Socket &socket;
                serialization::pimpl::PortableContext *context;

                void readValue(serialization::pimpl::Data &data);

                void readFully(std::vector<byte> &);

                int readShort();

                int readByte();

                int skipBytes(int i);

                bool readBoolean();
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_INPUT_SOCKET_STREAM

