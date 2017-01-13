/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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



#include "hazelcast/util/Util.h"
#include "hazelcast/util/CountDownLatch.h"
#include <time.h>
namespace hazelcast {
    namespace util {
        CountDownLatch::CountDownLatch(int count)
        : count(count) {

        }

        void CountDownLatch::countDown() {
            --count;
        }

        bool CountDownLatch::await(int seconds) {
            return awaitMillis(seconds * MILLISECONDS_IN_A_SECOND);
        }

        bool CountDownLatch::awaitMillis(size_t milliseconds) {
            size_t elapsed;
            return awaitMillis(milliseconds, elapsed);
        }

        bool CountDownLatch::awaitMillis(size_t milliseconds, size_t &elapsed) {
            // set elapsed to zero in case it returns before sleep
            elapsed = 0;

            if (count <= 0) {
                return true;
            }

            do {
                util::sleepmillis(CHECK_INTERVAL);
                elapsed += CHECK_INTERVAL;
                if (count <= 0) {
                    return true;
                }
            } while (elapsed < milliseconds);

            return false;
        }

        void CountDownLatch::await() {
            awaitMillis(HZ_INFINITE);
        }

        int CountDownLatch::get() {
            return count;
        }

        bool CountDownLatch::await(int seconds, int expectedCount) {
            while (seconds > 0 && count > expectedCount) {
                util::sleep(1);
                --seconds;
            }
            return count <= expectedCount;
        }

        CountDownLatchWaiter &CountDownLatchWaiter::add(CountDownLatch &latch) {
            latches.push_back(&latch);
            return *this;
        }

        bool CountDownLatchWaiter::awaitMillis(size_t milliseconds) {
            if (latches.empty()) {
                return true;
            }

            for (std::vector<util::CountDownLatch *>::const_iterator it = latches.begin();it != latches.end();++it) {
                size_t elapsed;
                bool result = (*it)->awaitMillis(milliseconds, elapsed);
                if (!result) {
                    return false;
                }
                milliseconds -= elapsed;
                if (milliseconds <= 0) {
                    return false;
                }
            }
            return true;
        }

        void CountDownLatchWaiter::reset() {
            latches.clear();
        }

    }
}

