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
// Created by sancar koyunlu on 6/25/13.




#ifndef HAZELCAST_LockPortableHook
#define HAZELCAST_LockPortableHook

namespace hazelcast {
    namespace client {
        namespace lock {

            namespace LockPortableHook {
                enum {
                    FACTORY_ID = -15,

                    LOCK = 1,
                    UNLOCK = 2,
                    IS_LOCKED = 3,
                    GET_LOCK_COUNT = 5,
                    GET_REMAINING_LEASE = 6
                };
            };
        }
    }
}


#endif //HAZELCAST_LockPortableHook

