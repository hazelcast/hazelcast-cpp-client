//
// Created by msk on 3/13/13.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_STRING_SOCKET_STREAM
#define HAZELCAST_STRING_SOCKET_STREAM

#include <iostream>
#include <vector>
#include "OutputStream.h"


namespace hazelcast {
    namespace client {
        namespace serialization {
            class OutputStringStream : public OutputStream {
            public:

                OutputStringStream();

                ~OutputStringStream();

                void put(char c);

                void put(int n, char c);

                void write(char const *buffer, int size);

                std::vector<byte> toByteArray();

                void reset();

                void resize(int size);

                int size();

            private:
                std::vector<byte> outputStream;
            };
        }
    }
}
#endif //HAZELCAST_STRING_SOCKET_STREAM
