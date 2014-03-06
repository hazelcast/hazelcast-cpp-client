//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/LogLevel.h"
#include <iostream>


namespace hazelcast {
    namespace util {
        namespace ILogger {
            void severe(const std::string &message) {
                if (client::SEVERE >= HazelcastLogLevel) {
                    (std::cout << "SEVERE:: "<< message << std::endl);
                }
            }

            void warning(const std::string &message) {
                if (client::WARNING >= HazelcastLogLevel) {
                    (std::cout << "WARNING:: " << message << std::endl);
                }
            }

            void info(const std::string &message) {
                if (client::WARNING >= HazelcastLogLevel) {
                    (std::cout << "INFO:: " << message << std::endl);
                }
            }
        }
    }
}