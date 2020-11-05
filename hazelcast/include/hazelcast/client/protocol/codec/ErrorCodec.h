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
#include <vector>
#include <boost/optional.hpp>
#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;

            namespace codec {
                struct HAZELCAST_API StackTraceElement {
                    std::string declaring_class;
                    std::string method_name;
                    boost::optional<std::string> file_name;
                    int line_number;
                };

                struct HAZELCAST_API ErrorHolder {
                    int32_t error_code;
                    std::string class_name;
                    boost::optional<std::string> message;
                    std::vector<codec::StackTraceElement> stack_trace;

                    std::string toString() const;
                };

                std::ostream &operator<<(std::ostream &out, const StackTraceElement &trace);

                struct HAZELCAST_API ErrorCodec {
                    static constexpr int32_t EXCEPTION_MESSAGE_TYPE = 0;

                    static std::vector<ErrorHolder> decode(ClientMessage &clientMessage);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
