//
// Created by sancar koyunlu on 8/22/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_RetryableRequest
#define HAZELCAST_RetryableRequest

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        class HAZELCAST_API RetryableRequest {
        public:
            virtual ~RetryableRequest();
        };
    }
}


#endif //HAZELCAST_RetryableRequest
