//
// Created by sancar koyunlu on 20/02/14.
//


#ifndef HAZELCAST_ILogger
#define HAZELCAST_ILogger

#include "hazelcast/util/HazelcastDll.h"
#include "Mutex.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        enum LogLevel {
            SEVERE = 100, WARNING = 90, INFO = 50, FINEST = 20
        };
    }

    namespace util {
        class HAZELCAST_API ILogger {
        public:
            static ILogger& getLogger() {
                static ILogger singleton;
                return singleton;
            }

            void setLogLevel(int logLevel);

            void severe(const std::string& message);

            void warning(const std::string& message);

            void info(const std::string& message);

            void finest(const std::string& message);

            void setPrefix(const std::string& prefix);

            bool isEnabled(int logLevel);

        private:
            int HazelcastLogLevel;
            std::string prefix;

            ILogger() : HazelcastLogLevel(client::INFO) {
            }

            ~ILogger() {
            }

            const char *getTime(char * buffer, size_t length) const;

            ILogger(const ILogger&);

            ILogger& operator=(const ILogger&);

            util::Mutex lockMutex;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#endif //HAZELCAST_ILogger
