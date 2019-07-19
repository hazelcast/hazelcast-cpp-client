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

INITIALIZE_EASYLOGGINGPP

namespace hazelcast {
    namespace util {

		class ILoggerLogDispatchCallback : public el::LogDispatchCallback {
		public:
			void setILogger(ILogger *pILogger) {
				m_pILogger = pILogger;
			}

		protected:
			virtual void handle(const el::LogDispatchData* data)
			{
				if (m_pILogger != nullptr &&
					m_pILogger->getConfig().getLoggerCallback() != nullptr)
				{
					switch (data->logMessage()->level())
					{
					case el::Level::Global:
					case el::Level::Trace:
					case el::Level::Verbose:
					case el::Level::Debug:
						m_pILogger->getConfig().getLoggerCallback()->logCallback(client::LogLevel::FINEST, data->logMessage()->message());
						break;
					case el::Level::Warning:
						m_pILogger->getConfig().getLoggerCallback()->logCallback(client::LogLevel::WARNING, data->logMessage()->message());
						break;
					case el::Level::Fatal:
					case el::Level::Error:
						m_pILogger->getConfig().getLoggerCallback()->logCallback(client::LogLevel::SEVERE, data->logMessage()->message());
						break;
					default:
						m_pILogger->getConfig().getLoggerCallback()->logCallback(client::LogLevel::INFO, data->logMessage()->message());
						break;
					}
				}
			}

		private:
			ILogger *m_pILogger;
		};

        ILogger::ILogger(const std::string &instanceName, const std::string &groupName, const std::string &version,
                const client::config::LoggerConfig &loggerConfig)
                : instanceName(instanceName), groupName(groupName), version(version), loggerConfig(loggerConfig) {
            std::stringstream out;
            out << instanceName << "[" << groupName << "] [" << HAZELCAST_VERSION << "]";
            prefix = out.str();

            easyLogger = el::Loggers::getLogger(instanceName);

            init();
        }

		ILogger::~ILogger() {
			if (loggerConfig.getLoggerCallback() != nullptr)
			{
				el::Helpers::uninstallLogDispatchCallback<ILoggerLogDispatchCallback>("ILoggerLogDispatchCallback");
			}
		}

        void ILogger::init() {

			if (loggerConfig.getLoggerCallback() != nullptr)
			{
				el::Helpers::installLogDispatchCallback<ILoggerLogDispatchCallback>("ILoggerLogDispatchCallback");

				ILoggerLogDispatchCallback* callback = el::Helpers::logDispatchCallback<ILoggerLogDispatchCallback>("ILoggerLogDispatchCallback");
				callback->setEnabled(true);
				callback->setILogger(this);
			}

            std::string configurationFileName = loggerConfig.getConfigurationFileName();
            if (!configurationFileName.empty()) {
                el::Configurations confFromFile(configurationFileName);
                el::Loggers::reconfigureLogger(easyLogger, confFromFile);
                return;
            }

            el::Configurations defaultConf;

            defaultConf.setGlobally(el::ConfigurationType::Format,
                    std::string("%datetime %level: [%thread] ") + prefix + " %log");

            defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "true");

            defaultConf.setGlobally(el::ConfigurationType::ToFile, "false");

            // Disable all levels first and then enable the desired levels
            defaultConf.setGlobally(el::ConfigurationType::Enabled, "false");

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
            el::Loggers::reconfigureLogger(easyLogger, defaultConf);
        }

		client::config::LoggerConfig& ILogger::getConfig()
		{
			return loggerConfig;
		}

        void ILogger::severe(const std::string &message) {
            CLOG(FATAL, instanceName.c_str()) << message;
        }

        void ILogger::warning(const std::string &message) {
			CLOG(WARNING, instanceName.c_str()) << message;
        }

        void ILogger::info(const std::string &message) {
			CLOG(INFO, instanceName.c_str()) << message;
        }

        void ILogger::finest(const std::string &message) {
			CLOG(DEBUG, instanceName.c_str()) << message;
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
					CLOG(DEBUG, logger.instanceName.c_str()) << out.str();
                    break;
                case client::LoggerLevel::INFO:
					CLOG(INFO, logger.instanceName.c_str()) << out.str();
                    break;
                case client::LoggerLevel::WARNING:
					CLOG(WARNING, logger.instanceName.c_str()) << out.str();
                    break;
                case client::LoggerLevel::SEVERE:
					CLOG(FATAL, logger.instanceName.c_str()) << out.str();
                    break;
            }
        }

        LeveledLogger::LeveledLogger(const LeveledLogger &rhs) : logger(rhs.logger),
                                                                 requestedLogLevel(rhs.requestedLogLevel) {
            out << out.str();
        }
    }
}
