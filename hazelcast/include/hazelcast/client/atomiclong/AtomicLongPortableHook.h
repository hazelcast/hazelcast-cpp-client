//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AtomicLongPortableHook
#define HAZELCAST_AtomicLongPortableHook

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            namespace AtomicLongPortableHook {
                enum {
                    F_ID = -17,

                    ADD_AND_GET = 1,
                    COMPARE_AND_SET = 2,
                    GET_AND_ADD = 3,
                    GET_AND_SET = 4,
                    SET = 5,
                };
            };
        }
    }
}

#endif //HAZELCAST_AtomicLongPortableHook

