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


#ifndef HAZELCAST_Packet
#define HAZELCAST_Packet

#include "hazelcast/client/serialization/pimpl/SocketWritable.h"
#include "hazelcast/client/serialization/pimpl/SocketReadable.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace util {
        class ByteBuffer;
    }
    namespace client {
        namespace connection {
            class Connection;
        }

        namespace serialization {
            namespace pimpl {

                class PortableContext;

                class HAZELCAST_API Packet : public SocketWritable , public SocketReadable {
                public:
                    static byte const VERSION;

                    static int const HEADER_EVENT;

                    Packet(PortableContext& ctx);

                    Packet(PortableContext& ctx, const Data& data);

                    Packet(PortableContext& ctx, const Data &data, int partition);

                    virtual ~Packet();

                    PortableContext &getPortableContext() const;


                    void setHeader(int bit);

                    bool isHeaderSet(int bit) const;

                    /**
                     * Returns the header of the Packet. The header is used to figure out what the content is of this Packet before
                     * the actual payload needs to be processed.
                     *
                     * @return the header.
                     */
                    short getHeader() const;

                    /**
                     * Returns the partition id of this packet. If this packet is not for a particular partition, -1 is returned.
                     *
                     * @return the partition id.
                     */
                    int getPartitionId() const;

                    // SocketWritable interface
                    bool writeTo(util::ByteBuffer &destination);

                    // SocketReadable interface
                    bool readFrom(util::ByteBuffer &source);


                    void setPartitionId(int partition);

					const Data &getData() const;

                    Data &getDataAsModifiable();

                private:
                    PortableContext &context;
                    Data data;
                    bool dataNull;
                    int partitionId;
                    short header;

                    int persistStatus;

                    size_t size;
                    size_t valueOffset;

                    // The value of these constants is important. The order needs to
                    // match the order in the read/write process
                    static short const PERSIST_VERSION;
                    static short const PERSIST_HEADER;
                    static short const PERSIST_PARTITION;
                    static short const PERSIST_SIZE;
                    static short const PERSIST_VALUE;

                    static short const PERSIST_COMPLETED;

                    bool readVersion(util::ByteBuffer& destination);
                    bool writeVersion(util::ByteBuffer& destination);

                    bool readHeader(util::ByteBuffer& destination);
                    bool writeHeader(util::ByteBuffer& destination);

                    bool readPartition(util::ByteBuffer& destination);
                    bool writePartition(util::ByteBuffer& destination);

                    bool readSize(util::ByteBuffer& destination);
                    bool writeSize(util::ByteBuffer& destination);

                    bool readValue(util::ByteBuffer& destination);
                    bool writeValue(util::ByteBuffer& destination);

                    void setPersistStatus(short persistStatus);

                    bool isPersistStatusSet(short status) const;

                };
            }
        }
    }
}

#endif //HAZELCAST_Packet
