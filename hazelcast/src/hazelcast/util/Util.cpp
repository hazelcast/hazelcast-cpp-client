//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "hazelcast/util/Util.h"

#include "hazelcast/util/Thread.h"

#include <string.h>

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
        #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
			Sleep(seconds * 1000);
        #else
			::sleep((unsigned int)seconds);
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
                returnCode = localtime_s(clock, result);
            #else
                 if (NULL != localtime_r(clock, result)) {
                     returnCode = 0;
                 }
            #endif

            return returnCode;
        }
    }
}


