//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"
#include <iostream>


namespace hazelcast {
    namespace util {

        void ILogger::setLogLevel(int logLevel) {
            HazelcastLogLevel = logLevel;
        }

        void ILogger::severe(const std::string& message) {
            if (isEnabled(client::SEVERE)) {
                (std::cout << "SEVERE: " << prefix << " [" << util::getThreadId() << "] " << message << std::endl);
            }
        }

        void ILogger::warning(const std::string& message) {
            if (isEnabled(client::WARNING)) {
                (std::cout << "WARNING: " << prefix << " [" << util::getThreadId() << "] " << message << std::endl);
            }
        }

        void ILogger::info(const std::string& message) {
            if (isEnabled(client::INFO)) {
                (std::cout << "INFO: " << prefix << " [" << util::getThreadId() << "] " << message << std::endl);
            }
        }


        void ILogger::finest(const std::string& message) {
            if (isEnabled(client::FINEST)) {
                (std::cout << "FINEST: " << prefix << " [" << util::getThreadId() << "] " << message << std::endl);
            }
        }

        void ILogger::setPrefix(const std::string& prefix) {
            this->prefix = prefix;
        }

        bool ILogger::isEnabled(int logLevel) {
            return logLevel >= HazelcastLogLevel;
        }
    }
}
