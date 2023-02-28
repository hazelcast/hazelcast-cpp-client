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

#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/item_listener.h"
#include "hazelcast/client/item_event.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/impl/BaseEventHandler.h"

namespace hazelcast {
namespace client {
namespace impl {
template<typename BaseType>
class item_event_handler : public BaseType
{
public:
    item_event_handler(
      std::string instance_name,
      spi::impl::ClientClusterServiceImpl& cluster_service,
      serialization::pimpl::SerializationService& serialization_service,
      item_listener&& listener,
      bool include_value)
      : instance_name_(std::move(instance_name))
      , cluster_service_(cluster_service)
      , serialization_service_(serialization_service)
      , listener_(std::move(listener))
      , include_value_(include_value){};

    void handle_item(const boost::optional<serialization::pimpl::data>& item,
                     boost::uuids::uuid uuid,
                     int32_t event_type) override
    {
        typed_data val;
        if (include_value_) {
            val = typed_data(std::move(*item), serialization_service_);
        }
        auto member = cluster_service_.get_member(uuid);
        item_event_type type(static_cast<item_event_type>(event_type));
        item_event itemEvent(
          instance_name_, type, std::move(val), std::move(member).value());
        if (type == item_event_type::ADDED) {
            listener_.added_(std::move(itemEvent));
        } else if (type == item_event_type::REMOVED) {
            listener_.removed_(std::move(itemEvent));
        }
    }

private:
    const std::string instance_name_;
    spi::impl::ClientClusterServiceImpl& cluster_service_;
    serialization::pimpl::SerializationService& serialization_service_;
    item_listener listener_;
    bool include_value_;
};
} // namespace impl
} // namespace client
} // namespace hazelcast
