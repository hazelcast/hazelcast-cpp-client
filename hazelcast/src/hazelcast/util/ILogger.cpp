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
        const char *client::LoggerLevel::levelStrings[4] = {"FINEST", "INFO", "WARNING", "SEVERE"};
    }

    namespace util {
        ILogger ILogger::singletonLogger;

        ILogger& ILogger::getLogger() {
            return singletonLogger;
        }

        ILogger::ILogger() : logLevel(client::LoggerLevel::INFO) {
        }

        ILogger::~ILogger() {
        }

        void ILogger::setLogLevel(const client::LoggerLevel::Level &logLevel) {
            this->logLevel = logLevel;
        }

        void ILogger::setLogLevel(int logLevel) {
            this->logLevel = (client::LoggerLevel::Level) logLevel;
        }

        void ILogger::severe(const std::string& message) {
            printLog(client::LoggerLevel::SEVERE, message);
        }

        void ILogger::warning(const std::string& message) {
            printLog(client::LoggerLevel::WARNING, message);
        }

        void ILogger::info(const std::string& message) {
            printLog(client::LoggerLevel::INFO, message);
        }

        void ILogger::finest(const std::string& message) {
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

        void ILogger::setPrefix(const std::string& prefix) {
            this->prefix = prefix;
        }

        bool ILogger::isEnabled(const client::LoggerLevel::Level &logLevel) const {
            return logLevel >= this->logLevel;
        }

        bool ILogger::isEnabled(int level) const {
            return isEnabled((client::LoggerLevel::Level) level);
        }

        const char *ILogger::getTime(char *buffer, size_t length) const {
            time_t rawtime;
            struct tm timeinfo;

            time (&rawtime);
            int timeResult = hazelcast::util::localtime (&rawtime, &timeinfo);
            assert(0 == timeResult);

            if (0 == timeResult) { // this if is needed for release build
                if (0 == strftime (buffer, length, "%b %d, %Y %I:%M:%S %p", &timeinfo)) {
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
            char buffer[TIME_STRING_LENGTH];
            std::cout << getTime(buffer, TIME_STRING_LENGTH) << " " << getLevelString(logLevel) << ": [" << getThreadId()
                      << "] " << prefix << " ";
        }

        void ILogger::printLog(client::LoggerLevel::Level level, const std::string &message) {
            if (!isEnabled(level)) {
                return;
            }

            {
                util::LockGuard l(lockMutex);
                printMessagePrefix(level);
                std::cout << message;
            }
            // Due to the problem faced in Linux environment which is described
            // in https://gcc.gnu.org/ml/gcc/2003-12/msg00743.html, we could not use
            // std::cout here. outstream flush() function in stdlib 3.4.4 does not handle pthread_cancel call
            // appropriately.
            std::flush(std::cout);
        }

        LeveledLogger::LeveledLogger(ILogger &logger, client::LoggerLevel::Level logLevel) : logger(logger) {
            if (logger.isEnabled(logLevel)) {
                logger.lockMutex.lock();
                logger.printMessagePrefix(logLevel);
            }
        }

        LeveledLogger::~LeveledLogger() {
            std::cout << std::endl;
            logger.lockMutex.unlock();
            std::flush(std::cout);
        }
    }
}
