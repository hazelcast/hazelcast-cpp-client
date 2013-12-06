//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_EntrySetRequest
#define HAZELCAST_EntrySetRequest

#include "hazelcast/client/multimap/AllPartitionsRequest.h"
#include "hazelcast/client/impl/RetryableRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API EntrySetRequest : public AllPartitionsRequest, public RetryableRequest {
            public:
                EntrySetRequest(const std::string& name);

                int getClassId() const;
            };
        }
    }
}
#endif //HAZELCAST_EntrySetRequest
