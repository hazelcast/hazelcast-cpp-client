//
// Created by sancar koyunlu on 31/12/13.
//

#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/Socket.h"
#include <cassert>

namespace hazelcast {
    namespace util {

        ByteBuffer::ByteBuffer(char *buffer, size_t capacity)
        : pos(0), lim(capacity), capacity(capacity), buffer(buffer){

        }

        ByteBuffer& ByteBuffer::flip() {
            lim = pos;
            pos = 0;
            return *this;
        }


        ByteBuffer& ByteBuffer::compact() {
            std::memcpy(buffer, ix(), (size_t)remaining());
            pos = remaining();
            lim = capacity;
            return *this;
        }

        ByteBuffer& ByteBuffer::clear() {
            pos = 0;
            lim = capacity;
            return *this;
        }

        size_t ByteBuffer::remaining() const {
            return lim - pos;
        }

        bool ByteBuffer::hasRemaining() const {
            return pos < lim;
        }

        size_t ByteBuffer::position() const {
            return pos;
        }


        void ByteBuffer::position(size_t i) {
            pos = i;
        }

        size_t ByteBuffer::readFrom(const client::Socket& socket, int flag) {
            int rm = remaining();
            size_t bytesReceived = (size_t)socket.receive(ix(), rm, flag);
            safeIncrementPosition(bytesReceived);
            return bytesReceived;
        }

        void ByteBuffer::writeTo(const client::Socket& socket) {
            int rm = remaining();
            int bytesSend = socket.send(ix(), rm);
            safeIncrementPosition(bytesSend);
        }

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


        int ByteBuffer::readShort() {
            byte a = readByte();
            byte b = readByte();
            return (0xff00 & (a << 8)) |
            (0x00ff & b);
        }

        void ByteBuffer::writeShort(int v) {
            writeByte(char(v >> 8));
            writeByte(char(v));
        }

        size_t ByteBuffer::writeTo(std::vector<byte>& destination, size_t offset) {
            size_t m = minimum(destination.size() - offset, remaining());
            std::memcpy((void *)(&destination[offset]), ix(), m);
            safeIncrementPosition(m);
            return m;
        }

        void ByteBuffer::writeTo(std::vector<byte>& destination) {
            size_t len = destination.size();
            std::memcpy((void *)(&destination[0]), ix(), len);
            safeIncrementPosition(len);
        }


        void ByteBuffer::readFrom(std::vector<byte> const& source) {
            size_t len = source.size();
            std::memcpy(ix(), (void *)(&source[0]), len);
            safeIncrementPosition(len);
        }

        size_t ByteBuffer::readFrom(std::vector<byte> const& source, size_t offset) {
            size_t m = minimum(source.size() - offset, remaining());
            std::memcpy(ix(), (void *)(&source[offset]), m);
            safeIncrementPosition(m);
            return m;
        }

        void ByteBuffer::skip(int l) {
            safeIncrementPosition(l);
        }

        char ByteBuffer::readByte() {
            char b = buffer[pos];
            safeIncrementPosition(1);
            return b;
        }

        void ByteBuffer::writeByte(char c) {
            buffer[pos] = c;
            safeIncrementPosition(1);
        }

        void *ByteBuffer::ix() const {
            return (void *)(buffer + pos);
        }

        size_t ByteBuffer::minimum(size_t a, size_t b) const {
            return a < b ? a : b;
        }

        size_t ByteBuffer::limit() const {
            return lim;
        }


        void ByteBuffer::safeIncrementPosition(size_t t) {
            assert(pos + t <= capacity);
            pos += t;
        }
    }
}


