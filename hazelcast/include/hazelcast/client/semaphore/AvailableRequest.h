//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AvailableRequest
#define HAZELCAST_AvailableRequest

#include "hazelcast/client/semaphore/SemaphoreRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class HAZELCAST_API AvailableRequest : public SemaphoreRequest{
            public:
                AvailableRequest(const std::string &instanceName);

                int getClassId() const;

                bool isRetryable() const;

            };
        }
    }
}

#endif //HAZELCAST_AvailableRequest

