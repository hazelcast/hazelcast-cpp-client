// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_QUEUE_PORTABLE_HOOK
#define HAZELCAST_QUEUE_PORTABLE_HOOK

namespace hazelcast {
    namespace client {
        namespace queue {
            namespace QueuePortableHook {
                enum {
                    F_ID = -11,
                    OFFER = 1,
                    SIZE = 2,
                    REMOVE = 3,
                    POLL = 4,
                    PEEK = 5,
                    ITERATOR = 6,
                    DRAIN = 7,
                    CONTAINS = 8,
                    COMPARE_AND_REMOVE = 9,
                    CLEAR = 10,
                    ADD_ALL = 11,
                    ADD_LISTENER = 12,
                    REMAINING_CAPACITY = 13,
                    DESTROY = 14,
                    TXN_OFFER = 15,
                    TXN_POLL = 16,
                    TXN_SIZE = 17
                };
            }

        }
    }
}


#endif //HAZELCAST__MAP_REQUEST_ID
