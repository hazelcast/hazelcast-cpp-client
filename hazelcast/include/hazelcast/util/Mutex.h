//
// Created by sancar koyunlu on 31/03/14.
//


#ifndef HAZELCAST_Mutex
#define HAZELCAST_Mutex


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#include <Windows.h>

namespace hazelcast {
    namespace util {
        class Mutex {
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
            enum status {
                alreadyLocked, ok
            };

            Mutex();

            ~Mutex();

            void lock();

            Mutex::status tryLock();

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


