//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_COLLECTION_PORTABLE_HOOK
#define HAZELCAST_COLLECTION_PORTABLE_HOOK

namespace hazelcast {
    namespace client {
        namespace collection {
            namespace CollectionPortableHook {
                enum {
                    F_ID = -12,

                    ADD_ALL = 1,
                    CLEAR = 2,
                    COMPARE_AND_REMOVE = 3,
                    CONTAINS_ALL = 4,
                    CONTAINS_ENTRY = 5,
                    CONTAINS = 6,
                    COUNT = 7,
                    ENTRY_SET = 8,
                    GET_ALL = 9,
                    GET = 10,
                    INDEX_OF = 11,
                    KEY_SET = 12,
                    PUT = 13,
                    REMOVE_ALL = 14,
                    REMOVE_INDEX = 15,
                    REMOVE = 16,
                    SET = 17,
                    SIZE = 18,
                    VALUES = 19,
                    ADD_ENTRY_LISTENER = 20,
                    ENTRY_SET_RESPONSE = 21,
                    LOCK = 22,
                    UNLOCK = 23,
                    IS_LOCKED = 24,
                    ADD_ITEM_LISTENER = 25,
                    DESTROY = 26,

                    TXN_MM_PUT = 27,
                    TXN_MM_GET = 28,
                    TXN_MM_REMOVE = 29,
                    TXN_MM_VALUE_COUNT = 30,
                    TXN_MM_SIZE = 31,

                    TXN_LIST_ADD = 32,
                    TXN_LIST_REMOVE = 33,
                    TXN_LIST_SIZE = 34,

                    TXN_SET_ADD = 35,
                    TXN_SET_REMOVE = 36,
                    TXN_SET_SIZE = 37
                };
            }

        }
    }
}


#endif //HAZELCAST_COLLECTION_PORTABLE_HOOK
