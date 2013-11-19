//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AtomicLongRequest
#define HAZELCAST_AtomicLongRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class PortableWriter;
        }

        namespace atomiclong {

            class AtomicLongRequest : public impl::PortableRequest {
            public:
                AtomicLongRequest(const std::string &instanceName, long delta);

                virtual int getFactoryId() const;

                void writePortable(serialization::PortableWriter &writer) const;

            private:
                long delta;
                const std::string &instanceName;
            };
        }
    }
}

#endif //HAZELCAST_AtomicLongRequest
