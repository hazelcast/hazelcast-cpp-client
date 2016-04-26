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

#include "hazelcast/util/Util.h"
#include "hazelcast/util/Thread.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

namespace hazelcast {
    namespace util {

        long getThreadId() {
            return util::Thread::getThreadID();
        }

		void sleep(int seconds){
            sleepmillis((unsigned long)(1000 * seconds));
		}

		void sleepmillis(unsigned long milliseconds){
        #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
			Sleep(milliseconds);
        #else
			::usleep((useconds_t)(1000 * milliseconds));
        #endif
		}

        char *strtok(char *str, const char *sep, char **context) {
            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                return strtok_s(str, sep, context);
            #else
                return strtok_r(str, sep, context);
            #endif
        }

        int localtime(const time_t *clock, struct tm *result) {
            int returnCode = -1;
            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                returnCode = localtime_s(result, clock);
            #else
                 if (NULL != localtime_r(clock, result)) {
                     returnCode = 0;
                 }
            #endif

            return returnCode;
        }

        int snprintf(char *str, size_t len, const char *format, ...) {
            va_list args;
            va_start(args, format);

            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            return vsnprintf_s(str, len, _TRUNCATE, format, args);
            #else
            return vsnprintf(str, len, format, args);
            #endif
        }

        void gitDateToHazelcastLogDate(std::string &date) {
            // convert the date string from "2016-04-20" to 20160420
            date.erase(std::remove(date.begin(), date.end(), '"'), date.end());
            if (date != "NOT_FOUND") {
                date.erase(std::remove(date.begin(), date.end(), '-'), date.end());
            }
        }
    }
}


