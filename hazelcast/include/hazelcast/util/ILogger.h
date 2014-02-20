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

            static void severe(const std::string &methodName, const std::string &message);

            static void warning(const std::string &methodName, const std::string &message);

            static void info(const std::string &methodName, const std::string &message);

            static void log(int logLevel, const std::string &methodName, const std::string &message);
        };
    }
}

#endif //HAZELCAST_ILogger
