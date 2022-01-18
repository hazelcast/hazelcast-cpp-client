/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include <time.h>
#include <algorithm>
#include <string>
#include <vector>
#include <assert.h>
#include <stdint.h>
#include <chrono>

#include "hazelcast/util/export.h"

#define HAZELCAST_STRINGIZE(STR) HAZELCAST_INTERNAL_STRINGIZE(STR)
#define HAZELCAST_INTERNAL_STRINGIZE(STR) #STR

namespace hazelcast {
    namespace util {
        HAZELCAST_API int64_t get_current_thread_id();

        HAZELCAST_API void sleep(int seconds);

        /**
         * Fills the result with localtime if succesful
         * @return 0 on success, non-zero on error
         */
        HAZELCAST_API int localtime(const time_t *clock, struct tm *result);

        /**
         * Portable snprintf implementation
         */
        HAZELCAST_API int hz_snprintf(char *str, size_t len, const char *format, ...);

        /**
         * // converts the date string to 20160420. Removes '-' and '"' characters if exist
         * @param date The date to be modified
         */
        HAZELCAST_API void git_date_to_hazelcast_log_date(std::string &date);

        /**
         * @return the difference, measured in milliseconds, between the current time and midnight, January 1, 1970 UTC.
         */
        HAZELCAST_API int64_t current_time_millis();

        /**
         * @return the difference, measured in nanoseconds, between the current time and midnight, January 1, 1970 UTC.
         */
        HAZELCAST_API int64_t current_time_nanos();

        int32_t HAZELCAST_API get_available_core_count();

        template <typename T>
        const T &(min)(const T &left, const T &right) {
            return std::min<T>(left, right);
        }

        class HAZELCAST_API StringUtil {
        public:
            /**
             * Returns a String representation of the time.
             * <p>
             *
             * @param t time
             * @return the the formatted time string. Format is "%Y-%m-%d %H:%M:%S.%f".
             */
            static std::string time_to_string(std::chrono::steady_clock::time_point t);
        };

        class HAZELCAST_API Int64Util {
        public:
            /**
             * Returns the number of zero bits preceding the highest-order
             * ("leftmost") one-bit in the two's complement binary representation
             * of the specified {@code long} value.  Returns 64 if the
             * specified value has no one-bits in its two's complement representation,
             * in other words if it is equal to zero.
             *
             * <p>Note that this method is closely related to the logarithm base 2.
             * For all positive {@code long} values x:
             * <ul>
             * <li>floor(log<sub>2</sub>(x)) = {@code 63 - numberOfLeadingZeros(x)}
             * <li>ceil(log<sub>2</sub>(x)) = {@code 64 - numberOfLeadingZeros(x - 1)}
             * </ul>
             *
             * @param i the value whose number of leading zeros is to be computed
             * @return the number of zero bits preceding the highest-order
             *     ("leftmost") one-bit in the two's complement binary representation
             *     of the specified {@code long} value, or 64 if the value
             *     is equal to zero.
             */
            static int number_of_leading_zeros(int64_t i);
        };
    }
}




