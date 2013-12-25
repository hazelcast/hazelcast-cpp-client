//
// Created by sancar koyunlu on 24/12/13.
//




#ifndef HAZELCAST_CircularBuffer
#define HAZELCAST_CircularBuffer

#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace util {
        class CircularBuffer {
        public:
            CircularBuffer(int capacity)
            : capacity(capacity)
            , begin(new char [capacity])
            , writeHead(begin)
            , readHead(begin) {

            };

            void readFrom(client::Socket &socket) {
                int remaining = distanceToEnd(writeHead);
                int i = socket.receive((void *) writeHead, remaining, 0);
                if (i < remaining) {
                    writeHead += i;
                    return;
                }
                writeHead = begin;
                i = socket.receive((void *) writeHead, capacity - remaining, 0);
                writeHead += i;
            };

            //range check is necessary before call to this function
            int readInt() {
                char a = readByte();
                char b = readByte();
                char c = readByte();
                char d = readByte();
                return (0xff000000 & (a << 24)) |
                        (0x00ff0000 & (b << 16)) |
                        (0x0000ff00 & (c << 8)) |
                        (0x000000ff & d);
            }
            
            int read(byte* to, int len){
                int m = std::max(remaining(), len);
                std::memcpy(to, readHead, m);
                advance(m);
                return m;
            }

            void write() {
                //TODO not implemented
            };

            ~CircularBuffer() {
                delete [] begin;
            }

            //range check is necessary before call to this function
            void readFully(std::vector<byte>&  buffer){
                std::memcpy(&(buffer[0]), readHead, buffer.size());
                readHead += buffer.size();
            }

            int remaining() const {
                int r = writeHead - readHead;
                return r >= 0 ? r : capacity - r;
            }

            //range check is necessary before call to this function
            void advance(int l) {
                int i = distanceToEnd(readHead) - l;
                readHead = i > 0 ? readHead + l : begin - i;
            }

        private:
            char readByte() {
                char c = *readHead;
                advance(1);
                return c;
            }


            int distanceToEnd(char *p) {
                return begin + capacity - p;
            }


            int capacity;
            char *begin;
            char *writeHead;
            char *readHead;

        };
    }
}


#endif //HAZELCAST_CircularBuffer
