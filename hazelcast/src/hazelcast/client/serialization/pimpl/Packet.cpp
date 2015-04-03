/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// Created by sancar koyunlu on 24/07/14.
// Modified: ihsan demir on 18/03/2015
//

#include "hazelcast/client/serialization/pimpl/Packet.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/util/Bits.h"

#include <sstream>
#include <limits.h>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                byte const Packet::VERSION = 4;

                int const Packet::HEADER_EVENT = 2;

                // The value of these constants is important. The order needs to match the order in the read/write process
                short const Packet::PERSIST_VERSION = 1;
                short const Packet::PERSIST_HEADER = 2;
                short const Packet::PERSIST_PARTITION = 3;
                short const Packet::PERSIST_SIZE = 4;
                short const Packet::PERSIST_VALUE = 5;

                short const Packet::PERSIST_COMPLETED = SHRT_MAX;

                Packet::Packet(PortableContext &ctx) :
                        context(ctx),
                        partitionId(-1),
                        header(0),
                        persistStatus(0),
                        persistedSize(0),
                        valueOffset(0)
				{
                }

                Packet::Packet(PortableContext &ctx, const Data &packetData) :
                        context(ctx),
                        data(packetData),
                        partitionId(-1),
                        header(0),
                        persistStatus(0),
                        persistedSize(0),
                        valueOffset(0) {
                }

                Packet::Packet(PortableContext &ctx, const Data &packetData, int partition) :
                        context(ctx),
                        data(packetData),
                        partitionId(partition),
                        header(0),
                        persistStatus(0),
                        persistedSize(0),
                        valueOffset(0) {
                }

                Packet::~Packet() {
                }

                void Packet::setHeader(int bit) {
                    header |= 1 << bit;
                }

                bool Packet::isHeaderSet(int bit) const {
                    return (header & 1 << bit) != 0;
                }

                /**
                 * Returns the header of the Packet. The header is used to figure out what the content is of this Packet before
                 * the actual payload needs to be processed.
                 *
                 * @return the header.
                 */
                short  Packet::getHeader() const {
                    return header;
                }

                /**
                 * Returns the partition id of this packet. If this packet is not for a particular partition, -1 is returned.
                 *
                 * @return the partition id.
                 */
                int Packet::getPartitionId() const {
                    return partitionId;
                }

                bool Packet::writeTo(util::ByteBuffer &destination) {
                    if (!writeVersion(destination)) {
                        return false;
                    }

                    if (!writeHeader(destination)) {
                        return false;
                    }

                    if (!writePartition(destination)) {
                        return false;
                    }

                    if (!writeSize(destination)) {
                        return false;
                    }

                    if (!writeValue(destination)) {
                        return false;
                    }

                    setPersistStatus(PERSIST_COMPLETED);
                    return true;
                }

                bool Packet::readFrom(util::ByteBuffer &source) {
                    if (!readVersion(source)) {
                        return false;
                    }

                    if (!readHeader(source)) {
                        return false;
                    }

                    if (!readPartition(source)) {
                        return false;
                    }

                    if (!readSize(source)) {
                        return false;
                    }

                    if (!readValue(source)) {
                        return false;
                    }

                    setPersistStatus(PERSIST_COMPLETED);
                    return true;
                }

                // ========================= version =================================================
                bool Packet::readVersion(util::ByteBuffer &source) {
                    if (!isPersistStatusSet(PERSIST_VERSION)) {
                        if (!source.hasRemaining()) {
                            return false;
                        }

                        byte version = source.readByte();
                        setPersistStatus(PERSIST_VERSION);
                        if (VERSION != version) {
                            std::stringstream stringstream;
                            stringstream << "Packet versions are not matching! This -> " << VERSION << ", Incoming -> " << version;
                            throw exception::IllegalArgumentException("Packet::readVersion", stringstream.str());
                        }
                    }
                    return true;
                }


				bool Packet::writeVersion(util::ByteBuffer &destination) {
					if (!isPersistStatusSet(PERSIST_VERSION)) {
						if (!destination.hasRemaining()) {
							return false;
						}
						destination.writeByte(VERSION);
						setPersistStatus(PERSIST_VERSION);
						return true;
					}
                    return true;
				}

			    // ========================= header =================================================

			    bool Packet::readHeader(util::ByteBuffer &source) {
			        if (!isPersistStatusSet(PERSIST_HEADER)) {
			            if (source.remaining() < util::Bits::SHORT_SIZE_IN_BYTES) {
			                return false;
			            }
			            header = source.readShort();
			            setPersistStatus(PERSIST_HEADER);
			        }
			        return true;
			    }

			    bool Packet::writeHeader(util::ByteBuffer &destination) {
			        if (!isPersistStatusSet(PERSIST_HEADER)) {
			            if (destination.remaining() < util::Bits::SHORT_SIZE_IN_BYTES) {
			                return false;
			            }
			            destination.writeShort(header);
			            setPersistStatus(PERSIST_HEADER);
			        }
			        return true;
			    }

			    // ========================= partition =================================================

			    bool Packet::readPartition(util::ByteBuffer &source) {
			        if (!isPersistStatusSet(PERSIST_PARTITION)) {
			            if (source.remaining() < util::Bits::INT_SIZE_IN_BYTES) {
			                return false;
			            }
			            partitionId = source.readInt();
			            setPersistStatus(PERSIST_PARTITION);
			        }
			        return true;
			    }


			    bool Packet::writePartition(util::ByteBuffer &destination) {
			        if (!isPersistStatusSet(PERSIST_PARTITION)) {
			            if (destination.remaining() < util::Bits::INT_SIZE_IN_BYTES) {
			                return false;
			            }
			            destination.writeInt(partitionId);
			            setPersistStatus(PERSIST_PARTITION);
			        }
			        return true;
			    }


			    // ========================= size =================================================

			    bool Packet::readSize(util::ByteBuffer &source) {
			        if (!isPersistStatusSet(PERSIST_SIZE)) {
			            if (source.remaining() < util::Bits::INT_SIZE_IN_BYTES) {
			                return false;
			            }

			            persistedSize = (size_t)source.readInt();
			            setPersistStatus(PERSIST_SIZE);
			        }
			        return true;
			    }

			    bool Packet::writeSize(util::ByteBuffer &destination) {
			        if (!isPersistStatusSet(PERSIST_SIZE)) {
			            if (destination.remaining() < util::Bits::INT_SIZE_IN_BYTES) {
			                return false;
			            }

			            persistedSize = data.totalSize();
			            destination.writeInt((int)persistedSize);
			            setPersistStatus(PERSIST_SIZE);
			        }
			        return true;
			    }

			    // ========================= value =================================================

			    bool Packet::writeValue(util::ByteBuffer &destination) {
			        if (!isPersistStatusSet(PERSIST_VALUE)) {
			            if (persistedSize > 0) {
			                // the number of bytes that can be written to the bb.
			                size_t bytesWritable = destination.remaining();

			                // the number of bytes that need to be written.
			                size_t bytesNeeded = persistedSize - valueOffset;

			                size_t bytesWrite;
			                bool done;
			                if (bytesWritable >= bytesNeeded) {
			                    // All bytes for the value are available.
			                    bytesWrite = bytesNeeded;
			                    done = true;
			                } else {
			                    // Not all bytes for the value are available. So lets write as much as is available.
			                    bytesWrite = bytesWritable;
			                    done = false;
			                }

			                std::vector<byte> &dataBytesVector = data.toByteArray();
                            size_t numActuallyWritten = destination.readFrom(dataBytesVector, valueOffset, (size_t)bytesWrite);
			                valueOffset += numActuallyWritten;

                            done = done && (numActuallyWritten == bytesWrite);

			                if (!done) {
			                    return false;
			                }
			            }
			            setPersistStatus(PERSIST_VALUE);
			        }
			        return true;
			    }

			    bool Packet::readValue(util::ByteBuffer &source) {
			        if (!isPersistStatusSet(PERSIST_VALUE)) {
                        std::vector<byte> &byteVector = data.toByteArray();

			            if (persistedSize > 0) {
			                size_t bytesReadable = source.remaining();

			                size_t bytesNeeded = persistedSize - valueOffset;

			                bool done;
			                size_t bytesRead;
			                if (bytesReadable >= bytesNeeded) {
			                    bytesRead = bytesNeeded;
			                    done = true;
			                } else {
			                    bytesRead = bytesReadable;
			                    done = false;
			                }

			                // read the data from the byte-buffer into the bytes-array.
                            size_t numActuallyRead = source.writeTo(byteVector, valueOffset, bytesRead);
                            valueOffset += numActuallyRead;

                            done = done && (numActuallyRead == bytesRead);

			                if (!done) {
			                    return false;
			                }
			            }

			            setPersistStatus(PERSIST_VALUE);
			        }
			        return true;
			    }

                void Packet::setPartitionId(int partition) {
                    partitionId = partition;
                }

			    const Data &Packet::getData() const {
			        return data;
			    }

                Data &Packet::getDataAsModifiable() {
                    return data;
                }

			    void Packet::setPersistStatus(short persistStatus) {
			        this->persistStatus = persistStatus;
			    }

			    bool Packet::isPersistStatusSet(short status) const {
			        return this->persistStatus >= status;
			    }

                PortableContext &Packet::getPortableContext() const {
                    return context;
                }

            }
        }
    }
}
