//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SemaphoreRequest
#define HAZELCAST_SemaphoreRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace semaphore {
            class HAZELCAST_API SemaphoreRequest : public impl::PortableRequest {
            public:
                SemaphoreRequest(const std::string &instanceName, int permitCount);

                virtual int getFactoryId() const;

                virtual void write(serialization::PortableWriter &writer) const;

            private:

                std::string instanceName;
                int permitCount;
            };
        }
    }
}

#endif //HAZELCAST_SemaphoreRequest

