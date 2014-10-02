//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ValuesRequest
#define HAZELCAST_ValuesRequest

#include "hazelcast/client/multimap/AllPartitionsRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class ValuesRequest : public AllPartitionsRequest{
            public:
                ValuesRequest(const std::string& name);

                int getClassId() const;

                bool isRetryable() const;
            };
        }
    }
}


#endif //HAZELCAST_ValuesRequest

