//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_THREAD
#define HAZELCAST_THREAD

#include <pthread.h>

namespace hazelcast {
    namespace util {
        class Thread {
        public:
            Thread(void *(*runnable)(void *));

            Thread(void *(*runnable)(void *), void *parameters);

            Thread(void *(*runnable)(void *), pthread_attr_t const *, void *parameters);

            virtual void start();

            virtual void join();

            virtual void detach();

            virtual ~Thread();

        private:
            pthread_t thread;

            void *(*runnable)(void *);

            void *parameters;

            pthread_attr_t const *attributes;

            Thread(const Thread&);

            Thread& operator = (const Thread&);
        };

    }
}


#endif //HAZELCAST_THREAD
