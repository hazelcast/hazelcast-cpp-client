//
// Created by sancar koyunlu on 04/04/14.
//


#ifndef HAZELCAST_ExceptionHandler
#define HAZELCAST_ExceptionHandler

#include "hazelcast/util/HazelcastDll.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace exception {
            class HAZELCAST_API ExceptionHandler {
            public:
                static std::string INTERRUPTED;
                static std::string INSTANCE_NOT_ACTIVE;

                static void rethrow(const std::string &exceptionName, const std::string &message);

            };


        }
    }
}

#endif //HAZELCAST_ExceptionHandler
