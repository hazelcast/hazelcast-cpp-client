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

#ifndef HAZELCAST_ILogger
#define HAZELCAST_ILogger

#include <string>
#include <iostream>
#include <sstream>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
// We need to include this header before easylogging++/easylogging++.h
#include <winsock2.h>
#endif
#include <easylogging++/easylogging++.h>


#include <boost/enable_shared_from_this.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/config/LoggerConfig.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/LockGuard.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace util {
        class LeveledLogger;

        class HAZELCAST_API ILogger : public boost::enable_shared_from_this<ILogger> {
            friend class LeveledLogger;

        public:
            ILogger(const std::string &instanceName, const std::string &groupName, const std::string &version,
                    const client::config::LoggerConfig &loggerConfig);

            ~ILogger();

            void severe(const std::string &message);

            void warning(const std::string &message);

            void info(const std::string &message);

            void finest(const std::string &message);

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

			client::config::LoggerConfig& getConfig();

        private:
            const std::string instanceName;
            const std::string groupName;
            const std::string version;
            std::string prefix;
            el::Logger *easyLogger;
            client::config::LoggerConfig loggerConfig;

            void init();

            ILogger(const ILogger &);

            ILogger &operator=(const ILogger &);

        };

        /**
         * TODO: The lock may not be released on thread cancellations since the destructor may not be called.
         */
        class HAZELCAST_API LeveledLogger {
        public:
            LeveledLogger(ILogger &logger, client::LoggerLevel::Level logLevel);

            LeveledLogger(const LeveledLogger &rhs);

            virtual ~LeveledLogger();

            template<typename T>
            LeveledLogger &operator<<(const T &value) {
                if (logger.isEnabled(requestedLogLevel)) {
                    out << value;
                }
                return *this;
            }

        private:
            LeveledLogger();

            ILogger &logger;
            client::LoggerLevel::Level requestedLogLevel;
            std::ostringstream out;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#endif //HAZELCAST_ILogger
