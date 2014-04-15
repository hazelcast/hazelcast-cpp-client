//
// Created by sancar koyunlu on 11/04/14.
//

#include "hazelcast/util/Mutex.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#include <cassert>

namespace hazelcast {
    namespace util {

        Mutex::Mutex() {
            InitializeCriticalSection(&mutex);
        }

        Mutex::~Mutex() {
            DeleteCriticalSection(&mutex);
        }

        void Mutex::lock() {
            EnterCriticalSection(&mutex);
        }

        Mutex::status Mutex::tryLock() {
            BOOL success = TryEnterCriticalSection(&mutex);
            if (!success) {
                return Mutex::alreadyLocked;
            }
            return Mutex::ok;
        }

        void Mutex::unlock() {
            LeaveCriticalSection(&mutex);
        }
    }
}


#else

#include <cassert>
#include <sys/errno.h>

namespace hazelcast {
    namespace util {

        Mutex::Mutex() {
            pthread_mutex_init(&mutex, NULL);
        }

        Mutex::~Mutex() {
            pthread_mutex_destroy(&mutex);
        }

        void Mutex::lock() {
            int err = pthread_mutex_lock(&mutex);
            assert (!(err == EINVAL || err == EAGAIN));
            assert (err != EDEADLK);
        }

        Mutex::status Mutex::tryLock() {
            int err = pthread_mutex_trylock(&mutex);
            assert (!(err == EINVAL || err == EAGAIN));
            if (err == EBUSY) {
                return Mutex::alreadyLocked;
            }
            return Mutex::ok;
        }

        void Mutex::unlock() {
            int err = pthread_mutex_unlock(&mutex);
            assert (!(err == EINVAL || err == EAGAIN));
            assert (err != EPERM);
        }
    }
}


#endif

