//
// Created by sancar koyunlu on 6/26/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ReadWriteLock
#define HAZELCAST_ReadWriteLock

#include <pthread.h>

namespace hazelcast {
    namespace util {
        class ReadWriteLock {
        public:
            ReadWriteLock();

            bool tryReadLock();

            bool tryWriteLock();

            void readLock();

            void writeLock();

            void unlock();

            ~ReadWriteLock();

        private:
            pthread_rwlock_t lock;

            ReadWriteLock(const ReadWriteLock& lock);

            ReadWriteLock& operator = (const ReadWriteLock&);
        };

    }
}
#endif //HAZELCAST_ReadWriteLock
