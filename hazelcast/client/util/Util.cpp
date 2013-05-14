//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "Util.h"
#include <sstream>

namespace hazelcast {
    namespace client {
        namespace util {
            std::string to_string(int value) {
                std::stringstream s;
                s << value;
                return s.str();
            };
        }
    }
}

