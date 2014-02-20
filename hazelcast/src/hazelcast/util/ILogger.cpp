//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/ClientConfig.h"

namespace hazelcast {
    namespace util {
        namespace ILogger {
            void severe(const std::string &methodName, const std::string &message) {
                log((int) client::SEVERE, methodName, message);
            }

            void warning(const std::string &methodName, const std::string &message) {
                log((int) client::WARNING, methodName, message);
            }

            void info(const std::string &methodName, const std::string &message) {
                log((int) client::INFO, methodName, message);
            }

            void log(int logLevel, const std::string &methodName, const std::string &message) {
                if (logLevel >= HazelcastLogLevel) {
                    (std::cout << message << " at " << methodName << std::endl);
                }

            }
        }
    }
}