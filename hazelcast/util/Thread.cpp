//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "Thread.h"


namespace hazelcast {
    namespace util {
        Thread::Thread(void *(*runnable)(void *)) {
            pthread_create(&thread, NULL, runnable, NULL);
        };

        Thread::Thread(void *(*runnable)(void *), void *param) {
            pthread_create(&thread, NULL, runnable, param);
        };

        Thread::Thread(void *(*runnable)(void *), pthread_attr_t const *attributes, void *param) {
            pthread_create(&thread, attributes, runnable, param);
        };


        void Thread::join() {
            pthread_join(thread, NULL);
        };

        Thread::~Thread() {

        };

        Thread::Thread(Thread const & param) {

        };

        Thread & Thread::operator = (Thread const & param) {
            return (*this);
        };


    }
}