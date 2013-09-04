//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DataSerializableHook
#define HAZELCAST_DataSerializableHook

namespace hazelcast {
    namespace client {
        namespace map {
            namespace DataSerializableHook {
                enum {
                    F_ID = -10,
                    PUT = 0,
                    GET = 1,
                    REMOVE = 2,
                    PUT_BACKUP = 3,
                    REMOVE_BACKUP = 4,
                    DATA_RECORD = 5,
                    OBJECT_RECORD = 6,
                    CACHED_RECORD = 7,
                    KEY_SET = 8,
                    VALUES = 9,
                    ENTRY_SET = 10,
                    ENTRY_VIEW = 11,
                    MAP_STATS = 12,
                    QUERY_RESULT_ENTRY = 13,
                    QUERY_RESULT_SET = 14
                };
            }

        }
    }
}


#endif //HAZELCAST__MAP_RESPONSE_ID
