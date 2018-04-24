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

//
// Created by sancar koyunlu on 20/02/14.
//

#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/LockGuard.h"

#include <stdio.h>
#include <iostream>
#include <time.h>
#include <assert.h>

namespace hazelcast {
    namespace client {
        const char *LoggerLevel::getLevelString(const Level &level) {
            switch (level) {
                case SEVERE:
                    return "SEVERE";
                case WARNING:
                    return "WARNING";
                case INFO:
                    return "INFO";
                case FINEST:
                    return "FINEST";
                default:
                    assert(0);
            }
            return NULL;
        }
    }

    namespace util {
        ILogger ILogger::singletonLogger;

        ILogger &ILogger::getLogger() {
            return singletonLogger;
        }

        ILogger::ILogger() : logLevel(client::LoggerLevel::INFO) {
        }

        ILogger::~ILogger() {
        }

        void ILogger::setLogLevel(const client::LoggerLevel::Level &logLevel) {
            this->logLevel = logLevel;
        }

        void ILogger::severe(const std::string &message) {
            printLog(client::LoggerLevel::SEVERE, message);
        }

        void ILogger::warning(const std::string &message) {
            printLog(client::LoggerLevel::WARNING, message);
        }

        void ILogger::info(const std::string &message) {
            printLog(client::LoggerLevel::INFO, message);
        }

        void ILogger::finest(const std::string &message) {
            printLog(client::LoggerLevel::FINEST, message);
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

        void ILogger::setPrefix(const std::string &prefix) {
            this->prefix = prefix;
        }

        bool ILogger::isEnabled(const client::LoggerLevel::Level &logLevel) const {
            return logLevel >= this->logLevel;
        }

        bool ILogger::isEnabled(int level) const {
            return isEnabled(static_cast<client::LoggerLevel::Level>(level));
        }

        const char *ILogger::getTime(char *buffer, size_t length) const {
            time_t rawtime;
            struct tm timeinfo;

            time(&rawtime);
            int timeResult = hazelcast::util::localtime(&rawtime, &timeinfo);
            assert(0 == timeResult);

            if (0 == timeResult) { // this if is needed for release build
                if (0 == strftime(buffer, length, "%b %d, %Y %I:%M:%S %p", &timeinfo)) {
                    buffer[0] = '\0'; // In case the strftime fails, just return an empty string
                }
            }

            // TODO: Change to thread specific stored buffer
            return buffer;
        }

        bool ILogger::isFinestEnabled() const {
            return isEnabled(client::LoggerLevel::FINEST);
        }

        const char *ILogger::getLevelString(client::LoggerLevel::Level logLevel) const {
            return client::LoggerLevel::getLevelString(logLevel);
        }

        void ILogger::printMessagePrefix(client::LoggerLevel::Level logLevel) const {
            printMessagePrefix(logLevel, std::cout);
        }

        void ILogger::printLog(client::LoggerLevel::Level level, const std::string &message, bool printPrefix) {
            if (!isEnabled(level)) {
                return;
            }

            {
                util::LockGuard l(lockMutex);
                if (printPrefix) {
                    printMessagePrefix(level);
                }
                std::cout << message << std::endl;
                std::flush(std::cout);
            }
        }

        void ILogger::printMessagePrefix(client::LoggerLevel::Level logLevel, std::ostream &out) const {
            char buffer[TIME_STRING_LENGTH];
            out << getTime(buffer, TIME_STRING_LENGTH) << " " << getLevelString(logLevel) << ": [" << getCurrentThreadId()
                << "] " << prefix << " ";
        }

        LeveledLogger::LeveledLogger(ILogger &logger, client::LoggerLevel::Level logLevel) : logger(logger),
                                                                                             requestedLogLevel(
                                                                                                     logLevel) {
            if (!logger.isEnabled(requestedLogLevel)) {
                return;
            }

            logger.printMessagePrefix(requestedLogLevel, out);
        }

        LeveledLogger::~LeveledLogger() {
            logger.printLog(requestedLogLevel, out.str(), false);
        }

        LeveledLogger::LeveledLogger(const LeveledLogger &rhs) : logger(rhs.logger),
                                                                 requestedLogLevel(rhs.requestedLogLevel) {
            out << out.str();
        }
    }
}
