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
#include "hazelcast/client/config/LoggerConfig.h"

namespace hazelcast {
    namespace client {
        namespace config {

            const std::string &LoggerConfig::getFileName() const {
                return fileName;
            }

            void LoggerConfig::setFileName(const std::string &fileName) {
                LoggerConfig::fileName = fileName;
            }

            bool LoggerConfig::isEnabledStandardOutput() const {
                return enabledStandardOutput;
            }

            void LoggerConfig::setEnabledStandardOutput(bool enabledStandardOutput) {
                LoggerConfig::enabledStandardOutput = enabledStandardOutput;
            }

            LoggerConfig::LoggerConfig() : type(Type::EASYLOGGINGPP), enabledStandardOutput(true),
                                           logLevel(LoggerLevel::INFO) {}

            LoggerConfig::Type::LoggerType LoggerConfig::getType() const {
                return type;
            }

            void LoggerConfig::setType(LoggerConfig::Type::LoggerType type) {
                LoggerConfig::type = type;
            }

            LoggerLevel::Level LoggerConfig::getLogLevel() const {
                return logLevel;
            }

            void LoggerConfig::setLogLevel(LoggerLevel::Level logLevel) {
                LoggerConfig::logLevel = logLevel;
            }
        }
    }
}

