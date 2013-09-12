//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetAllRequest
#define HAZELCAST_GetAllRequest

#include "KeyBasedRequest.h"
#include "RetryableRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class GetAllRequest : public KeyBasedRequest, public RetryableRequest {
            public:
                GetAllRequest(const std::string& name, const serialization::Data& key);

                int getClassId() const;
            };
        }
    }
}

#endif //HAZELCAST_GetAllRequest
