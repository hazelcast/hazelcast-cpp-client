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
//
// Created by sancar koyunlu on 20/02/14.
//

#ifndef HAZELCAST_ILogger
#define HAZELCAST_ILogger

#include <string>
#include <iostream>
#include <sstream>
#include <mutex>

#include <easylogging++.h>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/config/LoggerConfig.h"
#include <mutex>


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace util {
        class HAZELCAST_API ILogger : public std::enable_shared_from_this<ILogger> {
        public:
            ILogger(const std::string &instanceName, const std::string &groupName, const std::string &version,
                    const client::config::LoggerConfig &loggerConfig);

            ~ILogger();

            template <typename T, typename... Targs>
            void severe(const T &value, const Targs&... fargs) {
                log(el::Level::Fatal, value, fargs...);
            }

            template <typename T, typename... Targs>
            void warning(const T &value, const Targs&... fargs) {
                log(el::Level::Warning, value, fargs...);
            }

            template <typename T, typename... Targs>
            void info(const T &value, const Targs&... fargs) {
                log(el::Level::Info, value, fargs...);
            }

            template <typename T, typename... Targs>
            void finest(const T &value, const Targs&... fargs) {
                log(el::Level::Debug, value, fargs...);
            }

            bool isEnabled(const client::LoggerLevel::Level &logLevel) const;

            /**
             * @deprecated Please use isEnabled(const client::LogLevel::Level &logLevel)
             * @param logLevel The level of the logger for which it will print the logs.
             * @return true if the level is enabled.
             */
            bool isEnabled(int logLevel) const;

            bool isFinestEnabled() const;

            bool start();

            const std::string &getInstanceName() const;

        private:
            const std::string instanceName;
            const std::string groupName;
            const std::string version;
            std::string prefix;
            el::Logger *easyLogger;
            client::config::LoggerConfig loggerConfig;
            std::once_flag elOnceflag;

            ILogger(const ILogger &);

            ILogger &operator=(const ILogger &);

            void composeMessage(std::ostringstream &out) {}

            template <typename T, typename... Targs>
            void composeMessage(std::ostringstream &out, const T &value, const Targs&... fargs) {
                out << value;
                composeMessage(out, fargs...);
            }

            template <typename T, typename... Targs>
            void log(el::Level level, const T &value, const Targs&... fargs) {
                if (!easyLogger->enabled(level)) {
                    return;
                }
                std::ostringstream out;
                composeMessage(out, value, fargs...);
                switch (level) {
                    case el::Level::Debug:
                        easyLogger->debug(out.str());
                        break;
                    case el::Level::Info:
                        easyLogger->info(out.str());
                        break;
                    case el::Level::Warning:
                        easyLogger->warn(out.str());
                        break;
                    case el::Level::Fatal:
                        easyLogger->fatal(out.str());
                        break;
                    default:
                        break;
                }
            }

        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


#endif //HAZELCAST_ILogger
