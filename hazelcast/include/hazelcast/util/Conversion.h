//
// Created by sancar koyunlu on 8/22/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_Conversion
#define HAZELCAST_Conversion

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        template <class T, class U>
        class HAZELCAST_API Conversion {
            typedef char Small;

            class Big {
                char dummy[2];
            };

            static Small Test(U);

            static Big Test(...);

            static T MakeT();

        public:
            enum {
                exists = sizeof(Test(MakeT())) == sizeof(Small)
            };
        };
    }
}


#endif //HAZELCAST_Conversion
