//
// Created by sancar koyunlu on 31/12/13.
//


#ifndef HAZELCAST_ByteBuffer
#define HAZELCAST_ByteBuffer

#include "hazelcast/util/HazelcastDll.h"
#include <vector>

namespace hazelcast {
    namespace client {
        class Socket;
    }
    namespace util {
        class HAZELCAST_API ByteBuffer {
        public:
            ByteBuffer(int capacity);

            ~ByteBuffer();

            ByteBuffer &flip();

            ByteBuffer &compact();

            ByteBuffer &clear();

            int remaining() const;

            bool hasRemaining() const;

            int position() const;

            void readFrom(const client::Socket &socket);

            void writeTo(const client::Socket &socket);

            int readInt();

            void writeInt(int v);

            int writeTo(std::vector<byte> &destination, int offset);

            void writeTo(std::vector<byte> &destination);

            void readFrom(std::vector<byte> const &source);

            int readFrom(std::vector<byte> const &source, int offset);

            void skip(int l);

        private:
            char readByte();

            void writeByte(char c);

            void *ix() const;

            int minimum(int a, int b) const;

            int pos;
            int limit;
            int capacity;
            char *buffer;
        };
    }
}

#endif //HAZELCAST_ByteBuffer
