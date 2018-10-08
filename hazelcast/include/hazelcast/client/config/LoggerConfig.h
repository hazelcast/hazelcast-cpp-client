/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_CONFIG_LOGGERCONFIG_H_
#define HAZELCAST_CLIENT_CONFIG_LOGGERCONFIG_H_

#include <string>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        class HAZELCAST_API LoggerLevel {
        public:
            enum Level {
                SEVERE = 100, WARNING = 90, INFO = 50, FINEST = 20
            };

            static const char *getLevelString(const Level &level);
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
                 * @return The logger filename to be used for logging to a file. If this is filled, then file logging
                 * is enabled.
                 */
                const std::string &getFileName() const;

                /**
                 *
                 * @param fileName The file name into which the logs will be printed. This is a relative path to the
                 * process working directory or or an absolute path. File logging is closed by default.
                 */
                void setFileName(const std::string &fileName);

                /**
                 *
                 * @return true if printing to the standard output is enabled. This is enabled by default.
                 */
                bool isEnabledStandardOutput() const;

                /**
                 *
                 * @param enabledStandardOutput Enable/disable standard output logging
                 */
                void setEnabledStandardOutput(bool enabledStandardOutput);

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
                std::string fileName;
                bool enabledStandardOutput;
                LoggerLevel::Level logLevel;
            };
        }
    }
}

#endif /* HAZELCAST_CLIENT_CONFIG_LOGGERCONFIG_H_ */
