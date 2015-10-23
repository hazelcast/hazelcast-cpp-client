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




#ifndef HAZELCAST_MAP_PortableHook
#define HAZELCAST_MAP_PortableHook

namespace hazelcast {
    namespace client {
        namespace map {
            namespace PortableHook {
                enum {
                    F_ID = -10,
                    GET = 1,
                    PUT = 2,
                    PUT_IF_ABSENT = 3,
                    TRY_PUT = 4,
                    PUT_TRANSIENT = 5,
                    SET = 6,
                    CONTAINS_KEY = 7,
                    CONTAINS_VALUE = 8,
                    REMOVE = 9,
                    REMOVE_IF_SAME = 10,
                    DELETE_R = 11,
                    FLUSH = 12,
                    GET_ALL = 13,
                    TRY_REMOVE = 14,
                    REPLACE = 15,
                    REPLACE_IF_SAME = 16,
                    LOCK = 17,
                    IS_LOCKED = 18,
                    UNLOCK = 20,
                    EVICT = 21,
                    ADD_INTERCEPTOR = 23,
                    REMOVE_INTERCEPTOR = 24,
                    ADD_ENTRY_LISTENER = 25,
                    ADD_ENTRY_LISTENER_SQL = 26,
                    GET_ENTRY_VIEW = 27,
                    ADD_INDEX = 28,
                    KEY_SET = 29,
                    VALUES = 30,
                    ENTRY_SET = 31,
                    SIZE = 33,
                    QUERY = 34,
                    SQL_QUERY = 35,
                    CLEAR = 36,
                    GET_LOCAL_MAP_STATS = 37,
                    EXECUTE_ON_KEY = 38,
                    EXECUTE_ON_ALL_KEYS = 39,
                    PUT_ALL = 40,
                    TXN_REQUEST = 41,
                    TXN_REQUEST_WITH_SQL_QUERY = 42,
                    EXECUTE_WITH_PREDICATE = 43,
                    REMOVE_ENTRY_LISTENER = 44,
                    EXECUTE_ON_KEYS = 45,
                    EVICT_ALL = 46,
                    LOAD_ALL_GIVEN_KEYS = 47,
                    LOAD_ALL_KEYS = 48,
                    IS_EMPTY = 49
                };
            }

        }
    }
}


#endif //HAZELCAST__MAP_REQUEST_ID

