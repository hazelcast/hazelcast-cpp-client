//
// Created by sancar koyunlu on 31/12/13.
//

#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace util {
        ByteBuffer::ByteBuffer(int capacity)
        :pos(0),
        limit(capacity),
        capacity(capacity),
        buffer(new char [capacity]) {

        }


        ByteBuffer::~ByteBuffer() {
            delete [] buffer;
        }

        ByteBuffer &ByteBuffer::flip() {
            limit = pos;
            pos = 0;
            return *this;
        }


        ByteBuffer &ByteBuffer::compact() {
            std::memcpy(buffer, ix(), (size_t) remaining());
            pos = remaining();
            limit = capacity;
            return *this;
        }

        ByteBuffer &ByteBuffer::clear() {
            pos = 0;
            limit = capacity;
            return *this;
        }

        int ByteBuffer::remaining() const {
            return limit - pos;
        }

        bool ByteBuffer::hasRemaining() const {
            return pos < limit;
        }

        int ByteBuffer::position() const {
            return pos;
        }

        void ByteBuffer::readFrom(const client::Socket &socket) {
            int rm = remaining();
            int bytesReceived = socket.receive(ix(), rm, 0);
            pos += bytesReceived;
        };

        void ByteBuffer::writeTo(const client::Socket &socket) {
            int rm = remaining();
            int bytesSend = socket.send(ix(), rm);
            pos += bytesSend;
        };

        int ByteBuffer::readInt() {
            char a = readByte();
            char b = readByte();
            char c = readByte();
            char d = readByte();
            return (0xff000000 & (a << 24)) |
                    (0x00ff0000 & (b << 16)) |
                    (0x0000ff00 & (c << 8)) |
                    (0x000000ff & d);
        }

        void ByteBuffer::writeInt(int v) {
            writeByte(char(v >> 24));
            writeByte(char(v >> 16));
            writeByte(char(v >> 8));
            writeByte(char(v));
        }

        int ByteBuffer::writeTo(std::vector<byte> &destination, int offset) {
            int m = minimum(destination.size() - offset, remaining());
            std::memcpy((void *) (&destination[offset]), ix(), m);
            pos += m;
            return m;
        }

        void ByteBuffer::writeTo(std::vector<byte> &destination) {
            int len = destination.size();
            std::memcpy((void *) (&destination[0]), ix(), len);
            pos += len;
        }


        void ByteBuffer::readFrom(std::vector<byte> const &source) {
            int len = source.size();
            std::memcpy(ix(), (void *) (&source[0]), len);
            pos += len;
        }

        int ByteBuffer::readFrom(std::vector<byte> const &source, int offset) {
            int m = minimum(source.size() - offset, remaining());
            std::memcpy(ix(), (void *) (&source[offset]), m);
            pos += m;
            return m;
        }

        //private
        void ByteBuffer::skip(int l) {
            pos += l;
        }

        char ByteBuffer::readByte() {
            return buffer[pos++];
        }

        void ByteBuffer::writeByte(char c) {
            buffer[pos++] = c;
        }

        void *ByteBuffer::ix() const {
            return (void *) (buffer + pos);
        }

        int ByteBuffer::minimum(int a, int b) const {
            return a < b ? a : b;
        }

    }
}

