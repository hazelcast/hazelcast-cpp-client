//
// Created by sancar koyunlu on 31/03/14.
//


#ifndef HAZELCAST_AtomicInt
#define HAZELCAST_AtomicInt

#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace util {
        class AtomicInt {
        public:
            AtomicInt(int v):v(v) {

            }

            int increment() {
                LockGuard lockGuard(mutex);
                return ++v;
            }

            int decrement() {
                LockGuard lockGuard(mutex);
                return --v;
            }

            bool isEqualto(int i) {
                LockGuard lockGuard(mutex);
                return i == v;
            }

        private:
            Mutex mutex;
            int v;

            AtomicInt(const AtomicInt &rhs);

            void operator = (const AtomicInt &rhs);
        };
    }
}

#endif //HAZELCAST_AtomicInt
