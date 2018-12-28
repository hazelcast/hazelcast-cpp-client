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
#ifndef HAZELCAST_UTIL_CONCURRENT_TIMEUNIT_H_
#define HAZELCAST_UTIL_CONCURRENT_TIMEUNIT_H_

#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        namespace concurrent {
            class NanoSeconds;
            class MicroSeconds;
            class MilliSeconds;
            class Seconds;
            class Minutes;
            class Hours;
            class Days;

            class HAZELCAST_API TimeUnit {
            public:
                // To maintain full signature compatibility with 1.5, and to improve the
                // clarity of the generated javadoc (see 6287639: Abstract methods in
                // enum classes should not be listed as abstract), method convert
                // etc. are not declared abstract but otherwise act as abstract methods.

                /**
                 * Converts the given time duration in the given unit to this unit.
                 * Conversions from finer to coarser granularities truncate, so
                 * lose precision. For example, converting {@code 999} milliseconds
                 * to seconds results in {@code 0}. Conversions from coarser to
                 * finer granularities with arguments that would numerically
                 * overflow saturate to {@code INT64_MIN} if negative or
                 * {@code INT64_MAX} if positive.
                 *
                 * <p>For example, to convert 10 minutes to milliseconds, use:
                 * {@code TimeUnit::MILLISECONDS::convert(10L, TimeUnit::MINUTES)}
                 *
                 * @param sourceDuration the time duration in the given {@code sourceUnit}
                 * @param sourceUnit the unit of the {@code sourceDuration} argument
                 * @return the converted duration in this unit,
                 * or {@code INT64_MIN} if conversion would negatively
                 * overflow, or {@code INT64_MAX} if it would positively overflow.
                 */
                virtual int64_t convert(int64_t sourceDuration, const TimeUnit &sourceUnit) const = 0;

                /**
                 * Equivalent to
                 * {@link #convert(int64_t, TimeUnit) NANOSECONDS.convert(duration, this)}.
                 * @param duration the duration
                 * @return the converted duration,
                 * or {@code INT64_MIN} if conversion would negatively
                 * overflow, or {@code INT64_MAX} if it would positively overflow.
                 */
                virtual int64_t toNanos(int64_t duration) const = 0;

                /**
                 * Equivalent to
                 * {@link #convert(int64_t, TimeUnit) MICROSECONDS.convert(duration, this)}.
                 * @param duration the duration
                 * @return the converted duration,
                 * or {@code INT64_MIN} if conversion would negatively
                 * overflow, or {@code INT64_MAX} if it would positively overflow.
                 */
                virtual int64_t toMicros(int64_t duration) const = 0;

                /**
                 * Equivalent to
                 * {@link #convert(int64_t, TimeUnit) MILLISECONDS.convert(duration, this)}.
                 * @param duration the duration
                 * @return the converted duration,
                 * or {@code INT64_MIN} if conversion would negatively
                 * overflow, or {@code INT64_MAX} if it would positively overflow.
                 */
                virtual int64_t toMillis(int64_t duration) const = 0;

                /**
                 * Equivalent to
                 * {@link #convert(int64_t, TimeUnit) SECONDS.convert(duration, this)}.
                 * @param duration the duration
                 * @return the converted duration,
                 * or {@code INT64_MIN} if conversion would negatively
                 * overflow, or {@code INT64_MAX} if it would positively overflow.
                 */
                virtual int64_t toSeconds(int64_t duration) const = 0;

                /**
                 * Equivalent to
                 * {@link #convert(int64_t, TimeUnit) MINUTES.convert(duration, this)}.
                 * @param duration the duration
                 * @return the converted duration,
                 * or {@code INT64_MIN} if conversion would negatively
                 * overflow, or {@code INT64_MAX} if it would positively overflow.
                 */
                virtual int64_t toMinutes(int64_t duration) const = 0;

                /**
                 * Equivalent to
                 * {@link #convert(int64_t, TimeUnit) HOURS.convert(duration, this)}.
                 * @param duration the duration
                 * @return the converted duration,
                 * or {@code INT64_MIN} if conversion would negatively
                 * overflow, or {@code INT64_MAX} if it would positively overflow.
                 */
                virtual int64_t toHours(int64_t duration) const = 0;

                /**
                 * Equivalent to
                 * {@link #convert(int64_t, TimeUnit) DAYS.convert(duration, this)}.
                 * @param duration the duration
                 * @return the converted duration
                 */
                virtual int64_t toDays(int64_t duration) const = 0;

                static const TimeUnit &NANOSECONDS();
                static const TimeUnit &MICROSECONDS();
                static const TimeUnit &MILLISECONDS();
                static const TimeUnit &SECONDS();
                static const TimeUnit &MINUTES();
                static const TimeUnit &HOURS();
                static const TimeUnit &DAYS();

            protected:
                // Handy constants for conversion methods
                static const int64_t C0 = 1L;
                static const int64_t C1 = C0 * 1000L;
                static const int64_t C2 = C1 * 1000L;
                static const int64_t C3 = C2 * 1000L;
                static const int64_t C4 = C3 * 60L;
                static const int64_t C5 = C4 * 60L;
                static const int64_t C6 = C5 * 24L;

                static const NanoSeconds NANOS;
                static const MicroSeconds MICROS;
                static const MilliSeconds MILLIS;
                static const Seconds SECS;
                static const Minutes MINS;
                static const Hours HRS;
                static const Days DS;

                static const int64_t MAX = INT64_MAX;

                static int64_t x(int64_t d, int64_t m, int64_t over);
            };

            /**
             * Time unit representing one thousandth of a microsecond
             */
            class HAZELCAST_API NanoSeconds : public TimeUnit {
            public:
                virtual int64_t convert(int64_t sourceDuration, const TimeUnit &sourceUnit) const;

                virtual int64_t toNanos(int64_t duration) const;

                virtual int64_t toMicros(int64_t duration) const;

                virtual int64_t toMillis(int64_t duration) const;

                virtual int64_t toSeconds(int64_t duration) const;

                virtual int64_t toMinutes(int64_t duration) const;

                virtual int64_t toHours(int64_t duration) const;

                virtual int64_t toDays(int64_t duration) const;
            };

            /**
             * Time unit representing one thousandth of a millisecond
             */
            class HAZELCAST_API MicroSeconds : public TimeUnit {
            public:
                virtual int64_t convert(int64_t sourceDuration, const TimeUnit &sourceUnit) const;

                virtual int64_t toNanos(int64_t duration) const;

                virtual int64_t toMicros(int64_t duration) const;

                virtual int64_t toMillis(int64_t duration) const;

                virtual int64_t toSeconds(int64_t duration) const;

                virtual int64_t toMinutes(int64_t duration) const;

                virtual int64_t toHours(int64_t duration) const;

                virtual int64_t toDays(int64_t duration) const;
            };

            /**
             * Time unit representing one thousandth of a second
             */
            class HAZELCAST_API MilliSeconds : public TimeUnit {
            public:
                virtual int64_t convert(int64_t sourceDuration, const TimeUnit &sourceUnit) const;

                virtual int64_t toNanos(int64_t duration) const;

                virtual int64_t toMicros(int64_t duration) const;

                virtual int64_t toMillis(int64_t duration) const;

                virtual int64_t toSeconds(int64_t duration) const;

                virtual int64_t toMinutes(int64_t duration) const;

                virtual int64_t toHours(int64_t duration) const;

                virtual int64_t toDays(int64_t duration) const;
            };

            /**
             * Time unit representing one second
             */
            class HAZELCAST_API Seconds : public TimeUnit {
            public:
                virtual int64_t convert(int64_t sourceDuration, const TimeUnit &sourceUnit) const;

                virtual int64_t toNanos(int64_t duration) const;

                virtual int64_t toMicros(int64_t duration) const;

                virtual int64_t toMillis(int64_t duration) const;

                virtual int64_t toSeconds(int64_t duration) const;

                virtual int64_t toMinutes(int64_t duration) const;

                virtual int64_t toHours(int64_t duration) const;

                virtual int64_t toDays(int64_t duration) const;
            };

            /**
             * Time unit representing sixty seconds
             */
            class HAZELCAST_API Minutes : public TimeUnit {
            public:
                virtual int64_t convert(int64_t sourceDuration, const TimeUnit &sourceUnit) const;

                virtual int64_t toNanos(int64_t duration) const;

                virtual int64_t toMicros(int64_t duration) const;

                virtual int64_t toMillis(int64_t duration) const;

                virtual int64_t toSeconds(int64_t duration) const;

                virtual int64_t toMinutes(int64_t duration) const;

                virtual int64_t toHours(int64_t duration) const;

                virtual int64_t toDays(int64_t duration) const;
            };

            /**
             * Time unit representing sixty minutes
             */
            class HAZELCAST_API Hours : public TimeUnit {
            public:
                virtual int64_t convert(int64_t sourceDuration, const TimeUnit &sourceUnit) const;

                virtual int64_t toNanos(int64_t duration) const;

                virtual int64_t toMicros(int64_t duration) const;

                virtual int64_t toMillis(int64_t duration) const;

                virtual int64_t toSeconds(int64_t duration) const;

                virtual int64_t toMinutes(int64_t duration) const;

                virtual int64_t toHours(int64_t duration) const;

                virtual int64_t toDays(int64_t duration) const;
            };

            /**
             * Time unit representing twenty four hours
             */
            class HAZELCAST_API Days : public TimeUnit {
            public:
                virtual int64_t convert(int64_t sourceDuration, const TimeUnit &sourceUnit) const;

                virtual int64_t toNanos(int64_t duration) const;

                virtual int64_t toMicros(int64_t duration) const;

                virtual int64_t toMillis(int64_t duration) const;

                virtual int64_t toSeconds(int64_t duration) const;

                virtual int64_t toMinutes(int64_t duration) const;

                virtual int64_t toHours(int64_t duration) const;

                virtual int64_t toDays(int64_t duration) const;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_CONCURRENT_TIMEUNIT_H_
