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
// Created by sancar koyunlu on 31/03/14.
//

#include "hazelcast/util/ConditionVariable.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#include "hazelcast/util/Mutex.h"
#include <cassert>

namespace hazelcast {
    namespace util {
        ConditionVariable::ConditionVariable() {
            InitializeConditionVariable(&condition);
        }

        ConditionVariable::~ConditionVariable() {

        }

        void ConditionVariable::wait(Mutex &mutex) {
            BOOL success = SleepConditionVariableCS(&condition,  &(mutex.mutex), INFINITE);
            assert(success && "SleepConditionVariable");
        }

        bool ConditionVariable::waitFor(Mutex &mutex, time_t timeSec) {
            BOOL interrupted = SleepConditionVariableCS(&condition,  &(mutex.mutex), (DWORD)timeSec * 1000);
            if(interrupted){
                return true;
            }
            return false;
        }

        void ConditionVariable::notify() {
            WakeConditionVariable(&condition);
        }

        void ConditionVariable::notify_all() {
            WakeAllConditionVariable(&condition);
        }
    }
}


#else

#include "hazelcast/util/Mutex.h"
#include <sys/errno.h>
#include <cassert>
#include <sys/time.h>

namespace hazelcast {
    namespace util {
        ConditionVariable::ConditionVariable() {
            int error = pthread_cond_init(&condition, NULL);
            (void)error;
            assert(EAGAIN != error);
            assert(ENOMEM != error);
            assert(EBUSY != error);
            assert(EINVAL != error);
        }

        ConditionVariable::~ConditionVariable() {
            int error = pthread_cond_destroy(&condition);
            (void)error;
            assert(EBUSY != error);
            assert(EINVAL != error);
        }

        bool ConditionVariable::waitFor(Mutex& mutex, time_t timeInSec) {
            struct timeval tv;
            ::gettimeofday(&tv, NULL);

            struct timespec ts;
            ts.tv_sec = tv.tv_sec;
            ts.tv_nsec = tv.tv_usec * 1000;
            ts.tv_sec += timeInSec;


            int error = pthread_cond_timedwait(&condition, &(mutex.mutex), &ts);
            (void)error;
            assert(EPERM != error);
            assert(EINVAL != error);

            if (ETIMEDOUT == error) {
                return false;
            }

            return true;
        }

        void ConditionVariable::wait(Mutex& mutex) {
            int error = pthread_cond_wait(&condition, &(mutex.mutex));
            (void)error;
            assert (EPERM != error);
            assert (EINVAL != error);
        }


        void ConditionVariable::notify() {
            int error = pthread_cond_signal(&condition);
            (void)error;
            assert(EINVAL != error);
        }

        void ConditionVariable::notify_all() {
            int error = pthread_cond_broadcast(&condition);
            (void)error;
            assert(EINVAL != error);
        }
    }
}


#endif

