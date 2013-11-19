//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SizeRequest
#define HAZELCAST_SizeRequest

#include "hazelcast/client/impl/RetryableRequest.h"
#include "hazelcast/client/multimap/AllPartitionsRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class SizeRequest : public AllPartitionsRequest, public RetryableRequest {
            public:
                SizeRequest(const std::string& name);

                int getClassId() const;
            };
        }
    }
}


#endif //HAZELCAST_SizeRequest
