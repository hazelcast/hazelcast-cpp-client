//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AtomicLongRequest
#define HAZELCAST_AtomicLongRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class PortableWriter;
        }

        namespace atomiclong {

            class HAZELCAST_API AtomicLongRequest : public impl::ClientRequest {
            public:
                AtomicLongRequest(const std::string &instanceName, long delta);

                virtual int getFactoryId() const;

                virtual void write(serialization::PortableWriter &writer) const;

            private:
                long delta;
                std::string instanceName;
            };
        }
    }
}

#endif //HAZELCAST_AtomicLongRequest

