//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SemaphorePortableHook
#define HAZELCAST_SemaphorePortableHook


namespace hazelcast {
    namespace client {
        namespace semaphore {

            namespace SemaphorePortableHook {
                enum {
                    F_ID = -16,

                    ACQUIRE = 1,
                    AVAILABLE = 2,
                    DRAIN = 3,
                    INIT = 4,
                    REDUCE = 5,
                    RELEASE = 6
                };
            };
        }
    }
}


#endif //HAZELCAST_SemaphorePortableHook

