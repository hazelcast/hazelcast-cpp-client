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

#include <utility>

#include "hazelcast/client/serialization/serialization.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class Member;

        namespace topic {
            class HAZELCAST_API Message {
            public:
                Message(const std::string &topic_name, TypedData &&message, int64_t publish_time,
                        boost::optional<Member> &&member) : Message(topic_name, std::move(message),
                                std::chrono::system_clock::from_time_t(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(publish_time)).count()),
                                std::move(member)) {}

                Message(std::string topic_name, TypedData &&message, std::chrono::system_clock::time_point publish_time,
                        boost::optional<Member> &&member)
                        : messageObject_(message), publishTime_(publish_time), publishingMember_(member), name_(std::move(topic_name)) {}

                const TypedData &get_message_object() const {
                    return messageObject_;
                }

                std::chrono::system_clock::time_point get_publish_time() const {
                    return publishTime_;
                }

                const Member *get_publishing_member() const {
                    return publishingMember_.get_ptr();
                }

                const std::string &get_source() const {
                    return name_;
                }

                const std::string &get_name() const {
                    return name_;
                }

            private:
                TypedData messageObject_;
                std::chrono::system_clock::time_point publishTime_;
                boost::optional<Member> publishingMember_;
                std::string name_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

