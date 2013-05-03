//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "StringUtil.h"

namespace hazelcast {
    namespace client {
        namespace util {
            std::string StringUtil::to_string(int value) {
                char temp[10];
                sprintf(temp, "%d", value);
                return std::string(temp);
            };
        }
    }
}

