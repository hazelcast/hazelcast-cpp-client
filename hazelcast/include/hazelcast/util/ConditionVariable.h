//
// Created by sancar koyunlu on 31/03/14.
//


#ifndef HAZELCAST_ConditionVariable
#define HAZELCAST_ConditionVariable

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

namespace hazelcast {
    namespace util {
        class ConditionVariable {
        public:

        private:
        };
    }
}


#else

#include "hazelcast/util/Mutex.h"
#include <pthread.h>
#include <sys/errno.h>


namespace hazelcast {
    namespace util {
        class ConditionVariable {
        public:
            enum status {
                timeout, invalidInput, notOwner, ok
            };

            ConditionVariable() {
                pthread_cond_init(&condition, NULL);
            };

            ~ConditionVariable() {
                pthread_cond_destroy(&condition);
            };

            ConditionVariable::status wait(Mutex &mutex) {
                int err = pthread_cond_wait(&condition,  &(mutex.mutex));
                if (EPERM == err) {
                    return ConditionVariable::notOwner;
                }

                if (EINVAL == err) {
                    return ConditionVariable::invalidInput;
                }

                return ConditionVariable::ok;
            };


            void notify() {
                pthread_cond_signal(&condition);
            };

            void notify_all() {
                pthread_cond_broadcast(&condition);
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
