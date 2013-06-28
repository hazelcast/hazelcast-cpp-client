//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/HazelcastException.h"
#include "Util.h"
#include "Lock.h"

namespace hazelcast {
    namespace util {
        Lock::Lock() {
            pthread_mutex_init(&mutex, NULL);
        };

        Lock::~Lock() {
            pthread_mutex_destroy(&mutex);
        };

        void Lock::lock() {
            int err = pthread_mutex_lock(&mutex);
            if (err) {
                throw hazelcast::client::HazelcastException("Lock::lock error no: " + to_string(err));
            }
        };

        void Lock::unlock() {
            int err = pthread_mutex_unlock(&mutex);
            if (err) {
                throw hazelcast::client::HazelcastException("Lock:unLock error no: " + to_string(err));
            }
        };

        Lock::Lock(Lock const & lock) {

        };

        Lock & Lock::operator = (Lock const & param) {
            return *this;
        };


    }
}