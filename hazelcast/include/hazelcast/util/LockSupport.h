//
// Created by sancar koyunlu on 9/3/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_LockSupport
#define HAZELCAST_LockSupport

#include <boost/thread/recursive_mutex.hpp>

namespace hazelcast {
    namespace util {

        namespace pImpl {
            static const long LOCKS_SIZE = 1024;
            static boost::recursive_mutex globalLocks[LOCKS_SIZE];
        }

        class LockSupport {
        public:
            static boost::recursive_mutex *getLock(long hash);
        };
    }
}

#endif //HAZELCAST_LockSupport
