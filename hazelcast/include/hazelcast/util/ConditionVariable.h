//
// Created by sancar koyunlu on 31/03/14.
//


#ifndef HAZELCAST_ConditionVariable
#define HAZELCAST_ConditionVariable

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#include "hazelcast/util/Mutex.h"
#include <cassert>

namespace hazelcast {
    namespace util {
        class ConditionVariable {
        public:
            ConditionVariable() {
                InitializeConditionVariable(&condition);
            };

            ~ConditionVariable() {

            };

            void wait(Mutex &mutex) {
                bool success = SleepConditionVariableCS(&condition,  &(mutex.mutex), INFINITE);
				assert(success && "SleepConditionVariable");
            };

			bool waitFor(Mutex &mutex, long timeInMillis) {
                bool success = SleepConditionVariableCS(&condition,  &(mutex.mutex), timeInMillis);
				if(success){
					return true;
				}
				assert(ERROR_TIMEOUT != GetLastError());
				return false;
            };

            void notify() {
                WakeConditionVariable(&condition);
            };

            void notify_all() {
                WakeAllConditionVariable(&condition);
            };


        private:
            CONDITION_VARIABLE condition;

            ConditionVariable(const ConditionVariable &rhs);

            void operator = (const ConditionVariable &rhs);
        };
    }
}


#else

#include "hazelcast/util/Mutex.h"
#include <pthread.h>
#include <sys/errno.h>
#include <cassert>

namespace hazelcast {
    namespace util {
        class ConditionVariable {
        public:
            ConditionVariable() {
                int error = pthread_cond_init(&condition, NULL);
                assert(EAGAIN != error);
                assert(ENOMEM != error);
                assert(EBUSY != error);
                assert(EINVAL != error);
            };

            ~ConditionVariable() {
                int error = pthread_cond_destroy(&condition);
                assert(EBUSY != error);
                assert(EINVAL != error);
            };

            void wait(Mutex &mutex) {
                int err = pthread_cond_wait(&condition,  &(mutex.mutex));
                assert (EPERM != err);
                assert (EINVAL != err);
            };


            void notify() {
                int err = pthread_cond_signal(&condition);
                assert(EINVAL != err);
            };

            void notify_all() {
                int err = pthread_cond_broadcast(&condition);
                assert(EINVAL != err);
            };


        private:
            pthread_cond_t condition;

            ConditionVariable(const ConditionVariable &rhs);

            void operator = (const ConditionVariable &rhs);
        };
    }
}


#endif


#endif //HAZELCAST_ConditinVarible

