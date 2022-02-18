/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include <memory>

#include "hazelcast/client/serialization/pimpl/data.h"
#include "hazelcast/client/topic/message.h"
#include "hazelcast/client/member.h"
#include "hazelcast/util/export.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace topic {
namespace impl {
namespace reliable {
class HAZELCAST_API ReliableTopicMessage
{
    friend struct serialization::hz_serializer<
      topic::impl::reliable::ReliableTopicMessage>;

public:
    ReliableTopicMessage();

    ReliableTopicMessage(serialization::pimpl::data&& payload_data,
                         std::unique_ptr<address> address);

    std::chrono::system_clock::time_point get_publish_time() const;

    const boost::optional<address>& get_publisher_address() const;

    serialization::pimpl::data& get_payload();

private:
    std::chrono::system_clock::time_point publish_time_;
    boost::optional<address> publisher_address_;
    serialization::pimpl::data payload_;
};
} // namespace reliable
} // namespace impl
} // namespace topic
namespace serialization {
template<>
struct HAZELCAST_API hz_serializer<topic::impl::reliable::ReliableTopicMessage>
  : public identified_data_serializer
{
    static constexpr int32_t F_ID = -9;
    static constexpr int32_t RELIABLE_TOPIC_MESSAGE = 2;

    static int32_t get_factory_id();

    static int32_t get_class_id();

    static void write_data(
      const topic::impl::reliable::ReliableTopicMessage& object,
      object_data_output& out);

    static topic::impl::reliable::ReliableTopicMessage read_data(
      object_data_input& in);
};
} // namespace serialization
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
