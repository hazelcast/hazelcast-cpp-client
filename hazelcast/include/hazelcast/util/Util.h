//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
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

        HAZELCAST_API char *strtok(char *str, const char *sep, char ** context);

        /**
         * Fills the result with localtime if succesful
         * @return 0 on success, non-zero on error
         */
        HAZELCAST_API int localtime(const time_t *clock, struct tm *result);
    }
}


#endif //HAZELCAST_UTIL_FUNCTIONS

