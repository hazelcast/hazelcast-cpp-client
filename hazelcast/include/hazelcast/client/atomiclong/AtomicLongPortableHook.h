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




#ifndef HAZELCAST_AtomicLongPortableHook
#define HAZELCAST_AtomicLongPortableHook

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            namespace AtomicLongPortableHook {
                enum {
                    F_ID = -17,

                    ADD_AND_GET = 1,
                    COMPARE_AND_SET = 2,
                    GET_AND_ADD = 3,
                    GET_AND_SET = 4,
                    SET = 5,
                };
            };
        }
    }
}

#endif //HAZELCAST_AtomicLongPortableHook

