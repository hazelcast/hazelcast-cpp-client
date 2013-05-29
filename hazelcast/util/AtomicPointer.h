//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ATOMIC_REFERENCE
#define HAZELCAST_ATOMIC_REFERENCE

#include <iosfwd>

namespace hazelcast {
    namespace util {
        template <typename T>
        class AtomicPointer {
        public:
            AtomicPointer() {
                __sync_lock_test_and_set(&pointer, NULL);
            };

            AtomicPointer(T *p) {
                __sync_lock_test_and_set(&pointer, p);
            };

            T *get() {
                return __sync_fetch_and_add(&pointer, 0);
            };

            void *set(T *p) {
                __sync_lock_test_and_set(&pointer, p);
            };

        private:
            T *pointer;
        };

    }
}


#endif //HAZELCAST_ATOMIC_REFERENCE
