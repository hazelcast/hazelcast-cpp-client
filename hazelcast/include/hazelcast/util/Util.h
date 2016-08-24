/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 5/3/13.

#ifndef HAZELCAST_UTIL_FUNCTIONS
#define HAZELCAST_UTIL_FUNCTIONS

#include "hazelcast/util/HazelcastDll.h"
#include <time.h>
#include <string>
#include <assert.h>
#include <stdint.h>

#define HAZELCAST_STRINGIZE(STR) STRINGIZE(STR)
#define STRINGIZE(STR) #STR

namespace hazelcast {
    namespace util {

        HAZELCAST_API long getThreadId();

        HAZELCAST_API void sleep(int seconds);

        HAZELCAST_API void sleepmillis(unsigned long milliseconds);

        HAZELCAST_API char *strtok(char *str, const char *sep, char ** context);

        /**
         * Fills the result with localtime if succesful
         * @return 0 on success, non-zero on error
         */
        HAZELCAST_API int localtime(const time_t *clock, struct tm *result);

        /**
         * Portable snprintf implementation
         */
        HAZELCAST_API int snprintf(char *str, size_t len, const char *format, ...);

        /**
         * // converts the date string to 20160420. Removes '-' and '"' characters if exist
         * @param date The date to be modified
         */
        HAZELCAST_API void gitDateToHazelcastLogDate(std::string &date);

        /**
         * @return the difference, measured in milliseconds, between the current time and midnight, January 1, 1970 UTC.
         */
        HAZELCAST_API int64_t currentTimeMillis();

        /**
         * @return 0 if error string could be obtained, non-zero otherwise
         */
        HAZELCAST_API int
        strerror_s(int errnum, char *strerrbuf, size_t buflen, const char *msgPrefix = (const char *) NULL);
    }
}


#endif //HAZELCAST_UTIL_FUNCTIONS

