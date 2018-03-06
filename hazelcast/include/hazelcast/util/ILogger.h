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

#ifndef HAZELCAST_ILogger
#define HAZELCAST_ILogger

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/LockGuard.h"

#include <string>
#include <iostream>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class HAZELCAST_API LoggerLevel {
        public:
            enum Level {
                SEVERE = 3, WARNING = 2, INFO = 1, FINEST = 0
            };

            static const char *getLevelString(const Level &level) {
                return levelStrings[level];
            }
        private:
            static const char *levelStrings[4];
        };

        enum LogLevel {
            SEVERE = LoggerLevel::SEVERE, WARNING = LoggerLevel::WARNING, INFO = LoggerLevel::INFO, FINEST = LoggerLevel::FINEST
        };
    }

    namespace util {
        #define TIME_STRING_LENGTH 25

        class LeveledLogger;

        class HAZELCAST_API ILogger {
            friend class LeveledLogger;
        public:
            static ILogger& getLogger();

            /**
             * @deprecated Please use setLogLevel(const client::LogLevel::Level &logLevel)
             * @param logLevel The log level values
             */
            void setLogLevel(int logLevel);

            void setLogLevel(const client::LoggerLevel::Level &logLevel);

            void severe(const std::string& message);

            void warning(const std::string& message);

            void info(const std::string& message);

            void finest(const std::string& message);

            void setPrefix(const std::string& prefix);

            LeveledLogger finest();

            LeveledLogger info();

            LeveledLogger warning();

            LeveledLogger severe();

            bool isEnabled(const client::LoggerLevel::Level &logLevel) const;

            /**
             * @deprecated Please use isEnabled(const client::LogLevel::Level &logLevel)
             * @param logLevel The level of the logger for which it will print the logs.
             * @return true if the level is enabled.
             */
            bool isEnabled(int logLevel) const;

            bool isFinestEnabled() const;
        private:
            client::LoggerLevel::Level logLevel;
            std::string prefix;

            ILogger();

            ~ILogger();

            const char *getTime(char * buffer, size_t length) const;

            const char *getLevelString(client::LoggerLevel::Level logLevel) const;

            void printMessagePrefix(client::LoggerLevel::Level logLevel) const;

            ILogger(const ILogger&);

            ILogger& operator=(const ILogger&);

            util::Mutex lockMutex;

            static ILogger singletonLogger;

            void printLog(client::LoggerLevel::Level level, const std::string &message);
        };

        /**
         * TODO: The lock may not be released on thread cancellations since the destructor may not be called.
         */
        class HAZELCAST_API LeveledLogger {
        public:
            // Gets the logger lock
            LeveledLogger(ILogger &logger, client::LoggerLevel::Level logLevel);

            // Releases the logger lock.
            virtual ~LeveledLogger();

            template <typename T>
            LeveledLogger &operator<<(const T &value) {
                if (logger.isEnabled(logger.logLevel)) {
                    std::cout << value;
                }
                return *this;
            }

        private:
            LeveledLogger();

            ILogger &logger;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#endif //HAZELCAST_ILogger
