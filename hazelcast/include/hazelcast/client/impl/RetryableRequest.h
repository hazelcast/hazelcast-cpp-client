//
// Created by sancar koyunlu on 8/22/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_RetryableRequest
#define HAZELCAST_RetryableRequest


namespace hazelcast {
    namespace client {
        class RetryableRequest {
        public:
            virtual ~RetryableRequest();
        };
    }
}


#endif //HAZELCAST_RetryableRequest
