/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include <stdint.h>

#include "hazelcast/util/concurrent/IdleStrategy.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        namespace concurrent {
            /**
             * Idling strategy for threads when they have no work to do.
             * <p/>
             * Spin for maxSpins, then
             * {@link Thread#yield()} for maxYields, then
             * {@link LockSupport#parkNanos(int64_t)} on an exponential backoff to maxParkPeriodNs
             */
            class HAZELCAST_API BackoffIdleStrategy : public IdleStrategy {
            public:
                /**
                 * Create a set of state tracking idle behavior
                 *
                 * @param maxSpins        to perform before moving to {@link Thread#yield()}
                 * @param maxYields       to perform before moving to {@link LockSupport#parkNanos(int64_t)}
                 * @param minParkPeriodNs to use when initiating parking
                 * @param maxParkPeriodNs to use when parking
                 */
                BackoffIdleStrategy(int64_t max_spins, int64_t max_yields, int64_t min_park_period_ns,
                                    int64_t max_park_period_ns);

                bool idle(int64_t n) override;

            private:
                int64_t park_time(int64_t n) const;

                static const int ARG_COUNT = 5;
                static const int ARG_MAX_SPINS = 1;
                static const int ARG_MAX_YIELDS = 2;
                static const int ARG_MIN_PARK_PERIOD = 3;
                static const int ARG_MAX_PARK_PERIOD = 4;

                int64_t yieldThreshold_;
                int64_t parkThreshold_;
                int64_t minParkPeriodNs_;
                int64_t maxParkPeriodNs_;
                int maxShift_;

            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


