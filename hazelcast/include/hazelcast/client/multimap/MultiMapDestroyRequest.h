//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_DestroyRequest
#define HAZELCAST_DestroyRequest

#include "AllPartitionsRequest.h"
#include "RetryableRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class MultiMapDestroyRequest : public AllPartitionsRequest, public RetryableRequest {
            public:
                MultiMapDestroyRequest(const std::string& name);

                int getClassId() const;
            };
        }
    }
}

#endif //HAZELCAST_DestroyRequest
