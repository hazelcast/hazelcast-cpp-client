//
// Created by sancar koyunlu on 9/3/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "LockSupport.h"


namespace hazelcast {
    namespace util {
        boost::mutex *LockSupport::getLock(long hash) {
            return &(pImpl::globalLocks[hash % pImpl::LOCKS_SIZE]);
        }
    }
}