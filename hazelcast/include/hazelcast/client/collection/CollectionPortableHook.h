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
                    LIST_ADD_ALL = 10,
                    LIST_GET = 11,
                    LIST_SET = 12,
                    LIST_ADD = 13,
                    LIST_REMOVE = 14,
                    LIST_INDEX_OF = 15,
                    LIST_SUB = 16,

                    TXN_LIST_ADD = 17,
                    TXN_LIST_REMOVE = 18,
                    TXN_LIST_SIZE = 19,

                    TXN_SET_ADD = 20,
                    TXN_SET_REMOVE = 21,
                    TXN_SET_SIZE = 22,
                    COLLECTION_REMOVE_LISTENER = 23
                };
            }

        }
    }
}


#endif //HAZELCAST_COLLECTION_PORTABLE_HOOK

