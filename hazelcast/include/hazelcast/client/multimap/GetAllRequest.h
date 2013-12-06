//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetAllRequest
#define HAZELCAST_GetAllRequest

#include "hazelcast/client/multimap/KeyBasedRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API GetAllRequest : public KeyBasedRequest, public RetryableRequest {
            public:
                GetAllRequest(const std::string& name, const serialization::Data& key);

                int getClassId() const;
            };
        }
    }
}

#endif //HAZELCAST_GetAllRequest
