//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_CountDownLatch
#define HAZELCAST_CountDownLatch

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/AtomicInt.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace util {
        class HAZELCAST_API CountDownLatch {
        public:
            CountDownLatch(int count);

            void countDown();

            bool await(int seconds);

            void await();

            /**
             * Wait for latch count to drop to the provided value
             */
            bool await(int seconds, int count);

            int get();

        private:
            util::AtomicInt count;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_CountDownLatch

