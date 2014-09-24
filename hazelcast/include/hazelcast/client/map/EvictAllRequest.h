//
// Created by sancar koyunlu on 04/09/14.
//


#ifndef HAZELCAST_EvictAllRequest
#define HAZELCAST_EvictAllRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API EvictAllRequest : public impl::ClientRequest {
            public:
                EvictAllRequest(const std::string& name);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                std::string name;
            };
        }
    }
}


#endif //HAZELCAST_EvictAllRequest
