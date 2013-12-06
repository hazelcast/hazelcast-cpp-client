//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ClearRequest
#define HAZELCAST_ClearRequest

#include "hazelcast/client/impl/RetryableRequest.h"
#include "hazelcast/client/multimap/AllPartitionsRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API ClearRequest : public AllPartitionsRequest, public RetryableRequest {
            public:
                ClearRequest(const std::string& name);

                int getClassId() const;

            };
        }
    }
}

#endif //HAZELCAST_ClearRequest
