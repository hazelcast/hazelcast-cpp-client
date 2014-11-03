//
// Created by sancar koyunlu on 24/07/14.
//

#include "hazelcast/client/serialization/pimpl/Packet.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include <sstream>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                byte const Packet::VERSION = 2;
                int const Packet::HEADER_OP = 0;
                int const Packet::HEADER_RESPONSE = 1;
                int const Packet::HEADER_EVENT = 2;
                int const Packet::HEADER_WAN_REPLICATION = 3;
                int const Packet::HEADER_URGENT = 4;
                int const Packet::ST_VERSION = 10;
                int const Packet::ST_HEADER = 11;
                int const Packet::ST_PARTITION = 12;

                Packet::Packet(PortableContext& context)
                : DataAdapter(context)
                , partitionId(-1) {

                }

                Packet::Packet(PortableContext& context, const Data& data)
                : DataAdapter(context, data)
                , partitionId(-1) {

                }

                bool Packet::isHeaderSet(int bit) const {
                    return (header & 1 << bit) != 0;
                }

                bool Packet::writeTo(util::ByteBuffer& destination) {
                    if (!isStatusSet(ST_VERSION)) {
                        if (!destination.hasRemaining()) {
                            return false;
                        }
                        destination.writeByte(VERSION);
                        setStatus(ST_VERSION);
                    }
                    if (!isStatusSet(ST_HEADER)) {
                        if (destination.remaining() < 2) {
                            return false;
                        }
                        destination.writeShort(header);
                        setStatus(ST_HEADER);
                    }
                    if (!isStatusSet(ST_PARTITION)) {
                        if (destination.remaining() < 4) {
                            return false;
                        }
                        destination.writeInt(partitionId);
                        setStatus(ST_PARTITION);
                    }
                    return DataAdapter::writeTo(destination);
                }

                bool Packet::readFrom(util::ByteBuffer& source) {
                    if (!isStatusSet(ST_VERSION)) {
                        if (!source.hasRemaining()) {
                            return false;
                        }
                        byte version = source.readByte();
                        setStatus(ST_VERSION);
                        if (VERSION != version) {
                            std::stringstream stringstream;
                            stringstream << "Packet versions are not matching! This -> " << VERSION << ", Incoming -> " << version;
                            throw exception::IllegalArgumentException("Packet::readFrom", stringstream.str());
                        }
                    }
                    if (!isStatusSet(ST_HEADER)) {
                        if (source.remaining() < 2) {
                            return false;
                        }
                        header = source.readShort();
                        setStatus(ST_HEADER);
                    }
                    if (!isStatusSet(ST_PARTITION)) {
                        if (source.remaining() < 4) {
                            return false;
                        }
                        partitionId = source.readInt();
                        setStatus(ST_PARTITION);
                    }
                    return DataAdapter::readFrom(source);
                }


                int Packet::getHeader() const {
                    return header;
                }

                int Packet::getPartitionId() const {
                    return partitionId;
                }

                void Packet::setHeader(short header) {
                    this->header = header;
                }

                void Packet::setPartitionId(int partitionId) {
                    this->partitionId = partitionId;
                }
            }
        }
    }
}
