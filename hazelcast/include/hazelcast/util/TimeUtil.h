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
#include <cstdint>
#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        namespace concurrent {
            class TimeUnit;
        }
        class HAZELCAST_API TimeUtil {
        public:
            /**
             * Convert time to milliseconds based on the input time-unit.
             * If conversion results in the value 0, then it the value is replaced with the positive 1.
             *
             * @param time The input time
             * @param timeunit The input time-unit to base the conversion on
             *
             * @return The milliseconds representation of the input time
             */
            static int64_t timeInMsOrOneIfResultIsZero(int64_t time, const concurrent::TimeUnit &timeunit);

        };
    }
}

