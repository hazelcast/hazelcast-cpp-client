//
// Created by sancar koyunlu on 20/02/14.
//


#ifndef HAZELCAST_ILogger
#define HAZELCAST_ILogger

#include "hazelcast/util/HazelcastDll.h"
#include <string>

namespace hazelcast {
    namespace client{
        enum LogLevel {
            SEVERE = 100, WARNING = 90, INFO = 50, FINEST = 20
        };
    }

    namespace util {
        class ILogger{
        public:
            static ILogger& getLogger()
            {
                static ILogger singleton;
                return singleton;
            }

            void setLogLevel(int logLevel);

            void severe(const std::string &message);

            void warning(const std::string &message);

            void info(const std::string &message);

            void finest(const std::string &message);

            void setPrefix(const std::string& prefix);

        private:
            int HazelcastLogLevel;
            std::string prefix;

            ILogger() :HazelcastLogLevel(client::INFO){}
            ~ILogger() {}
            ILogger(const ILogger&);
            ILogger& operator=(const ILogger&);
        };
    }
}

#endif //HAZELCAST_ILogger

