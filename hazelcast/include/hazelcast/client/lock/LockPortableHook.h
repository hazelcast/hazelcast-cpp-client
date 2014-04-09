//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_LockPortableHook
#define HAZELCAST_LockPortableHook

namespace hazelcast {
    namespace client {
        namespace lock {

            namespace LockPortableHook {
                enum {
                    FACTORY_ID = -15,

                    LOCK = 1,
                    UNLOCK = 2,
                    IS_LOCKED = 3,
                    GET_LOCK_COUNT = 5,
                    GET_REMAINING_LEASE = 6
                };
            };
        }
    }
}


#endif //HAZELCAST_LockPortableHook

