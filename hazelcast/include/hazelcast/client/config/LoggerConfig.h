/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <string>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class HAZELCAST_API LoggerLevel {
        public:
            enum Level {
                SEVERE = 100, WARNING = 90, INFO = 50, FINEST = 20
            };
        };

        enum LogLevel {
            SEVERE = LoggerLevel::SEVERE,
            WARNING = LoggerLevel::WARNING,
            INFO = LoggerLevel::INFO,
            FINEST = LoggerLevel::FINEST
        };

        namespace config {
            class HAZELCAST_API LoggerConfig {
            public:
                class Type {
                public:
                    enum LoggerType {
                        EASYLOGGINGPP
                    };
                };

                LoggerConfig();

                /**
                 *
                 * @return The type of the logger configured. see LoggerConfig::Type enum for possible loggers.
                 */
                Type::LoggerType getType() const;

                /**
                 *
                 * @param type The type of the logger that is configured.
                 */
                void setType(Type::LoggerType type);

                /**
                 *
                 * @return The logger configuration file. If this file is configured, no other configuration will be
                 * applied but only what is configured in the file will be applied. All log levels will work based on
                 * the provided configuration and setLogLevel will not be effective (You can enable disable any level
                 * in the configuration file).
                 */
                const std::string &getConfigurationFileName() const;

                /**
                 *
                 * @param fileName configuration file for the logger.
                 */
                void setConfigurationFileName(const std::string &fileName);

                /**
                 *
                 * @return The level for which the logs will be printed.
                 */
                LoggerLevel::Level getLogLevel() const;

                /**
                 *
                 * @param logLevel Set the log level for which the logs will be printed.
                 */
                void setLogLevel(LoggerLevel::Level logLevel);

            private:
                Type::LoggerType type;
                std::string configurationFileName;
                LoggerLevel::Level logLevel;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

