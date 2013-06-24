//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "LockGuard.h"
#include "Lock.h"

namespace hazelcast {
    namespace util {
        LockGuard::LockGuard(util::Lock& lock):lock(lock) {
            lock.lock();
        }


        LockGuard::~LockGuard() {
            lock.unlock();
        }

        LockGuard::LockGuard(LockGuard const & rhs):lock(rhs.lock) {
            //private
        }

        void LockGuard::operator = (LockGuard const & rhs) {
            //private
        }


    }
}