//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_CountRequest
#define HAZELCAST_CountRequest

#include "hazelcast/client/impl/RetryableRequest.h"
#include "hazelcast/client/multimap/KeyBasedRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API CountRequest : public KeyBasedRequest, public RetryableRequest {
            public:
                CountRequest(const std::string& name, const serialization::Data& key);

                int getClassId() const;
            };
        }
    }
}

#endif //HAZELCAST_CountRequest
