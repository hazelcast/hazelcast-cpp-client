//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ATOMIC_REFERENCE
#define HAZELCAST_ATOMIC_REFERENCE

#include <iosfwd>
#include "Lock.h"
#include "LockGuard.h"

namespace hazelcast {
    namespace util {
        template <typename T>
        class AtomicPointer {
        public:
            AtomicPointer():pointer(NULL) {
            };

            AtomicPointer(T *p) {
                util::LockGuard lg(mutex);
                pointer = p;
            };

            T *get() {
                util::LockGuard lg(mutex);
                return pointer;
            };

            T *set(T *p) {
                util::LockGuard lg(mutex);
                T *oldPointer = pointer;
                pointer = p;
                return oldPointer;
            };

        private:
            T *pointer;
            util::Lock mutex;
        };

    }
}


#endif //HAZELCAST_ATOMIC_REFERENCE
