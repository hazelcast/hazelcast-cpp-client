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
#pragma once

#include <stdexcept>
#include <string>

#include "hazelcast/logger.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Preconditions.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            class HAZELCAST_API LoggerConfig {
            public:
                /**
                 * Minimum level of log messages to be printed.
                 * Log messages with a severity level below the minimum level will be ignored.
                 * \return minimum severity level 
                 */
                logger::level min_level() {
                    return min_level_;
                }

                /**
                 * Set the minimum severity level of log messages to be printed.
                 * Log messages with a severity level below the minimum level will be ignored.
                 * \return *this
                 */
                LoggerConfig &min_level(logger::level level) {
                    min_level_ = level;
                    return *this;
                }


                /**
                 * \return log handler function 
                 */
                logger::handler_type handler() {
                    return handler_;
                }

                /**
                 * Set a log handler function to be invoked on each log message.
                 * Setting this config will cause the default logging behaviour to be disabled.
                 * The handler function takes the instance and cluster name of the client, the file
                 * and the line number from which the log was emitted, the severity level, and the
                 * log message. 
                 * \warning The handler function must be thread-safe.
                 * \return *this
                 */
                LoggerConfig &handler(logger::handler_type handler) {
                    util::Preconditions::checkTrue(handler, "log handler may not be empty");
                    handler_ = std::move(handler);
                    return *this;
                }

            private:
                logger::level min_level_{ logger::level::info };
                logger::handler_type handler_{ logger::default_handler };
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


