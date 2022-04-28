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
#include "hazelcast/client/local_date_time.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
/**
 * A date-time with an offset from UTC/Greenwich in the ISO-8601 calendar
 * system, such as {@code 2007-12-03T10:15:30+01:00}.
 * <p>
 * {@code offset_date_time} is an immutable representation of a date-time with
 * an offset. This class stores all date and time fields, to a precision of
 * nanoseconds, as well as the offset from UTC/Greenwich. For example, the value
 * "2nd October 2007 at 13:45:30.123456789 +02:00" can be stored in an {@code
 * offset_date_time}.
 * <p>
 * The minimum supported {@code offset_date_time},
 * '-999999999-01-01T00:00:00+18:00'. This is the local date-time of midnight at
 * the start of the minimum date in the maximum offset (larger offsets are
 * earlier on the time-line).
 * The maximum supported {@code offset_date_time},
 * '+999999999-12-31T23:59:59.999999999-18:00'. This is the local date-time just
 * before midnight at the end of the maximum date in the minimum offset (larger
 * negative offsets are later on the time-line).
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

bool HAZELCAST_API
operator<(const offset_date_time& lhs, const offset_date_time& rhs);
} // namespace client
} // namespace hazelcast
namespace std {
template<>
struct HAZELCAST_API hash<hazelcast::client::offset_date_time>
{
    std::size_t operator()(const hazelcast::client::offset_date_time& f) const;
};
} // namespace std

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
