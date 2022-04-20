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
#include <cstdint>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
/**
 * A time without a time-zone in the ISO-8601 calendar system,
 * such as {@code 10:15:30}.
 * <p>
 * {@code local_time} is an immutable date-time object that represents a time,
 * often viewed as hour-minute-second.
 * Time is represented to nanosecond precision.
 * For example, the value "13:45.30.123456789" can be stored in a {@code
 * local_time}.
 * <p>
 * This class does not store or represent a date or time-zone.
 * Instead, it is a description of the local time as seen on a wall clock.
 * It cannot represent an instant on the time-line without additional
 * information such as an offset or time-zone.
 * <p>
 * The ISO-8601 calendar system
 * is the modern civil calendar system used today in most of the world. This API
 * assumes that all calendar systems use the same representation, this class,
 * for time-of-day.
 */
struct HAZELCAST_API local_time
{
    /**
     * the hour-of-day to represent, from 0 to 23
     */
    uint8_t hours;
    /**
     * the minute-of-hour to represent, from 0 to 59
     */
    uint8_t minutes;
    /**
     * the second-of-minute to represent, from 0 to 59
     */
    uint8_t seconds;
    /**
     * the nanosecond-of-second to represent, from 0 to 999,999,999
     */
    int32_t nanos;
};

bool HAZELCAST_API
operator==(const local_time& lhs, const local_time& rhs);

/**
 *  A date without a time-zone in the ISO-8601 calendar system,
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

/**
 * * A date-time without a time-zone in the ISO-8601 calendar system,
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

/**
 * A date-time with an offset from UTC/Greenwich in the ISO-8601 calendar
 * system, such as {@code 2007-12-03T10:15:30+01:00}.
 * <p>
 * {@code offset_date_time} is an immutable representation of a date-time with
 * an offset. This class stores all date and time fields, to a precision of
 * nanoseconds, as well as the offset from UTC/Greenwich. For example, the value
 * "2nd October 2007 at 13:45:30.123456789 +02:00" can be stored in an {@code
 * offset_date_time}.
 *
 */
struct HAZELCAST_API offset_date_time
{
    /**
     * The local date-time.
     */
    local_date_time date_time;
    /**
     * The offset from UTC/Greenwich.
     * The offset must be in the range {@code -18:00} to {@code +18:00}, which
     * corresponds to -64800 to +64800 seconds.
     */
    int32_t zone_offset_in_seconds;
};

bool HAZELCAST_API
operator==(const offset_date_time& lhs, const offset_date_time& rhs);

} // namespace client
} // namespace hazelcast
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
