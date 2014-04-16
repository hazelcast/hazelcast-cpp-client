//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SetRequest
#define HAZELCAST_SetRequest

#include "hazelcast/client/atomiclong/AtomicLongRequest.h"

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            class HAZELCAST_API SetRequest : public AtomicLongRequest {
            public:
                SetRequest(const std::string &instanceName, long value);

                int getClassId() const;

            };
        }
    }
}


#endif //HAZELCAST_SetRequest

