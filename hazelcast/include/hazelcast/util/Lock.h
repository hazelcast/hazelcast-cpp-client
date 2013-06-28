//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_LOCK
#define HAZELCAST_LOCK

#include <pthread.h>

namespace hazelcast {
    namespace util {
        class Lock {
        public:
            Lock();

            ~Lock();

            void lock();

            void unlock();

        private:
            pthread_mutex_t mutex;

            Lock(const Lock& lock);

            Lock& operator = (const Lock&);
        };
    }
}


#endif //HAZELCAST_LOCK
