/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
class member;

namespace topic {
class HAZELCAST_API message
{
public:
    message(const std::string& topic_name,
            typed_data&& msg,
            int64_t publish_time,
            boost::optional<member>&& member)
      : message(topic_name,
                std::move(msg),
                std::chrono::system_clock::from_time_t(
                  std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::milliseconds(publish_time))
                    .count()),
                std::move(member))
    {}

    message(std::string topic_name,
            typed_data&& msg,
            std::chrono::system_clock::time_point publish_time,
            boost::optional<member>&& member)
      : message_object_(msg)
      , publish_time_(publish_time)
      , publishing_member_(member)
      , name_(std::move(topic_name))
    {}

    const typed_data& get_message_object() const { return message_object_; }

    std::chrono::system_clock::time_point get_publish_time() const
    {
        return publish_time_;
    }

    const member* get_publishing_member() const
    {
        return publishing_member_.get_ptr();
    }

    const std::string& get_source() const { return name_; }

    const std::string& get_name() const { return name_; }

private:
    typed_data message_object_;
    std::chrono::system_clock::time_point publish_time_;
    boost::optional<member> publishing_member_;
    std::string name_;
};
} // namespace topic
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
