//
// Created by sancar koyunlu on 9/3/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/util/LockSupport.h"


namespace hazelcast {
    namespace util {
        boost::recursive_mutex *LockSupport::getLock(long hash) {
            return &(pImpl::globalLocks[abs(hash) % pImpl::LOCKS_SIZE]);
        }
    }
}