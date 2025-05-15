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

#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/topic/message.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/topic/listener.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace topic {
namespace impl {
class TopicEventHandlerImpl
  : public protocol::codec::topic_addmessagelistener_handler
{
public:
    TopicEventHandlerImpl(
      const std::string& instance_name,
      logger &logger,
      spi::impl::ClientClusterServiceImpl& cluster_service,
      serialization::pimpl::SerializationService& serialization_service,
      listener&& message_listener)
      : protocol::codec::topic_addmessagelistener_handler(logger)
      , instance_name_(instance_name)
      , cluster_service_(cluster_service)
      , serialization_service_(serialization_service)
      , listener_(std::move(message_listener))
    {}

    void handle_topic(serialization::pimpl::data const& item,
                      int64_t publish_time,
                      boost::uuids::uuid uuid) override
    {
        listener_.received_(
          message(instance_name_,
                  typed_data(std::move(item), serialization_service_),
                  publish_time,
                  cluster_service_.get_member(uuid)));
    }

private:
    std::string instance_name_;
    spi::impl::ClientClusterServiceImpl& cluster_service_;
    serialization::pimpl::SerializationService& serialization_service_;
    listener listener_;
};
} // namespace impl
} // namespace topic
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
