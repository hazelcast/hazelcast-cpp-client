//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/ClientConfig.h"

namespace hazelcast {
    namespace util {
        namespace ILogger {
            void severe(const std::string &methodName, const std::string &message) {
                if (client::SEVERE >= HazelcastLogLevel) {
                    (std::cout << "SEVERE :: " << methodName << " " << message << std::endl);
                }
            }

            void warning(const std::string &methodName, const std::string &message) {
                if (client::WARNING >= HazelcastLogLevel) {
                    (std::cout << "WARNING :: " << methodName << " " << message << std::endl);
                }
            }

            void info(const std::string &methodName, const std::string &message) {
                if (client::WARNING >= HazelcastLogLevel) {
                    (std::cout << "INFO :: " << methodName << " " << message << std::endl);
                }
            }
        }
    }
}