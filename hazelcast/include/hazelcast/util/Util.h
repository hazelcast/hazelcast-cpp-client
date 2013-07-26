//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_UTIL_FUNCTIONS
#define HAZELCAST_UTIL_FUNCTIONS

#include <string>
#include <map>
#include <vector>

namespace hazelcast {
    namespace util {

        struct cStrCmp {
            bool operator ()(const char *lhs, const char *rhs) const {
                return strcmp(lhs, rhs) < 0;
            }
        };

        std::string to_string(int);

        int getThreadId();

    }
}


#endif //HAZELCAST_UTIL_FUNCTIONS
