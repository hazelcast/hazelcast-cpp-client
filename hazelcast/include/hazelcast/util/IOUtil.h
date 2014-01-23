//
// Created by sancar koyunlu on 23/01/14.
//


#ifndef HAZELCAST_IOUtil
#define HAZELCAST_IOUtil

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace util {
            class HAZELCAST_API IOUtil {
            public:
                enum PRIMITIVE_IDS {
                    PRIMITIVE_TYPE_BOOLEAN = 1,
                    PRIMITIVE_TYPE_BYTE = 2,
                    PRIMITIVE_TYPE_SHORT = 3,
                    PRIMITIVE_TYPE_INTEGER = 4,
                    PRIMITIVE_TYPE_LONG = 5,
                    PRIMITIVE_TYPE_FLOAT = 6,
                    PRIMITIVE_TYPE_DOUBLE = 7,
                    PRIMITIVE_TYPE_UTF = 8
                };

            };
        }
    }
}


#endif //HAZELCAST_IOUtil
