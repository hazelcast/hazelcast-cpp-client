/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_Mutex
#define HAZELCAST_Mutex


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        class HAZELCAST_API Mutex {
        public:
			enum status {
                alreadyLocked, ok
            };
            
            Mutex();

            ~Mutex() ;

            void lock();

            Mutex::status tryLock();

            void unlock();

            CRITICAL_SECTION mutex;
        private:
            Mutex(const Mutex &rhs);

            void operator = (const Mutex &rhs);
        };
    }
}


#else

#include <pthread.h>

namespace hazelcast {
    namespace util {

        class Mutex {
        public:

            Mutex();

            ~Mutex();

            void lock();

            void unlock();

            pthread_mutex_t mutex;
        private:
            Mutex(const Mutex &rhs);

            void operator = (const Mutex &rhs);
        };
    }
}


#endif


#endif //HAZELCAST_Mutex


