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

bool HAZELCAST_API
operator<(const local_time& lhs, const local_time& rhs);
} // namespace client
} // namespace hazelcast
namespace std {
template<>
struct HAZELCAST_API hash<hazelcast::client::local_time>
{
    std::size_t operator()(const hazelcast::client::local_time& f) const;
};
} // namespace std

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
