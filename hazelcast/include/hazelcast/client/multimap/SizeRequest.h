//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SizeRequest
#define HAZELCAST_SizeRequest

#include "hazelcast/client/multimap/AllPartitionsRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API SizeRequest : public AllPartitionsRequest{
            public:
                SizeRequest(const std::string& name);

                int getClassId() const;

                bool isRetryable() const;
            };
        }
    }
}


#endif //HAZELCAST_SizeRequest

