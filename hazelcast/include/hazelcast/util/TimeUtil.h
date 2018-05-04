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

#ifndef HAZELCAST_UTIL_TIMEUTIL_H_
#define HAZELCAST_UTIL_TIMEUTIL_H_

#include <boost/date_time/posix_time/ptime.hpp>

namespace hazelcast {
    namespace util {
        class TimeUtil {
        public:
            /**
             * @return the difference, measured in milliseconds, between the current time and midnight, January 1, 1970 UTC.
             */
            static boost::posix_time::time_duration getDurationSinceEpoch();
        };
    }
}

#endif //HAZELCAST_UTIL_TIMEUTIL_H_
