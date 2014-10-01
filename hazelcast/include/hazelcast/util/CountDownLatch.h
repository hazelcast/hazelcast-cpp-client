//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_CountDownLatch
#define HAZELCAST_CountDownLatch

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/AtomicInt.h"

namespace hazelcast {
    namespace util {
        class CountDownLatch {
        public:
            CountDownLatch(int count);

            void countDown();

            bool await(int seconds);

            void await();

        private:
            util::AtomicInt count;

        };
    }
}
#endif //HAZELCAST_CountDownLatch

