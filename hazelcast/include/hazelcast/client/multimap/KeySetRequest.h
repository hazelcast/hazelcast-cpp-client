//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_KeySetRequest
#define HAZELCAST_KeySetRequest

#include "hazelcast/client/multimap/AllPartitionsRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class KeySetRequest : public AllPartitionsRequest{
            public:
                KeySetRequest(const std::string& name);

                int getClassId() const;

                bool isRetryable() const;
            };
        }
    }
}
#endif //HAZELCAST_KeySetRequest

