//
// Created by sancar koyunlu on 31/03/14.
//


#ifndef HAZELCAST_ConditionVariable
#define HAZELCAST_ConditionVariable

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace hazelcast {
    namespace util {
        class Mutex;

        class ConditionVariable {
        public:
            ConditionVariable();

            ~ConditionVariable();

            void wait(Mutex &mutex);

			bool waitFor(Mutex &mutex, long timeInMillis);

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

namespace hazelcast {
    namespace util {

        class Mutex;

        class ConditionVariable {
        public:
            ConditionVariable();

            ~ConditionVariable();

            void wait(Mutex &mutex);

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

