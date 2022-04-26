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

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
/**
 * A date without a time-zone in the ISO-8601 calendar system,
 * such as {@code 2007-12-03}.
 * <p>
 * {@code local_date} is an immutable date-time object that represents a date,
 * often viewed as year-month-day. Other date fields, such as day-of-year,
 * day-of-week and week-of-year, can also be accessed.
 * For example, the value "2nd October 2007" can be stored in a {@code
 * local_date}.
 * <p>
 * This class does not store or represent a time or time-zone.
 * Instead, it is a description of the date, as used for birthdays.
 * It cannot represent an instant on the time-line without additional
 * information such as an offset or time-zone.
 *
 * The ISO-8601 calendar system is the modern civil calendar system used today
 * in most of the world. It is equivalent to the proleptic Gregorian calendar
 * system, in which today's rules for leap years are applied for all time.
 * For most applications written today, the ISO-8601 rules are entirely
 * suitable. However, any application that makes use of historical dates, and
 * requires them to be accurate will find the ISO-8601 approach unsuitable.
 *
 * The minimum supported {@code local_date}, '-999999999-01-01'.
 * This could be used by an application as a "far past" date.
 *
 * The maximum supported {@code LocalDate}, '+999999999-12-31'.
 * This could be used by an application as a "far future" date.
 */
struct HAZELCAST_API local_date
{
    /**
     * minimum value is -999999999
     * maximum value is 999999999
     */
    int32_t year;
    /**
     * minimum value is 1
     * maximum value is 12
     */
    uint8_t month;
    /**
     * minimum value is 1
     * maximum value is 31
     */
    uint8_t day_of_month;
};

bool HAZELCAST_API
operator==(const local_date& lhs, const local_date& rhs);

bool HAZELCAST_API
operator<(const local_date& lhs, const local_date& rhs);
} // namespace client
} // namespace hazelcast
namespace std {
template<>
struct HAZELCAST_API hash<hazelcast::client::local_date>
{
    std::size_t operator()(const hazelcast::client::local_date& f) const;
};
} // namespace std

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
