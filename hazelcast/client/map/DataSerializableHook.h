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
                    KEY_SET = 7,
                    VALUES = 8,
                    ENTRY_SET = 9,
                    ENTRY_VIEW = 10,
                    MAP_STATS = 11,
                    QUERY_RESULT_ENTRY = 12,
                    QUERY_RESULT_STREAM = 13
                };
            }

        }
    }
}


#endif //HAZELCAST__MAP_RESPONSE_ID
