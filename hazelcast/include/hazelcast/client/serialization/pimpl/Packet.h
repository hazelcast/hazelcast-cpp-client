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
                    static byte const VERSION = 2;
                    static int const HEADER_OP = 0;
                    static int const HEADER_RESPONSE = 1;
                    static int const HEADER_EVENT = 2;
                    static int const HEADER_WAN_REPLICATION = 3;
                    static int const HEADER_URGENT = 4;

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
                    static int const ST_VERSION = 11;
                    static int const ST_HEADER = 12;
                    static int const ST_PARTITION = 13;

                    short header;
                    int partitionId;
                };
            }
        }
    }
}

#endif //HAZELCAST_Packet
