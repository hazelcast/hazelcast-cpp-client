/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/temporals.h"

namespace hazelcast {
namespace client {

bool
operator==(const local_time& lhs, const local_time& rhs)
{
    return lhs.hours == rhs.hours && lhs.minutes == rhs.minutes &&
           lhs.seconds == rhs.seconds && lhs.nanos == rhs.nanos;
}

bool
operator==(const local_date& lhs, const local_date& rhs)
{
    return lhs.year == rhs.year && lhs.month == rhs.month &&
           lhs.day_of_month == rhs.day_of_month;
}

bool
operator==(const local_date_time& lhs, const local_date_time& rhs)
{
    return lhs.date == rhs.date && lhs.time == rhs.time;
}

bool
operator==(const offset_date_time& lhs, const offset_date_time& rhs)
{
    return lhs.date_time == rhs.date_time &&
           lhs.zone_offset_in_seconds == rhs.zone_offset_in_seconds;
}

} // namespace client
} // namespace hazelcast
