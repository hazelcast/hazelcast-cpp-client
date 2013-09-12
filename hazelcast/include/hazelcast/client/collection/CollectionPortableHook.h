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
                    F_ID = -20,

                    COLLECTION_SIZE = 1,
                    COLLECTION_CONTAINS = 2,
                    COLLECTION_ADD = 3,
                    COLLECTION_REMOVE = 4,
                    COLLECTION_ADD_ALL = 5,
                    COLLECTION_COMPARE_AND_REMOVE = 6,
                    COLLECTION_CLEAR = 7,
                    COLLECTION_GET_ALL = 8,
                    COLLECTION_ADD_LISTENER = 9,
                    COLLECTION_DESTROY = 10,
                    LIST_ADD_ALL = 11,
                    LIST_GET = 12,
                    LIST_SET = 13,
                    LIST_ADD = 14,
                    LIST_REMOVE = 15,
                    LIST_INDEX_OF = 16,
                    LIST_SUB = 17,

                    TXN_LIST_ADD = 18,
                    TXN_LIST_REMOVE = 19,
                    TXN_LIST_SIZE = 20,

                    TXN_SET_ADD = 21,
                    TXN_SET_REMOVE = 22,
                    TXN_SET_SIZE = 23
                };
            }

        }
    }
}


#endif //HAZELCAST_COLLECTION_PORTABLE_HOOK
