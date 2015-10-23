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
// Created by sancar koyunlu on 6/11/13.




#ifndef HAZELCAST_QUEUE_PORTABLE_HOOK
#define HAZELCAST_QUEUE_PORTABLE_HOOK

namespace hazelcast {
    namespace client {
        namespace queue {
            namespace QueuePortableHook {
                enum {
                    F_ID = -11,
                    OFFER = 1,
                    SIZE = 2,
                    REMOVE = 3,
                    POLL = 4,
                    PEEK = 5,
                    ITERATOR = 6,
                    DRAIN = 7,
                    CONTAINS = 8,
                    COMPARE_AND_REMOVE = 9,
                    CLEAR = 10,
                    ADD_ALL = 11,
                    ADD_LISTENER = 12,
                    REMAINING_CAPACITY = 13,
                    TXN_OFFER = 14,
                    TXN_POLL = 15,
                    TXN_SIZE = 16,
                    TXN_PEEK = 17,
                    REMOVE_LISTENER = 18
                };
            }

        }
    }
}


#endif //HAZELCAST__MAP_REQUEST_ID

