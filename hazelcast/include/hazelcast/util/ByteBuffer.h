//
// Created by sancar koyunlu on 31/12/13.
//


#ifndef HAZELCAST_ByteBuffer
#define HAZELCAST_ByteBuffer

#include "hazelcast/util/HazelcastDll.h"
#include <vector>
#include <cstdlib>
#include <boost/shared_ptr.hpp>

namespace hazelcast {
    namespace client {
        class Socket;
    }
    namespace util {
        typedef std::vector<byte> ByteVector;
        typedef boost::shared_ptr<ByteVector> ByteVector_ptr;

        typedef std::vector<char> CharVector;
        typedef boost::shared_ptr<CharVector> CharVector_ptr;

        class HAZELCAST_API ByteBuffer {
        public:
            ByteBuffer(char* buffer, size_t size);

            ByteBuffer &flip();

            ByteBuffer &compact();

            ByteBuffer &clear();

            size_t remaining() const;

            bool hasRemaining() const;

            size_t position() const;

            void position(size_t );

            size_t readFrom(const client::Socket &socket, int flag = 0);

            void writeTo(const client::Socket &socket);

            int readInt();

            int readShort();

            void writeInt(int v);

            void writeShort(int v);

            size_t writeTo(std::vector<byte> &destination, size_t offset);

            void writeTo(std::vector<byte> &destination);

            void readFrom(std::vector<byte> const &source);

            size_t readFrom(std::vector<byte> const &source, size_t offset);

            /**
             * This method is NOT SAFE!!!
             * Relative bulk <i>put</i> method&nbsp;&nbsp;<i>(optional operation)</i>.
             *
             * <p> This method transfers bytes into this buffer from the given
             * source array.
             *
             * @param  src
             *         The array from which bytes are to be read
             *
             * @param  offset
             *         The offset within the array of the first byte to be read;
             *         must be non-negative and no larger than <tt>array.length</tt>
             *
             * @param  length
             *         The number of bytes to be read from the given array;
             *         must be non-negative and no larger than
             *         <tt>array.length - offset</tt>
             *
             * @return  This buffer
             */
            ByteBuffer &put(const byte *src, int offset, int length);

            ByteBuffer &get(std::vector<byte> &dstBytesVector, int offset, int length);

            void skip(int l);

            char readByte();

            size_t limit() const;

            void writeByte(char c);
        private:

            void *ix() const;

            size_t minimum(size_t a, size_t b) const;

            void safeIncrementPosition(size_t );

            size_t pos;
            size_t lim;
            size_t capacity;
            char *buffer;
        };
    }
}

#endif //HAZELCAST_ByteBuffer

