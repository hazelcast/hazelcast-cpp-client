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

#include "hazelcast/client/entry_event.h"
#include "hazelcast/client/map_event.h"
#include "hazelcast/client/entry_listener.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/logger.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace impl {
template<typename BaseType>
class EntryEventHandler : public BaseType
{
public:
    EntryEventHandler(
      std::string instance_name,
      spi::impl::ClientClusterServiceImpl& cluster_service,
      serialization::pimpl::SerializationService& serialization_service,
      entry_listener&& listener,
      bool include_value,
      logger& lg)
      : instance_name_(std::move(instance_name))
      , cluster_service_(cluster_service)
      , serialization_service_(serialization_service)
      , listener_(std::move(listener))
      , include_value_(include_value)
      , logger_(lg)
    {}

    void handle_entry(
      const boost::optional<serialization::pimpl::data>& key,
      const boost::optional<serialization::pimpl::data>& value,
      const boost::optional<serialization::pimpl::data>& old_value,
      const boost::optional<serialization::pimpl::data>& merging_value,
      int32_t event_type,
      boost::uuids::uuid uuid,
      int32_t number_of_affected_entries) override
    {
        if (event_type == static_cast<int32_t>(entry_event::type::EVICT_ALL) ||
            event_type == static_cast<int32_t>(entry_event::type::CLEAR_ALL)) {
            fire_map_wide_event(key,
                                value,
                                old_value,
                                merging_value,
                                event_type,
                                uuid,
                                number_of_affected_entries);
            return;
        }

        fire_entry_event(key,
                         value,
                         old_value,
                         merging_value,
                         event_type,
                         uuid,
                         number_of_affected_entries);
    }

private:
    void fire_map_wide_event(
      const boost::optional<serialization::pimpl::data>& key,
      const boost::optional<serialization::pimpl::data>& value,
      const boost::optional<serialization::pimpl::data>& old_value,
      const boost::optional<serialization::pimpl::data>& merging_value,
      int32_t event_type,
      boost::uuids::uuid uuid,
      int32_t number_of_affected_entries)
    {
        auto member = cluster_service_.get_member(uuid);
        auto mapEventType = static_cast<entry_event::type>(event_type);
        map_event mapEvent(std::move(member).value(),
                           mapEventType,
                           instance_name_,
                           number_of_affected_entries);

        if (mapEventType == entry_event::type::CLEAR_ALL) {
            listener_.map_cleared_(std::move(mapEvent));
        } else if (mapEventType == entry_event::type::EVICT_ALL) {
            listener_.map_evicted_(std::move(mapEvent));
        }
    }

    void fire_entry_event(
      const boost::optional<serialization::pimpl::data>& key,
      const boost::optional<serialization::pimpl::data>& value,
      const boost::optional<serialization::pimpl::data>& old_value,
      const boost::optional<serialization::pimpl::data>& merging_value,
      int32_t event_type,
      boost::uuids::uuid uuid,
      int32_t number_of_affected_entries)
    {
        typed_data eventKey, val, oldVal, mergingVal;
        if (include_value_) {
            if (value) {
                val = typed_data(*value, serialization_service_);
            }
            if (old_value) {
                oldVal = typed_data(*old_value, serialization_service_);
            }
            if (merging_value) {
                mergingVal = typed_data(*merging_value, serialization_service_);
            }
        }
        if (key) {
            eventKey = typed_data(*key, serialization_service_);
        }
        auto m = cluster_service_.get_member(uuid);
        if (!m.has_value()) {
            m = member(uuid);
        }
        auto type = static_cast<entry_event::type>(event_type);
        entry_event entryEvent(instance_name_,
                               std::move(m.value()),
                               type,
                               std::move(eventKey),
                               std::move(val),
                               std::move(oldVal),
                               std::move(mergingVal));
        switch (type) {
            case entry_event::type::ADDED:
                listener_.added_(std::move(entryEvent));
                break;
            case entry_event::type::REMOVED:
                listener_.removed_(std::move(entryEvent));
                break;
            case entry_event::type::UPDATED:
                listener_.updated_(std::move(entryEvent));
                break;
            case entry_event::type::EVICTED:
                listener_.evicted_(std::move(entryEvent));
                break;
            case entry_event::type::EXPIRED:
                listener_.expired_(std::move(entryEvent));
                break;
            case entry_event::type::MERGED:
                listener_.merged_(std::move(entryEvent));
                break;
            default:
                HZ_LOG(
                  logger_,
                  warning,
                  boost::str(
                    boost::format("Received unrecognized event with type: %1% "
                                  "Dropping the event!!!") %
                    static_cast<int32_t>(type)));
        }
    }

private:
    const std::string instance_name_;
    spi::impl::ClientClusterServiceImpl& cluster_service_;
    serialization::pimpl::SerializationService& serialization_service_;
    entry_listener listener_;
    bool include_value_;
    logger& logger_;
};
} // namespace impl
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
