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
        std::string to_string(int);

        template <typename key, typename value>
        std::vector<value> values(const std::map<key, value> &m) {
            std::vector<value> v;
            typename std::map<key, value>::const_iterator it;
            for (it = m.begin(); it != m.end(); it++) {
                v.push_back(it->second);
            }
            return v;
        };

    }
}


#endif //HAZELCAST_UTIL_FUNCTIONS
