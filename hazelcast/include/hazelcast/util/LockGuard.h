//
// Created by sancar koyunlu on 31/03/14.
//


#ifndef HAZELCAST_LockGuard
#define HAZELCAST_LockGuard

#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        class HAZELCAST_API LockGuard {
        public:
            LockGuard(Mutex &mutex)
            :mutex(mutex) {
                mutex.lock();
            };

            ~LockGuard() {
                mutex.unlock();
            };

        private:
            Mutex &mutex;
        };
    }
}

#endif //HAZELCAST_LockGuard

