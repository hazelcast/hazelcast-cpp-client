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




#ifndef HAZELCAST_SemaphorePortableHook
#define HAZELCAST_SemaphorePortableHook


namespace hazelcast {
    namespace client {
        namespace semaphore {

            namespace SemaphorePortableHook {
                enum {
                    F_ID = -16,

                    ACQUIRE = 1,
                    AVAILABLE = 2,
                    DRAIN = 3,
                    INIT = 4,
                    REDUCE = 5,
                    RELEASE = 6
                };
            };
        }
    }
}


#endif //HAZELCAST_SemaphorePortableHook

