
//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/LockGuard.h"

#include <iostream>
#include <time.h>
#include <assert.h>

namespace hazelcast {
    namespace util {

#define TIME_STRING_LENGTH 25

        void ILogger::setLogLevel(int logLevel) {
            HazelcastLogLevel = logLevel;
        }

        void ILogger::severe(const std::string& message) {
            if (isEnabled(client::SEVERE)) {
                char buffer [TIME_STRING_LENGTH];
                util::LockGuard l(lockMutex);
                (std::cout << getTime(buffer, TIME_STRING_LENGTH) << " SEVERE: " << prefix << " [" << util::getThreadId() << "] " << message << std::endl);
            }
        }

        void ILogger::warning(const std::string& message) {
            if (isEnabled(client::WARNING)) {
                char buffer [TIME_STRING_LENGTH];
                util::LockGuard l(lockMutex);
                (std::cout << getTime(buffer, TIME_STRING_LENGTH) << " WARNING: " << prefix << " [" << util::getThreadId() << "] " << message << std::endl);
            }
        }

        void ILogger::info(const std::string& message) {
            if (isEnabled(client::INFO)) {
                char buffer [TIME_STRING_LENGTH];
                util::LockGuard l(lockMutex);
                (std::cout << getTime(buffer, TIME_STRING_LENGTH) <<" INFO: " << prefix << " [" << util::getThreadId() << "] " << message << std::endl);
            }
        }


        void ILogger::finest(const std::string& message) {
            if (isEnabled(client::FINEST)) {
                char buffer [TIME_STRING_LENGTH];
                util::LockGuard l(lockMutex);
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
            struct tm timeinfo;

            buffer[0] = '\0'; // In case, the strftime fails just return an empty string for time

            time (&rawtime);
            int timeResult = hazelcast::util::localtime (&rawtime, &timeinfo);
            assert(0 == timeResult);

            if (0 == timeResult) { // this if is needed for release build
                strftime (buffer, length, "%b %d, %Y %I:%M:%S %p", &timeinfo);
            }

            // TODO: Change to thread specific stored buffer
            return buffer;
        }
    }
}
