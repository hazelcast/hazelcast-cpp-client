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


namespace hazelcast {
    namespace util {

        class Mutex {
        public:
            enum status {
                alreadyLocked, notOwner, invalidState, ok
            };
            
            Mutex() {
                pthread_mutex_init(&mutex, NULL);
            }

            ~Mutex() {
                pthread_mutex_destroy(&mutex);
            }

            Mutex::status lock() {
                int err = pthread_mutex_lock(&mutex);
                if (err == EINVAL && err == EAGAIN) {
                    return Mutex::invalidState;
                }
                if (err == EDEADLK) {
                    return Mutex::alreadyLocked;
                }
                return Mutex::ok;
            }

            Mutex::status tryLock() {
                int err = pthread_mutex_trylock(&mutex);
                if (err == EINVAL && err == EAGAIN) {
                    return Mutex::invalidState;
                }
                if (err == EBUSY) {
                    return Mutex::alreadyLocked;
                }
                return Mutex::ok;
            }

            Mutex::status unlock() {
                int err = pthread_mutex_unlock(&mutex);
                if (err == EINVAL && err == EAGAIN) {
                    return Mutex::invalidState;
                }
                if (err == EPERM) {
                    return Mutex::notOwner;
                }
                return Mutex::ok;
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
