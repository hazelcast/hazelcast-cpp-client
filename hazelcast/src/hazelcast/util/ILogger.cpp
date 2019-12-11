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

            easyLogger = easyloggingpp::Loggers::getLogger(instanceName);

            init();
        }

        ILogger::~ILogger() {
        }

        void ILogger::init() {
            std::string configurationFileName = loggerConfig.getConfigurationFileName();
            if (!configurationFileName.empty()) {
                easyloggingpp::Configurations confFromFile(configurationFileName);
                easyloggingpp::Loggers::reconfigureLogger(easyLogger, confFromFile);
                return;
            }

            easyloggingpp::Configurations defaultConf;

            defaultConf.setAll(easyloggingpp::ConfigurationType::Format,
                    std::string("%datetime %level: [%thread] ") + prefix + " %log");

            defaultConf.setAll(easyloggingpp::ConfigurationType::ToStandardOutput, "true");

            defaultConf.setAll(easyloggingpp::ConfigurationType::ToFile, "false");

            // Disable all levels first and then enable the desired levels
            defaultConf.setAll(easyloggingpp::ConfigurationType::Enabled, "false");

            client::LoggerLevel::Level logLevel = loggerConfig.getLogLevel();
            if (logLevel <= client::LoggerLevel::FINEST) {
                defaultConf.set(easyloggingpp::Level::Debug, easyloggingpp::ConfigurationType::Enabled, "true");
            }
            if (logLevel <= client::LoggerLevel::INFO) {
                defaultConf.set(easyloggingpp::Level::Info, easyloggingpp::ConfigurationType::Enabled, "true");
            }
            if (logLevel <= client::LoggerLevel::WARNING) {
                defaultConf.set(easyloggingpp::Level::Warning, easyloggingpp::ConfigurationType::Enabled, "true");
            }
            if (logLevel <= client::LoggerLevel::SEVERE) {
                defaultConf.set(easyloggingpp::Level::Fatal, easyloggingpp::ConfigurationType::Enabled, "true");
            }
            easyloggingpp::Loggers::reconfigureLogger(easyLogger, defaultConf);
        }

        void ILogger::severe(const std::string &message) {
            CFATAL(instanceName) << message;
        }

        void ILogger::warning(const std::string &message) {
            CWARNING(instanceName) << message;
        }

        void ILogger::info(const std::string &message) {
            CINFO(instanceName) << message;
        }

        void ILogger::finest(const std::string &message) {
            CDEBUG(instanceName) << message;
        }

        LeveledLogger ILogger::finest() {
            return LeveledLogger(*this, client::LoggerLevel::FINEST);
        }

        LeveledLogger ILogger::info() {
            return LeveledLogger(*this, client::LoggerLevel::INFO);
        }

        LeveledLogger ILogger::warning() {
            return LeveledLogger(*this, client::LoggerLevel::WARNING);
        }

        LeveledLogger ILogger::severe() {
            return LeveledLogger(*this, client::LoggerLevel::SEVERE);
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

        LeveledLogger::LeveledLogger(ILogger &logger, client::LoggerLevel::Level logLevel) : logger(logger),
                                                                                             requestedLogLevel(
                                                                                                     logLevel) {
        }

        LeveledLogger::~LeveledLogger() {
            switch (requestedLogLevel) {
                case client::LoggerLevel::FINEST:
                    CDEBUG(logger.instanceName) << out.str();
                    break;
                case client::LoggerLevel::INFO:
                    CINFO(logger.instanceName) << out.str();
                    break;
                case client::LoggerLevel::WARNING:
                    CWARNING(logger.instanceName) << out.str();
                    break;
                case client::LoggerLevel::SEVERE:
                    CFATAL(logger.instanceName) << out.str();
                    break;
            }
        }

        LeveledLogger::LeveledLogger(const LeveledLogger &rhs) : logger(rhs.logger),
                                                                 requestedLogLevel(rhs.requestedLogLevel) {
            out << out.str();
        }
    }
}
