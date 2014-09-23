//
// Created by sancar koyunlu on 20/08/14.
//


#ifndef HAZELCAST_ClientPingRequest
#define HAZELCAST_ClientPingRequest

#include "hazelcast/client/impl/ClientRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class PortableWriter;
        }

        namespace impl {
            class ClientPingRequest : public impl::ClientRequest {
            public:
                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

                virtual bool isBindToSingleConnection() const;
            };
        }
    }
}

#endif //HAZELCAST_ClientPingRequest
