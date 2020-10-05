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

#include <string>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/logger.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            class HAZELCAST_API LoggerConfig {
                // TODO too much typing here, alias the std::function or the signature.
            public:
                LoggerConfig();

                void logger_factory(std::function<std::unique_ptr<logger>(std::string, std::string)> make_logger) {
                    make_logger_ = std::move(make_logger);
                }

                std::function<std::unique_ptr<logger>(std::string, std::string)> logger_factory() {
                    return make_logger_;
                }

            private:
                std::function<std::unique_ptr<logger>(std::string, std::string)> make_logger_{};
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


