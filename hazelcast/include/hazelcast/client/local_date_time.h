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
#pragma once

#include "hazelcast/util/export.h"
#include "hazelcast/client/local_date.h"
#include "hazelcast/client/local_time.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {

/**
 * A date-time without a time-zone in the ISO-8601 calendar system,
 * such as {@code 2007-12-03T10:15:30}.
 * <p>
 * {@code LocalDateTime} is an immutable date-time object that represents a
 * date-time, often viewed as year-month-day-hour-minute-second. Other date and
 * time fields, such as day-of-year, day-of-week and week-of-year, can also be
 * accessed. Time is represented to nanosecond precision. For example, the value
 * "2nd October 2007 at 13:45.30.123456789" can be stored in a {@code
 * LocalDateTime}.
 * <p>
 * This class does not store or represent a time-zone.
 * Instead, it is a description of the date, as used for birthdays, combined
 * with the local time as seen on a wall clock. It cannot represent an instant
 * on the time-line without additional information such as an offset or
 * time-zone.
 * <p>
 * The ISO-8601 calendar system is the modern civil calendar
 * system used today in most of the world. It is equivalent to the proleptic
 * Gregorian calendar system, in which today's rules for leap years are applied
 * for all time. For most applications written today, the ISO-8601 rules are
 * entirely suitable. However, any application that makes use of historical
 * dates, and requires them to be accurate will find the ISO-8601 approach
 * unsuitable.
 */
struct HAZELCAST_API local_date_time
{
    local_date date;
    local_time time;
};

bool HAZELCAST_API
operator==(const local_date_time& lhs, const local_date_time& rhs);

bool HAZELCAST_API
operator<(const local_date_time& lhs, const local_date_time& rhs);
} // namespace client
} // namespace hazelcast
namespace std {
template<>
struct HAZELCAST_API hash<hazelcast::client::local_date_time>
{
   std::size_t operator()(const hazelcast::client::local_date_time& f) const;
};
} // namespace std

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
