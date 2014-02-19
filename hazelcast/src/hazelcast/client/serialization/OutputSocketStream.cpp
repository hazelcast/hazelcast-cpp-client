#include "hazelcast/client/serialization/OutputSocketStream.h"
#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            OutputSocketStream::OutputSocketStream(Socket &socket)
            :socket(socket) {
            }

            void OutputSocketStream::write(const std::vector<byte> &bytes) {
                socket.send((void *) bytes.data(), sizeof (char) * bytes.size());
            };


            void OutputSocketStream::writeByte(int i) {
                char x = (char) (0xff & i);
                socket.send((void *) &(x), sizeof(char));
            };


            void OutputSocketStream::writeInt(int v) {
                writeByte((v >> 24));
                writeByte((v >> 16));
                writeByte((v >> 8));
                writeByte(v);
            };

        }
    }
}