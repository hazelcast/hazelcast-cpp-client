//
// Created by sancar koyunlu on 21/08/14.
//


#ifndef HAZELCAST_RemoveAllListeners
#define HAZELCAST_RemoveAllListeners


#include "hazelcast/client/impl/ClientRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class PortableWriter;
        }

        namespace impl {
            class RemoveAllListeners : public impl::ClientRequest {
            public:

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;
            };
        }
    }
}


#endif //HAZELCAST_RemoveAllListeners
