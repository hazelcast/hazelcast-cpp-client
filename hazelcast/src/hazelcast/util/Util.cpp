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
//
// Created by sancar koyunlu on 5/3/13.

#include "hazelcast/util/Util.h"
#include "hazelcast/util/Thread.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/date_time.hpp>

#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

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

		void sleepmillis(uint64_t milliseconds){
        #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
			Sleep((DWORD) milliseconds);
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

        int hz_snprintf(char *str, size_t len, const char *format, ...) {
            va_list args;
            va_start(args, format);

            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            int result = vsnprintf_s(str, len, _TRUNCATE, format, args);
            if (result < 0) {
                return len > 0 ? len - 1 : 0;
            }
            return result;
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

        int64_t currentTimeMillis() {
            boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1));
            boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
            boost::posix_time::time_duration diff = now - epoch;
            return diff.total_milliseconds();
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
            #elif defined(__llvm__)
                return ::strerror_r(errnum, strerrbuf + numChars, buflen - numChars);
            #elif defined(__GNUC__)
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
            boost::posix_time::time_facet* facet = new boost::posix_time::time_facet("%Y-%m-%d %H:%M:%S.%f");
            std::stringstream dateStream;
            dateStream.imbue(std::locale(dateStream.getloc(), facet));
            boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1));
            dateStream << epoch + boost::posix_time::milliseconds(timeInMillis);
            return dateStream.str();
        }

        std::string StringUtil::timeToStringFriendly(int64_t timeInMillis) {
            return timeInMillis == 0 ? "never" : timeToString(timeInMillis);
        }
    }
}


