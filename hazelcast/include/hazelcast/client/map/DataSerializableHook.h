/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 6/11/13.




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
                    QUERY_RESULT_ROW = 13,
                    QUERY_RESULT_SET = 14,
                    QUERY_RESULT = 15
                };
            }

        }
    }
}


#endif //HAZELCAST__MAP_RESPONSE_ID

