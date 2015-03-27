#include <hazelcast/client/serialization/pimpl/Packet.h>
#include "hazelcast/client/connection/OutputSocketStream.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            OutputSocketStream::OutputSocketStream(Socket& socket)
            : socket(socket) {
            }

            void OutputSocketStream::write(const std::vector<byte>& bytes) {
                socket.send((void *)&(bytes[0]), (int)(sizeof(char) * bytes.size()));
            }


            void OutputSocketStream::writeByte(int i) {
                char x = (char)(0xff & i);
                socket.send((void *)&(x), sizeof(char));
            }


            void OutputSocketStream::writeInt(int v) {
                writeByte((v >> 24));
                writeByte((v >> 16));
                writeByte((v >> 8));
                writeByte(v);
            }


            void OutputSocketStream::writeShort(int v) {
                writeByte((v >> 8));
                writeByte(v);
            }

            void OutputSocketStream::writePacket(serialization::pimpl::Packet const& packet) {
                writeByte(serialization::pimpl::Packet::VERSION);

                writeShort(packet.getHeader());

                writeInt(packet.getPartitionId());

                writeValue(packet.getData());
            }


            void OutputSocketStream::writeValue(serialization::pimpl::Data const &data) {
                writeInt((int)data.totalSize());

                write(data.toByteArray());
            }
        }
    }
}
