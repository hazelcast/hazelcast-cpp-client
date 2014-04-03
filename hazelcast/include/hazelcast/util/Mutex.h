//
// Created by sancar koyunlu on 31/03/14.
//


#ifndef HAZELCAST_Mutex
#define HAZELCAST_Mutex


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

namespace hazelcast {
    namespace util {
        class Mutex {
        public:

        private:
        };
    }
}


#else

#include <pthread.h>
#include <sys/errno.h>
#include <cassert>


namespace hazelcast {
    namespace util {

        class Mutex {
        public:
            enum status {
                alreadyLocked, ok
            };
            
            Mutex() {
                pthread_mutex_init(&mutex, NULL);
            }

            ~Mutex() {
                pthread_mutex_destroy(&mutex);
            }

            void lock() {
                int err = pthread_mutex_lock(&mutex);
                assert (!(err == EINVAL || err == EAGAIN));
                assert (err != EDEADLK);
            }

            Mutex::status tryLock() {
                int err = pthread_mutex_trylock(&mutex);
                assert (!(err == EINVAL || err == EAGAIN));
                if (err == EBUSY) {
                    return Mutex::alreadyLocked;
                }
                return Mutex::ok;
            }

            void unlock() {
                int err = pthread_mutex_unlock(&mutex);
                assert (!(err == EINVAL || err == EAGAIN));
                assert (err != EPERM);
            }

            pthread_mutex_t mutex;
        private:
            Mutex(const Mutex &rhs);

            void operator = (const Mutex &rhs);
        };
    }
}


#endif


#endif //HAZELCAST_Mutex
