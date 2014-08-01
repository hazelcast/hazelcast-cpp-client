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

        bool ConditionVariable::waitFor(Mutex &mutex, long timeInMillis) {
            BOOL interrupted = SleepConditionVariableCS(&condition,  &(mutex.mutex), timeInMillis);
            if(!interrupted){
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

        void ConditionVariable::wait(Mutex &mutex) {
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

