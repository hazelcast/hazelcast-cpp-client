
//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/LockGuard.h"

#include <stdio.h>
#include <iostream>
#include <time.h>
#include <assert.h>

namespace hazelcast {
    namespace util {

#define TIME_STRING_LENGTH 25

        ILogger& ILogger::getLogger() {
            static ILogger singleton;
            return singleton;
        }

        ILogger::ILogger() : HazelcastLogLevel(client::INFO) {
        }

        ILogger::~ILogger() {
        }

        void ILogger::setLogLevel(int logLevel) {
            HazelcastLogLevel = logLevel;
        }

        void ILogger::severe(const std::string& message) {
            if (isEnabled(client::SEVERE)) {
                char buffer [TIME_STRING_LENGTH];
                util::LockGuard l(lockMutex);
                // Due to the problem faced in Linux environment which is described
                // in https://gcc.gnu.org/ml/gcc/2003-12/msg00743.html, we could not use
                // std::cout here. outstream flush() function in stdlib 3.4.4 does not handle pthread_cancel call
                // appropriately.
                printf("%s SEVERE: %s [%ld] %s\n", getTime(buffer, TIME_STRING_LENGTH), prefix.c_str(), util::getThreadId(), message.c_str());
            }
        }

        void ILogger::warning(const std::string& message) {
            if (isEnabled(client::WARNING)) {
                char buffer [TIME_STRING_LENGTH];
                util::LockGuard l(lockMutex);
                printf("%s WARNING: %s [%ld] %s\n", getTime(buffer, TIME_STRING_LENGTH), prefix.c_str(), util::getThreadId(), message.c_str());
            }
        }

        void ILogger::info(const std::string& message) {
            if (isEnabled(client::INFO)) {
                char buffer [TIME_STRING_LENGTH];
                util::LockGuard l(lockMutex);
                printf("%s INFO: %s [%ld] %s\n", getTime(buffer, TIME_STRING_LENGTH), prefix.c_str(), util::getThreadId(), message.c_str());
            }
        }


        void ILogger::finest(const std::string& message) {
            if (isEnabled(client::FINEST)) {
                char buffer [TIME_STRING_LENGTH];
                util::LockGuard l(lockMutex);
                printf("%s FINEST: %s [%ld] %s\n", getTime(buffer, TIME_STRING_LENGTH), prefix.c_str(), util::getThreadId(), message.c_str());
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

            time (&rawtime);
            int timeResult = hazelcast::util::localtime (&rawtime, &timeinfo);
            assert(0 == timeResult);

            if (0 == timeResult) { // this if is needed for release build
                if (0 == strftime (buffer, length, "%b %d, %Y %I:%M:%S %p", &timeinfo)) {
                    buffer[0] = '\0'; // In case the strftime fails, just return an empty string
                }
            }

            // TODO: Change to thread specific stored buffer
            return buffer;
        }
    }
}
