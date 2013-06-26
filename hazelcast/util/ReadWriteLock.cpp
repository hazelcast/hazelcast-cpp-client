//
// Created by sancar koyunlu on 6/26/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ReadWriteLock.h"
#include "Util.h"
#include "../client/HazelcastException.h"
#include <sys/errno.h>

namespace hazelcast {
    namespace util {
        ReadWriteLock::ReadWriteLock() {
            pthread_rwlock_init(&lock, NULL);
        };

        ReadWriteLock::~ReadWriteLock() {
            pthread_rwlock_destroy(&lock);
        };


        bool ReadWriteLock::tryReadLock() {
            int err = pthread_rwlock_tryrdlock(&lock);
            if (err == 0)
                return true;
            if (err == EBUSY)
                return false;
            throw client::HazelcastException("ReadWriteLock::tryReadLock error no: " + to_string(err));
        };

        bool ReadWriteLock::tryWriteLock() {
            int err = pthread_rwlock_trywrlock(&lock);
            if (err == 0)
                return true;
            if (err == EBUSY)
                return false;
            throw client::HazelcastException("ReadWriteLock::tryWriteLock error no: " + to_string(err));
        };

        void ReadWriteLock::readLock() {
            int err = pthread_rwlock_rdlock(&lock);
            if (err)
                throw client::HazelcastException("ReadWriteLock::readLock error no: " + to_string(err));
        };

        void ReadWriteLock::writeLock() {
            int err = pthread_rwlock_rdlock(&lock);
            if (err)
                throw client::HazelcastException("ReadWriteLock::writeLock error no: " + to_string(err));
        };

        void ReadWriteLock::unlock() {
            int err = pthread_rwlock_unlock(&lock);
            if (err)
                throw client::HazelcastException("ReadWriteLock::unlock error no: " + to_string(err));
        };

        ReadWriteLock::ReadWriteLock(const ReadWriteLock& lock) {
            //private
        };

        ReadWriteLock& ReadWriteLock::operator = (ReadWriteLock const & param) {
            //private
            return *this;
        };


    }
}