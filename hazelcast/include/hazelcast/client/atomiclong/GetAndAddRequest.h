//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetAndAddRequest
#define HAZELCAST_GetAndAddRequest

#include "hazelcast/client/atomiclong/AtomicLongRequest.h"

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            class HAZELCAST_API GetAndAddRequest : public AtomicLongRequest {
            public:
                GetAndAddRequest(const std::string &instanceName, long delta);

                int getClassId() const;

            };
        }
    }
}


#endif //HAZELCAST_GetAndAddRequest
