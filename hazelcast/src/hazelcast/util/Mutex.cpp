/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 11/04/14.
//

#include "hazelcast/util/Mutex.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#include <cassert>

namespace hazelcast {
    namespace util {

        Mutex::Mutex() {
            InitializeCriticalSection(&mutex);
        }

        Mutex::~Mutex() {
            DeleteCriticalSection(&mutex);
        }

        void Mutex::lock() {
			EnterCriticalSection(&mutex);
        }

        Mutex::status Mutex::tryLock() {
            BOOL success = TryEnterCriticalSection(&mutex);
            if (!success) {
                return Mutex::alreadyLocked;
            }
            return Mutex::ok;
        }

        void Mutex::unlock() {
            LeaveCriticalSection(&mutex);
        }
    }
}


#else

#include <cassert>
#include <sys/errno.h>

namespace hazelcast {
    namespace util {

        Mutex::Mutex() {
            pthread_mutex_init(&mutex, NULL);
        }

        Mutex::~Mutex() {
            pthread_mutex_destroy(&mutex);
        }

        void Mutex::lock() {
            int error = pthread_mutex_lock(&mutex);
            (void)error;
            assert (!(error == EINVAL || error == EAGAIN));
            assert (error != EDEADLK);
        }

        void Mutex::unlock() {
            int error = pthread_mutex_unlock(&mutex);
            (void)error;
            assert (!(error == EINVAL || error == EAGAIN));
            assert (error != EPERM);
        }
    }
}


#endif

