/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 31/03/14.
//

#ifndef HAZELCAST_ConditionVariable
#define HAZELCAST_ConditionVariable

#include <stdint.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        class Mutex;

        class HAZELCAST_API ConditionVariable {
        public:
            ConditionVariable();

            ~ConditionVariable();

            void wait(Mutex &mutex);

			bool waitFor(Mutex &mutex, int64_t timeInMilliseconds);

            void notify();

            void notify_all();

        private:
            CONDITION_VARIABLE condition;

            ConditionVariable(const ConditionVariable &rhs);

            void operator = (const ConditionVariable &rhs);
        };
    }
}

#else

#include <pthread.h>
#include <stdint.h>

namespace hazelcast {
    namespace util {

        class Mutex;

        class ConditionVariable {
        public:
            ConditionVariable();

            ~ConditionVariable();

            void wait(Mutex &mutex);

            bool waitFor(Mutex &mutex, int64_t timeInMilliseconds);

            void notify();

            void notify_all();

        private:
            pthread_cond_t condition;

            ConditionVariable(const ConditionVariable &rhs);

            void operator = (const ConditionVariable &rhs);
        };
    }
}


#endif


#endif //HAZELCAST_ConditinVarible

