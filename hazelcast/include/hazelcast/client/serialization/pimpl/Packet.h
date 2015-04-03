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

#include "SocketWritable.h"
#include "SocketReadable.h"
#include "Data.h"
#include <hazelcast/client/connection/Connection.h>

using namespace hazelcast::client::connection;

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;
        }

        namespace serialization {
            namespace pimpl {

                class Packet : public SocketWritable , public SocketReadable {
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
                    bool writeTo(ByteBuffer &destination);

                    // SocketReadable interface
                    bool readFrom(ByteBuffer &source);


                    void setPartitionId(int partition);

					const Data &getData() const;

                    Data &getDataAsModifiable();

                private:
                    PortableContext &context;
                    Data data;
                    int partitionId;
                    short header;

                    int persistStatus;

                    size_t persistedSize;
                    size_t valueOffset;

                    // The value of these constants is important. The order needs to
                    // match the order in the read/write process
                    static short const PERSIST_VERSION;
                    static short const PERSIST_HEADER;
                    static short const PERSIST_PARTITION;
                    static short const PERSIST_SIZE;
                    static short const PERSIST_VALUE;

                    static short const PERSIST_COMPLETED;

                    bool readVersion(ByteBuffer& destination);
                    bool writeVersion(ByteBuffer& destination);

                    bool readHeader(ByteBuffer& destination);
                    bool writeHeader(ByteBuffer& destination);

                    bool readPartition(ByteBuffer& destination);
                    bool writePartition(ByteBuffer& destination);

                    bool readSize(ByteBuffer& destination);
                    bool writeSize(ByteBuffer& destination);

                    bool readValue(ByteBuffer& destination);
                    bool writeValue(ByteBuffer& destination);

                    void setPersistStatus(short persistStatus);

                    bool isPersistStatusSet(short status) const;

                };
            }
        }
    }
}

#endif //HAZELCAST_Packet
