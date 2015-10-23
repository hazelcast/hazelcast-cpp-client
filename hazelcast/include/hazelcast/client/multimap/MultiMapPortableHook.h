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
// Created by sancar koyunlu on 9/10/13.




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
                    REMOVE_ENTRY_LISTENER = 24,
                    TXN_MM_REMOVEALL = 25,
                    KEY_BASED_CONTAINS = 26,
                };
            }

        }
    }
}

#endif //HAZELCAST_MultiMapPortableHook

