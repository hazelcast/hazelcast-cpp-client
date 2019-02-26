/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/util/concurrent/locks/LockSupport.h"
#include "hazelcast/util/concurrent/BackoffIdleStrategy.h"
#include "hazelcast/util/concurrent/locks/LockSupport.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace util {
        namespace concurrent {
            BackoffIdleStrategy::BackoffIdleStrategy(int64_t maxSpins, int64_t maxYields, int64_t minParkPeriodNs,
                                                     int64_t maxParkPeriodNs) {
                Preconditions::checkNotNegative(maxSpins, "maxSpins must be positive or zero");
                Preconditions::checkNotNegative(maxYields, "maxYields must be positive or zero");
                Preconditions::checkNotNegative(minParkPeriodNs, "minParkPeriodNs must be positive or zero");
                Preconditions::checkNotNegative(maxParkPeriodNs - minParkPeriodNs,
                                                "maxParkPeriodNs must be greater than or equal to minParkPeriodNs");
                this->yieldThreshold = maxSpins;
                this->parkThreshold = maxSpins + maxYields;
                this->minParkPeriodNs = minParkPeriodNs;
                this->maxParkPeriodNs = maxParkPeriodNs;
                this->maxShift = Int64Util::numberOfLeadingZeros(minParkPeriodNs) -
                                 Int64Util::numberOfLeadingZeros(maxParkPeriodNs);

            }

            bool BackoffIdleStrategy::idle(int64_t n) {
                if (n < yieldThreshold) {
                    return false;
                }
                if (n < parkThreshold) {
                    Thread::yield();
                    return false;
                }
                int64_t time = parkTime(n);
                locks::LockSupport::parkNanos(time);
                return time == maxParkPeriodNs;
            }

            int64_t BackoffIdleStrategy::parkTime(int64_t n) const {
                const int64_t proposedShift = n - parkThreshold;
                const int64_t allowedShift = min<int64_t>(maxShift, proposedShift);
                return proposedShift > maxShift ? maxParkPeriodNs
                                                : proposedShift < maxShift ? minParkPeriodNs << allowedShift
                                                                           : min(minParkPeriodNs << allowedShift, maxParkPeriodNs);
            }
        }
    }
}
