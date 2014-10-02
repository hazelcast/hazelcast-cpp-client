//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ReleaseRequest
#define HAZELCAST_ReleaseRequest

#include "hazelcast/client/semaphore/SemaphoreRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class ReleaseRequest : public SemaphoreRequest {
            public:
                ReleaseRequest(const std::string &instanceName, int permitCount);

                int getClassId() const;

            };
        }
    }
}

#endif //HAZELCAST_ReleaseRequest

