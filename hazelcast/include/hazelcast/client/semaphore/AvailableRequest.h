//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AvailableRequest
#define HAZELCAST_AvailableRequest

#include "hazelcast/client/semaphore/SemaphoreRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class AvailableRequest : public SemaphoreRequest, public RetryableRequest {
            public:
                AvailableRequest(const std::string &instanceName);

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;
            };
        }
    }
}

#endif //HAZELCAST_AvailableRequest
