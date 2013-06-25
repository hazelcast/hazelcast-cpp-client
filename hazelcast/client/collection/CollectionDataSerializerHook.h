//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CollectionDataSerializerHook
#define HAZELCAST_CollectionDataSerializerHook


namespace hazelcast {
    namespace client {
        namespace collection {
            namespace CollectionDataSerializerHook {
                enum {
                    F_ID = -12,

                    ADD_ALL_BACKUP = 0,
                    ADD_ALL = 1,
                    CLEAR_BACKUP = 2,
                    CLEAR = 3,
                    COMPARE_AND_REMOVE_BACKUP = 4,
                    COMPARE_AND_REMOVE = 5,
                    CONTAINS_ALL = 6,
                    CONTAINS_ENTRY = 7,
                    CONTAINS = 8,
                    COUNT = 9,
                    ENTRY_SET = 10,
                    GET_ALL = 11,
                    GET = 12,
                    INDEX_OF = 13,
                    KEY_SET = 14,
                    PUT_BACKUP = 15,
                    PUT = 16,
                    REMOVE_ALL_BACKUP = 17,
                    REMOVE_ALL = 18,
                    REMOVE_BACKUP = 19,
                    REMOVE = 20,
                    REMOVE_INDEX_BACKUP = 21,
                    REMOVE_INDEX = 22,
                    SET_BACKUP = 23,
                    SET = 24,
                    SIZE = 25,
                    VALUES = 26,

                    TXN_COMMIT_BACKUP = 27,
                    TXN_COMMIT = 28,
                    TXN_GENERATE_RECORD_ID = 29,
                    TXN_LOCK_AND_GET = 30,
                    TXN_PREPARE_BACKUP = 31,
                    TXN_PREPARE = 32,
                    TXN_PUT = 33,
                    TXN_REMOVE_ALL = 34,
                    TXN_REMOVE = 35,
                    TXN_ROLLBACK_BACKUP = 36,
                    TXN_ROLLBACK = 37,

                    COLLECTION_PROXY_ID = 38
                };
            };
        }
    }
}

#endif //HAZELCAST_CollectionDataSerializarHook
