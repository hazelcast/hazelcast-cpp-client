#include <hazelcast/client/serialization/pimpl/Packet.h>
#include "hazelcast/client/connection/OutputSocketStream.h"
#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            OutputSocketStream::OutputSocketStream(Socket& socket)
            : socket(socket) {
            }

            void OutputSocketStream::write(const std::vector<byte>& bytes) {
                socket.send((void *)&(bytes[0]), sizeof(char) * bytes.size());
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
                writeData(packet.getData(), packet.getPortableContext());
            }


            void OutputSocketStream::writeData(serialization::pimpl::Data const& data, serialization::pimpl::PortableContext& context) {
                writeInt(data.getType());
                bool hasClassDef = data.hasClassDefinition();
                if (hasClassDef) {
                    writeByte(1);
                    int classDefCount = data.getClassDefinitionCount();
                    writeInt(classDefCount);
                    std::vector<boost::shared_ptr<serialization::ClassDefinition> > classDefinitions = data.getClassDefinitions(context);
                    for (int classDefIndex = 0; classDefIndex < classDefCount; classDefIndex++) {
                        boost::shared_ptr<serialization::ClassDefinition> cd = classDefinitions[classDefIndex];

                        //writeHeader
                        writeInt(cd->getFactoryId());
                        writeInt(cd->getClassId());
                        writeInt(cd->getVersion());

                        std::vector<byte> const& binary = cd->getBinary();
                        writeInt(binary.size());

                        //writeData
                        write(binary);
                    }
                } else {
                    writeByte(0);
                }
                writeInt(data.hasPartitionHash() ? data.getPartitionHash() : 0);
                int len = data.bufferSize();
                writeInt(len);
                if (len > 0) {
                    write(*(data.data.get()));
                }

            }
        }
    }
}
