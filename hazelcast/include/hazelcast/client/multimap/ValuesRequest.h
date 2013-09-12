//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ValuesRequest
#define HAZELCAST_ValuesRequest

#include "RetryableRequest.h"
#include "AllPartitionsRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class ValuesRequest : public AllPartitionsRequest, public RetryableRequest {
            public:
                ValuesRequest(const std::string& name);

                int getClassId() const;
            };
        }
    }
}


#endif //HAZELCAST_ValuesRequest
