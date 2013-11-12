//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CountDownLatchPortableHook
#define HAZELCAST_CountDownLatchPortableHook

namespace hazelcast {
    namespace client {
        namespace countdownlatch {

            namespace CountDownLatchPortableHook {
                enum {
                    F_ID = -14,

                    COUNT_DOWN = 1,
                    AWAIT = 2,
                    SET_COUNT = 3,
                    GET_COUNT = 4
                };
            };
        }
    }
}


#endif //HAZELCAST_CountDownLatchPortableHook
