//
// Created by sancar koyunlu on 20/02/14.
//


#ifndef HAZELCAST_ILogger
#define HAZELCAST_ILogger

#include "hazelcast/util/HazelcastDll.h"
#include <string>

namespace hazelcast {
    namespace util {
        namespace ILogger{
            static int HazelcastLogLevel;

            void severe(const std::string &methodName, const std::string &message);

            void warning(const std::string &methodName, const std::string &message);

            void info(const std::string &methodName, const std::string &message);

        };
    }
}

#endif //HAZELCAST_ILogger
