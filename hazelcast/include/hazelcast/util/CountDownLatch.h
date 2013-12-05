//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_CountDownLatch
#define HAZELCAST_CountDownLatch

#include "hazelcast/util/HazelcastDll.h"
#include <boost/atomic/atomic.hpp>
#include <boost/thread.hpp>

namespace hazelcast {
    namespace util {
        class HAZELCAST_API CountDownLatch {
        public:
            CountDownLatch(int count);

            void countDown();

            bool await(long timeInMillis);

            void await();

        private:
            boost::atomic<int> count;
            boost::condition_variable conditionVariable;
            boost::mutex mutex;

        };
    }
}
#endif //HAZELCAST_CountDownLatch
