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

        bool ConditionVariable::waitFor(Mutex &mutex, int64_t timeInMilliseconds) {
            BOOL interrupted = SleepConditionVariableCS(&condition,  &(mutex.mutex), (DWORD) timeInMilliseconds);
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
#include <limits>

namespace hazelcast {
    namespace util {
        #define NANOS_IN_A_SECOND 1000 * 1000 * 1000
        #define MILLIS_IN_A_SECOND 1000
        #define NANOS_IN_A_MILLISECOND 1000 * 1000
        #define NANOS_IN_A_USECOND 1000

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

        bool ConditionVariable::waitNanos(Mutex& mutex, int64_t nanos) {
            struct timespec ts = calculateTimeFromNanos(nanos);

            int error = pthread_cond_timedwait(&condition, &(mutex.mutex), &ts);
            (void)error;
            assert(EPERM != error);
            assert(EINVAL != error);

            if (ETIMEDOUT == error) {
                return false;
            }

            return true;
        }

        bool ConditionVariable::waitFor(Mutex& mutex, int64_t timeInMilliseconds) {
            struct timespec ts = calculateTimeFromMilliseconds(timeInMilliseconds);

            int error = pthread_cond_timedwait(&condition, &(mutex.mutex), &ts);
            (void)error;
            assert(EPERM != error);
            assert(EINVAL != error);

            if (ETIMEDOUT == error) {
                return false;
            }

            return true;
        }

        struct timespec ConditionVariable::calculateTimeFromMilliseconds(int64_t timeInMilliseconds) const {
            struct timeval tv;
            gettimeofday(&tv, NULL);

            struct timespec ts;
            ts.tv_sec = tv.tv_sec;
            ts.tv_nsec = tv.tv_usec * 1000;
            int64_t seconds = timeInMilliseconds / 1000;
            if (seconds > std::numeric_limits<time_t>::max()) {
                ts.tv_sec = std::numeric_limits<time_t>::max();
            } else {
                ts.tv_sec += (time_t) (timeInMilliseconds / MILLIS_IN_A_SECOND);
                long nsec = tv.tv_usec * NANOS_IN_A_USECOND + (timeInMilliseconds % 1000) * NANOS_IN_A_MILLISECOND;
                if (nsec >= NANOS_IN_A_SECOND) {
                    nsec -= NANOS_IN_A_SECOND;
                    ++ts.tv_sec;
                }
                ts.tv_nsec = nsec;
            }
            return ts;
        }

        struct timespec ConditionVariable::calculateTimeFromNanos(int64_t nanos) const {
            struct timeval tv;
            gettimeofday(&tv, NULL);

            struct timespec ts;
            ts.tv_sec = tv.tv_sec;
            ts.tv_nsec = tv.tv_usec * 1000;
            int64_t seconds = nanos / NANOS_IN_A_SECOND;
            if (seconds > std::numeric_limits<time_t>::max()) {
                ts.tv_sec = std::numeric_limits<time_t>::max();
            } else {
                ts.tv_sec += (time_t) (nanos / NANOS_IN_A_SECOND);
                long nsec = tv.tv_usec * NANOS_IN_A_USECOND + (nanos % 1000);
                if (nsec >= NANOS_IN_A_SECOND) {
                    nsec -= NANOS_IN_A_SECOND;
                    ++ts.tv_sec;
                }
                ts.tv_nsec = nsec;
            }
            return ts;
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

