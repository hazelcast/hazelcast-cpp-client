//
// Created by sancar koyunlu on 9/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MultiMapPortableHook
#define HAZELCAST_MultiMapPortableHook


namespace hazelcast {
    namespace client {
        namespace multimap {
            namespace MultiMapPortableHook {
                enum {
                    F_ID = -12,

                    CLEAR = 1,
                    CONTAINS_ENTRY = 2,
                    COUNT = 3,
                    ENTRY_SET = 4,
                    GET_ALL = 5,
                    GET = 6,
                    KEY_SET = 7,
                    PUT = 8,
                    REMOVE_ALL = 9,
                    REMOVE = 10,
                    SET = 11,
                    SIZE = 12,
                    VALUES = 13,
                    ADD_ENTRY_LISTENER = 14,
                    ENTRY_SET_RESPONSE = 15,
                    LOCK = 16,
                    UNLOCK = 17,
                    IS_LOCKED = 18,

                    TXN_MM_PUT = 19,
                    TXN_MM_GET = 20,
                    TXN_MM_REMOVE = 21,
                    TXN_MM_VALUE_COUNT = 22,
                    TXN_MM_SIZE = 23,
                    REMOVE_ENTRY_LISTENER = 24
                };
            }

        }
    }
}

#endif //HAZELCAST_MultiMapPortableHook

