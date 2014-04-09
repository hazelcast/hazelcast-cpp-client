//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DrainRequest
#define HAZELCAST_DrainRequest

#include "hazelcast/client/semaphore/SemaphoreRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class HAZELCAST_API DrainRequest : public SemaphoreRequest {
            public:
                DrainRequest(const std::string &instanceName);

                int getClassId() const;
            };
        }
    }
}


#endif //HAZELCAST_DrainRequest

