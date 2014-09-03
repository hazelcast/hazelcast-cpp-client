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

        bool ConditionVariable::waitFor(Mutex &mutex, int timeSec) {
            BOOL interrupted = SleepConditionVariableCS(&condition,  &(mutex.mutex), timeSec * 1000);
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
            assert(EAGAIN != error);
            assert(ENOMEM != error);
            assert(EBUSY != error);
            assert(EINVAL != error);
        }

        ConditionVariable::~ConditionVariable() {
            int error = pthread_cond_destroy(&condition);
            assert(EBUSY != error);
            assert(EINVAL != error);
        }

        bool ConditionVariable::waitFor(Mutex& mutex, int timeInSec) {
            struct timeval tv;
            ::gettimeofday(&tv, NULL);

            struct timespec ts;
            ts.tv_sec = tv.tv_sec;
            ts.tv_nsec = tv.tv_usec * 1000;
            ts.tv_sec += timeInSec;


            int error = pthread_cond_timedwait(&condition, &(mutex.mutex), &ts);
            assert(EPERM != error);
            assert(EINVAL != error);

            if (ETIMEDOUT == error) {
                return false;
            }

            return true;
        }

        void ConditionVariable::wait(Mutex& mutex) {
            int err = pthread_cond_wait(&condition, &(mutex.mutex));
            assert (EPERM != err);
            assert (EINVAL != err);
        }


        void ConditionVariable::notify() {
            int err = pthread_cond_signal(&condition);
            assert(EINVAL != err);
        }

        void ConditionVariable::notify_all() {
            int err = pthread_cond_broadcast(&condition);
            assert(EINVAL != err);
        }
    }
}


#endif

