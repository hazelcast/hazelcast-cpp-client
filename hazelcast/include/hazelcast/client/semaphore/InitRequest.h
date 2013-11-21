//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_InitRequest
#define HAZELCAST_InitRequest

#include "hazelcast/client/semaphore/SemaphoreRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class InitRequest : public SemaphoreRequest {
            public:
                InitRequest(const std::string &instanceName, int permitCount);

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

            };
        }
    }
}

#endif //HAZELCAST_InitRequest
