/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 8/15/13.



#ifndef HAZELCAST_CountDownLatch
#define HAZELCAST_CountDownLatch

#include <vector>
#include <climits>
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
            static const size_t CHECK_INTERVAL = 100; //msecs
            static const size_t MILLISECONDS_IN_A_SECOND = 1000;

            CountDownLatch(int count);

            void countDown();

            bool await(int seconds);

            bool awaitMillis(size_t milliseconds);

            bool awaitMillis(size_t milliseconds, size_t &elapsed);

            void await();

            /**
             * Wait for latch count to drop to the provided value
             */
            bool await(int seconds, int count);

            int get();

        private:
            util::AtomicInt count;
            static const size_t HZ_INFINITE = UINT_MAX;
        };

        class HAZELCAST_API CountDownLatchWaiter {
        public:
            CountDownLatchWaiter &add(CountDownLatch &latch);

            bool awaitMillis(size_t milliseconds);

            void reset();
        private:
            std::vector<CountDownLatch *> latches;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_CountDownLatch

