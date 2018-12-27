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

#include <hazelcast/util/concurrent/TimeUnit.h>

#include "hazelcast/util/concurrent/TimeUnit.h"

namespace hazelcast {
    namespace util {
        namespace concurrent {
            const NanoSeconds TimeUnit::NANOSECONDS;
            const MicroSeconds TimeUnit::MICROSECONDS;
            const MilliSeconds TimeUnit::MILLISECONDS;
            const Seconds TimeUnit::SECONDS;
            const Minutes TimeUnit::MINUTES;
            const Days TimeUnit::DAYS;

            /**
             * Scale d by m, checking for overflow.
             * This has a short name to make above code more readable.
             */
            int64_t TimeUnit::x(int64_t d, int64_t m, int64_t over) {
                if (d >  over) return INT64_MAX;
                if (d < -over) return INT64_MIN;
                return d * m;
            }
            
            int64_t NanoSeconds::toNanos(int64_t d) const   { return d; }
            int64_t NanoSeconds::toMicros(int64_t d) const  { return d/(C1/C0); }
            int64_t NanoSeconds::toMillis(int64_t d) const  { return d/(C2/C0); }
            int64_t NanoSeconds::toSeconds(int64_t d) const { return d/(C3/C0); }
            int64_t NanoSeconds::toMinutes(int64_t d) const { return d/(C4/C0); }
            int64_t NanoSeconds::toHours(int64_t d) const   { return d/(C5/C0); }
            int64_t NanoSeconds::toDays(int64_t d) const    { return d/(C6/C0); }
            int64_t NanoSeconds::convert(int64_t d, const TimeUnit &u) const { return u.toNanos(d); }

            int64_t MicroSeconds::toNanos(int64_t d) const   { return x(d, C1/C0, MAX/(C1/C0)); }
            int64_t MicroSeconds::toMicros(int64_t d) const  { return d; }
            int64_t MicroSeconds::toMillis(int64_t d) const  { return d/(C2/C1); }
            int64_t MicroSeconds::toSeconds(int64_t d) const { return d/(C3/C1); }
            int64_t MicroSeconds::toMinutes(int64_t d) const { return d/(C4/C1); }
            int64_t MicroSeconds::toHours(int64_t d) const   { return d/(C5/C1); }
            int64_t MicroSeconds::toDays(int64_t d) const    { return d/(C6/C1); }
            int64_t MicroSeconds::convert(int64_t d, const TimeUnit &u) const { return u.toMicros(d); }

            int64_t MilliSeconds::toNanos(int64_t d) const   { return x(d, C2/C0, MAX/(C2/C0)); }
            int64_t MilliSeconds::toMicros(int64_t d) const  { return x(d, C2/C1, MAX/(C2/C1)); }
            int64_t MilliSeconds::toMillis(int64_t d) const  { return d; }
            int64_t MilliSeconds::toSeconds(int64_t d) const { return d/(C3/C2); }
            int64_t MilliSeconds::toMinutes(int64_t d) const { return d/(C4/C2); }
            int64_t MilliSeconds::toHours(int64_t d) const   { return d/(C5/C2); }
            int64_t MilliSeconds::toDays(int64_t d) const    { return d/(C6/C2); }
            int64_t MilliSeconds::convert(int64_t d, const TimeUnit &u) const  { return u.toMillis(d); }

            int64_t Seconds::toNanos(int64_t d) const   { return x(d, C3/C0, MAX/(C3/C0)); }
            int64_t Seconds::toMicros(int64_t d) const  { return x(d, C3/C1, MAX/(C3/C1)); }
            int64_t Seconds::toMillis(int64_t d) const  { return x(d, C3/C2, MAX/(C3/C2)); }
            int64_t Seconds::toSeconds(int64_t d) const { return d; }
            int64_t Seconds::toMinutes(int64_t d) const { return d/(C4/C3); }
            int64_t Seconds::toHours(int64_t d) const   { return d/(C5/C3); }
            int64_t Seconds::toDays(int64_t d) const    { return d/(C6/C3); }
            int64_t Seconds::convert(int64_t d, const TimeUnit &u) const { return u.toSeconds(d); }

            int64_t Minutes::toNanos(int64_t d) const   { return x(d, C4/C0, MAX/(C4/C0)); }
            int64_t Minutes::toMicros(int64_t d) const  { return x(d, C4/C1, MAX/(C4/C1)); }
            int64_t Minutes::toMillis(int64_t d) const  { return x(d, C4/C2, MAX/(C4/C2)); }
            int64_t Minutes::toSeconds(int64_t d) const { return x(d, C4/C3, MAX/(C4/C3)); }
            int64_t Minutes::toMinutes(int64_t d) const { return d; }
            int64_t Minutes::toHours(int64_t d) const   { return d/(C5/C4); }
            int64_t Minutes::toDays(int64_t d) const    { return d/(C6/C4); }
            int64_t Minutes::convert(int64_t d, const TimeUnit &u) const { return u.toMinutes(d); }

            int64_t Hours::toNanos(int64_t d) const   { return x(d, C5/C0, MAX/(C5/C0)); }
            int64_t Hours::toMicros(int64_t d) const  { return x(d, C5/C1, MAX/(C5/C1)); }
            int64_t Hours::toMillis(int64_t d) const  { return x(d, C5/C2, MAX/(C5/C2)); }
            int64_t Hours::toSeconds(int64_t d) const { return x(d, C5/C3, MAX/(C5/C3)); }
            int64_t Hours::toMinutes(int64_t d) const { return x(d, C5/C4, MAX/(C5/C4)); }
            int64_t Hours::toHours(int64_t d) const   { return d; }
            int64_t Hours::toDays(int64_t d) const    { return d/(C6/C5); }
            int64_t Hours::convert(int64_t d, const TimeUnit &u) const { return u.toHours(d); }

            int64_t Days::toNanos(int64_t d) const   { return x(d, C6/C0, MAX/(C6/C0)); }
            int64_t Days::toMicros(int64_t d) const  { return x(d, C6/C1, MAX/(C6/C1)); }
            int64_t Days::toMillis(int64_t d) const  { return x(d, C6/C2, MAX/(C6/C2)); }
            int64_t Days::toSeconds(int64_t d) const { return x(d, C6/C3, MAX/(C6/C3)); }
            int64_t Days::toMinutes(int64_t d) const { return x(d, C6/C4, MAX/(C6/C4)); }
            int64_t Days::toHours(int64_t d) const   { return x(d, C6/C5, MAX/(C6/C5)); }
            int64_t Days::toDays(int64_t d) const    { return d; }
            int64_t Days::convert(int64_t d, const TimeUnit &u) const { return u.toDays(d); }
        }
    }
}
