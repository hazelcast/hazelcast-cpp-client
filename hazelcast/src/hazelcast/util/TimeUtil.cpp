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

#include <boost/date_time/posix_time/posix_time.hpp>

#include "hazelcast/util/TimeUtil.h"
#include "hazelcast/util/concurrent/TimeUnit.h"

namespace hazelcast {
    namespace util {
        boost::posix_time::time_duration TimeUtil::getDurationSinceEpoch() {
            boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
            boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
            boost::posix_time::time_duration diff = now - epoch;
            return diff;
        }

        int64_t TimeUtil::timeInMsOrOneIfResultIsZero(int64_t time, const concurrent::TimeUnit &timeunit) {
            int64_t timeInMillis = timeunit.toMillis(time);
            if (time > 0 && timeInMillis == 0) {
                timeInMillis = 1;
            }

            return timeInMillis;
        }
    }
}
