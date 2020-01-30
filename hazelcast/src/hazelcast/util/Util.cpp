/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/util/TimeUtil.h"

#include <chrono>
#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <thread>
#include <regex>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#endif

namespace hazelcast {
    namespace util {

        int64_t getCurrentThreadId() {
        #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            return (int64_t) GetCurrentThreadId();
        #else
            int64_t threadId = 0;
            pthread_t thread = pthread_self();
            memcpy(&threadId, &thread, std::min(sizeof(threadId), sizeof(thread)));
            return threadId;
        #endif
        }

		void sleep(int seconds){
            sleepmillis((unsigned long)(1000 * seconds));
		}

		void sleepmillis(uint64_t milliseconds){
        #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
			Sleep((DWORD) milliseconds);
        #else
			::usleep((useconds_t)(1000 * milliseconds));
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

        int hz_snprintf(char *str, size_t len, const char *format, ...) {
            va_list args;
            va_start(args, format);

            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            int result = vsnprintf_s(str, len, _TRUNCATE, format, args);
            if (result < 0) {
                return len > 0 ? len - 1 : 0;
            }
            va_end(args);
            return result;
            #else
            int result = vsnprintf(str, len, format, args);
            va_end(args);
            return result;
            #endif
        }

        void gitDateToHazelcastLogDate(std::string &date) {
            // convert the date string from "2016-04-20" to 20160420
            date.erase(std::remove(date.begin(), date.end(), '"'), date.end());
            if (date != "NOT_FOUND") {
                date.erase(std::remove(date.begin(), date.end(), '-'), date.end());
            }
        }

        int64_t currentTimeMillis() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        int64_t currentTimeNanos() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        int strerror_s(int errnum, char *strerrbuf, size_t buflen, const char *msgPrefix) {
            int numChars = 0;
            if ((const char *)NULL != msgPrefix) {
                numChars = util::hz_snprintf(strerrbuf, buflen, "%s ", msgPrefix);
                if (numChars < 0) {
                    return numChars;
                }

                if (numChars >= (int)buflen - 1) {
                    return 0;
                }
            }

            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  errnum,
                  0,
                  (LPTSTR)(strerrbuf + numChars),
                  buflen - numChars,
                  NULL)) {
                return -1;
            }
            return 0;
            #elif defined(__llvm__) && ! _GNU_SOURCE
                /* XSI-compliant */
                return ::strerror_r(errnum, strerrbuf + numChars, buflen - numChars);
            #else
                /* GNU-specific */
                char *errStr = ::strerror_r(errnum, strerrbuf + numChars, buflen - numChars);
                int result = util::hz_snprintf(strerrbuf + numChars, buflen - numChars, "%s", errStr);
                if (result < 0) {
                    return result;
                }
                return 0;
            #endif
        }

        int32_t getAvailableCoreCount() {
            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            return sysinfo.dwNumberOfProcessors;
            #else
            return (int32_t) sysconf(_SC_NPROCESSORS_ONLN);
            #endif
        }

        std::string StringUtil::timeToString(int64_t timeInMillis) {
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::string s(30, '\0');
            std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
            return s;
        }

        std::string StringUtil::timeToStringFriendly(int64_t timeInMillis) {
            return timeInMillis == 0 ? "never" : timeToString(timeInMillis);
        }

        std::vector<std::string> StringUtil::tokenizeVersionString(const std::string &version) {
            // passing -1 as the submatch index parameter performs splitting
            std::regex re(".");
            std::sregex_token_iterator first{version.begin(), version.end(), re, -1}, last;
            return {first, last};
        }

        int Int64Util::numberOfLeadingZeros(int64_t i) {
            // HD, Figure 5-6
            if (i == 0)
                return 64;
            int n = 1;
            int64_t x = (int64_t)(i >> 32);
            if (x == 0) { n += 32; x = (int64_t)i; }
            if (x >> 16 == 0) { n += 16; x <<= 16; }
            if (x >> 24 == 0) { n +=  8; x <<=  8; }
            if (x >> 28 == 0) { n +=  4; x <<=  4; }
            if (x >> 30 == 0) { n +=  2; x <<=  2; }
            n -= (int) (x >> 31);
            return n;
        }
    }
}


