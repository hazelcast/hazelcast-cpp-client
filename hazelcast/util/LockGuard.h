//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_LOCK_GUARD
#define HAZELCAST_LOCK_GUARD

namespace hazelcast {
    namespace util {
        class Lock;

        class LockGuard {
        public:
            LockGuard(util::Lock& lock);

            ~LockGuard();

        private:
            LockGuard(const LockGuard&);

            void operator = (const LockGuard&);

            Lock& lock;
        };
    }
}

#endif //HAZELCAST_LOCK_GUARD
