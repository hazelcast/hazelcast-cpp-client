/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 04/08/14.
//
#ifndef HAZELCAST_UTIL_CLOSABLE_H_
#define HAZELCAST_UTIL_CLOSABLE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        class HAZELCAST_API Closeable {
        public:
            virtual ~Closeable();

            virtual void close(const char *closeReason) = 0;
        };
    }
}
#endif //HAZELCAST_UTIL_CLOSABLE_H_
