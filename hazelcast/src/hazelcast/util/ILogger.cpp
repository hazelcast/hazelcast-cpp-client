/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace util {
        ILogger::ILogger(const std::string &instanceName, const std::string &groupName, const std::string &version,
                const client::config::LoggerConfig &loggerConfig)
                : instanceName(instanceName), groupName(groupName), version(version), loggerConfig(loggerConfig) {
            std::stringstream out;
            out << instanceName << "[" << groupName << "] [" << HAZELCAST_VERSION << "]";
            prefix = out.str();

            easyLogger = el::Loggers::getLogger(instanceName);
        }

        ILogger::~ILogger() {
        }

        bool ILogger::start() {
            std::string configurationFileName = loggerConfig.getConfigurationFileName();
            if (!configurationFileName.empty()) {
                el::Configurations defaultConf(configurationFileName);
                if (!defaultConf.parseFromFile(configurationFileName)) {
                    return false;
                }
                return el::Loggers::reconfigureLogger(easyLogger, defaultConf) != nullptr;
            }

            el::Configurations defaultConf;

            std::call_once(elOnceflag, el::Loggers::addFlag, el::LoggingFlag::DisableApplicationAbortOnFatalLog);

            defaultConf.set(el::Level::Global, el::ConfigurationType::Format,
                    std::string("%datetime{%d/%M/%Y %h:%m:%s.%g} %level: [%thread] ") + prefix + " %msg");

            defaultConf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "true");

            defaultConf.set(el::Level::Global, el::ConfigurationType::ToFile, "false");

            // Disable all levels first and then enable the desired levels
            defaultConf.set(el::Level::Global, el::ConfigurationType::Enabled, "false");

            client::LoggerLevel::Level logLevel = loggerConfig.getLogLevel();
            if (logLevel <= client::LoggerLevel::FINEST) {
                defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "true");
            }
            if (logLevel <= client::LoggerLevel::INFO) {
                defaultConf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
            }
            if (logLevel <= client::LoggerLevel::WARNING) {
                defaultConf.set(el::Level::Warning, el::ConfigurationType::Enabled, "true");
            }
            if (logLevel <= client::LoggerLevel::SEVERE) {
                defaultConf.set(el::Level::Fatal, el::ConfigurationType::Enabled, "true");
            }
            return el::Loggers::reconfigureLogger(easyLogger, defaultConf) != nullptr;
        }

        bool ILogger::isEnabled(const client::LoggerLevel::Level &logLevel) const {
            return logLevel >= this->loggerConfig.getLogLevel();
        }

        bool ILogger::isEnabled(int level) const {
            return isEnabled(static_cast<client::LoggerLevel::Level>(level));
        }

        bool ILogger::isFinestEnabled() const {
            return isEnabled(client::LoggerLevel::FINEST);
        }

        const std::string &ILogger::getInstanceName() const {
            return instanceName;
        }
    }
}
