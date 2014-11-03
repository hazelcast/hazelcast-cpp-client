//
// Created by sancar koyunlu on 24/07/14.
//


#ifndef HAZELCAST_Packet
#define HAZELCAST_Packet

#include "hazelcast/client/serialization/pimpl/DataAdapter.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;
        }

        namespace serialization {
            namespace pimpl {
                class Packet : public DataAdapter {
                public:
                    static byte const VERSION;
                    static int const HEADER_OP;
                    static int const HEADER_RESPONSE;
                    static int const HEADER_EVENT;
                    static int const HEADER_WAN_REPLICATION;
                    static int const HEADER_URGENT;

                    Packet(PortableContext& context);

                    Packet(PortableContext& context, const Data& data);

                    bool isHeaderSet(int bit) const;

                    bool writeTo(util::ByteBuffer& destination);

                    bool readFrom(util::ByteBuffer& source);

                    int getHeader() const;

                    int getPartitionId() const;

                    void setHeader(short header);

                    void setPartitionId(int partitionId);

                private:
                    static int const ST_VERSION;
                    static int const ST_HEADER;
                    static int const ST_PARTITION;

                    short header;
                    int partitionId;
                };
            }
        }
    }
}

#endif //HAZELCAST_Packet
