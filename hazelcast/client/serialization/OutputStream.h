//
//  Portable.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


#ifndef HAZELCAST_OUTPUT_STREAM
#define HAZELCAST_OUTPUT_STREAM

#include <string>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace serialization {
            typedef unsigned char byte;

            class OutputStream {
            public:

                virtual void put(char c) = 0;

                virtual void put(int n, char c) = 0;

                virtual void write(char const *buffer, int size) = 0;

                virtual std::vector<byte> toByteArray() = 0;

                virtual void reset() = 0;

                virtual void resize(int size) = 0;

                virtual int size() = 0;

                virtual ~OutputStream() {
                };

            };

        }
    }
}
#endif /* HAZELCAST_OUTPUT_STREAM */
