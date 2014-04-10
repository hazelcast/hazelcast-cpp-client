//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/LogLevel.h"
#include "hazelcast/util/Thread.h"
#include <iostream>


namespace hazelcast {
    namespace util {

        void ILogger::setLogLevel(int logLevel) {
            HazelcastLogLevel = logLevel;
        }

        void ILogger::severe(const std::string &message) {
            if (client::SEVERE >= HazelcastLogLevel) {
                (std::cout << "SEVERE: " << prefix << " [" << util::Thread::getThreadID() << "] " << message << std::endl);
            }
        }

        void ILogger::warning(const std::string &message) {
            if (client::WARNING >= HazelcastLogLevel) {
                (std::cout << "WARNING: " << prefix <<  " [" << util::Thread::getThreadID() << "] " << message << std::endl);
            }
        }

        void ILogger::info(const std::string &message) {
            if (client::WARNING >= HazelcastLogLevel) {
                (std::cout << "INFO: " << prefix << " [" << util::Thread::getThreadID() << "] " << message << std::endl);
            }
        }

        void ILogger::setPrefix(const std::string &prefix) {
            this->prefix = prefix;
        }
    }
}
