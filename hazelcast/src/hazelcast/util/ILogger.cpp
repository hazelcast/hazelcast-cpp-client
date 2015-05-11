
//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"
#include <iostream>

namespace hazelcast {
    namespace util {

#define TIME_STRING_LENGTH 25

        void ILogger::setLogLevel(int logLevel) {
            HazelcastLogLevel = logLevel;
        }

        void ILogger::severe(const std::string& message) {
            if (isEnabled(client::SEVERE)) {
                char buffer [TIME_STRING_LENGTH];

                (std::cout << getTime(buffer, TIME_STRING_LENGTH) << " SEVERE: " << prefix << " [" << util::getThreadId() << "] " << message << std::endl);
            }
        }

        void ILogger::warning(const std::string& message) {
            if (isEnabled(client::WARNING)) {
                char buffer [TIME_STRING_LENGTH];

                (std::cout << getTime(buffer, TIME_STRING_LENGTH) << " WARNING: " << prefix  << message << std::endl);
            }
        }

        void ILogger::info(const std::string& message) {
            if (isEnabled(client::INFO)) {
                char buffer [TIME_STRING_LENGTH];

                (std::cout << getTime(buffer, TIME_STRING_LENGTH) <<" INFO: " << prefix << " [" << util::getThreadId() << "] " << message << std::endl);
            }
        }


        void ILogger::finest(const std::string& message) {
            if (isEnabled(client::FINEST)) {
                char buffer [TIME_STRING_LENGTH];

                (std::cout << getTime(buffer, TIME_STRING_LENGTH) << " FINEST: " << prefix << " [" << util::getThreadId() << "] " << message << std::endl);
            }
        }

        void ILogger::setPrefix(const std::string& prefix) {
            this->prefix = prefix;
        }

        bool ILogger::isEnabled(int logLevel) {
            return logLevel >= HazelcastLogLevel;
        }

        const char *ILogger::getTime(char *buffer, size_t length) const {
            time_t rawtime;
            struct tm * timeinfo;

            time (&rawtime);
            timeinfo = localtime (&rawtime);

            strftime (buffer, length, "%b %d, %Y %r", timeinfo);

            // TODO: Change to thread specific stored buffer
            return buffer;
        }
    }
}