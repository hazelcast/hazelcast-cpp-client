//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AcquireRequest
#define HAZELCAST_AcquireRequest

#include "hazelcast/client/semaphore/SemaphoreRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class HAZELCAST_API AcquireRequest : public SemaphoreRequest {
            public:
                AcquireRequest(const std::string &instanceName, int permitCount, long timeout);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:

                long timeout;
            };
        }
    }
}

#endif //HAZELCAST_AcquireRequest
