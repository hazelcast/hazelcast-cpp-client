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

//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_UTIL_FUNCTIONS
#define HAZELCAST_UTIL_FUNCTIONS

#include "hazelcast/util/HazelcastDll.h"
#include <time.h>

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
    }
}


#endif //HAZELCAST_UTIL_FUNCTIONS

