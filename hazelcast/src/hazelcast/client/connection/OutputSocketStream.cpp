#include "hazelcast/client/connection/OutputSocketStream.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            OutputSocketStream::OutputSocketStream(Socket &socket)
            :socket(socket) {
            }

            void OutputSocketStream::write(const std::vector<byte> &bytes) {
                socket.send((void *) &(bytes[0]), sizeof (char) * bytes.size());
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


            void OutputSocketStream::writeData(serialization::pimpl::Data const &data) {
                writeInt(data.getType());
                if (data.cd != NULL) {
                    writeInt(data.cd->getClassId());
                    writeInt(data.cd->getFactoryId());
                    writeInt(data.cd->getVersion());
                    const std::vector<byte> &classDefBytes = data.cd->getBinary();

                    writeInt(classDefBytes.size());
                    write(classDefBytes);
                } else {
                    writeInt(data.NO_CLASS_ID);
                }
                int len = data.bufferSize();
                writeInt(len);
                if (len > 0) {
                    write(*(data.buffer.get()));
                }
                writeInt(data.getPartitionHash());
            }
        }
    }
}
