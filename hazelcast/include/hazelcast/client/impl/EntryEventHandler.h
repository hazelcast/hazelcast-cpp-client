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

#include <memory>

#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/MapEvent.h"
#include "hazelcast/client/EntryListener.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/logger.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            template<typename BaseType>
            class EntryEventHandler : public BaseType {
            public:
                EntryEventHandler(const std::string &instance_name, spi::impl::ClientClusterServiceImpl &cluster_service,
                                  serialization::pimpl::SerializationService &serialization_service,
                                  EntryListener &&listener, bool include_value, logger &lg)
                : instanceName_(instance_name), clusterService_(cluster_service), serializationService_(serialization_service)
                , listener_(std::move(listener)), includeValue_(include_value), logger_(lg) {}

                void handle_entry(const boost::optional<serialization::pimpl::Data> &key,
                                  const boost::optional<serialization::pimpl::Data> &value,
                                  const boost::optional<serialization::pimpl::Data> &old_value,
                                  const boost::optional<serialization::pimpl::Data> &merging_value,
                                  int32_t event_type, boost::uuids::uuid uuid,
                                  int32_t number_of_affected_entries) override {
                    if (event_type == static_cast<int32_t>(EntryEvent::type::EVICT_ALL) || event_type == static_cast<int32_t>(EntryEvent::type::CLEAR_ALL)) {
                        fire_map_wide_event(key, value, old_value, merging_value, event_type, uuid, number_of_affected_entries);
                        return;
                    }

                    fire_entry_event(key, value, old_value, merging_value, event_type, uuid, number_of_affected_entries);
                }

            private:
                void fire_map_wide_event(const boost::optional<serialization::pimpl::Data> &key,
                                      const boost::optional<serialization::pimpl::Data> &value,
                                      const boost::optional<serialization::pimpl::Data> &old_value,
                                      const boost::optional<serialization::pimpl::Data> &merging_value,
                                      int32_t event_type, boost::uuids::uuid uuid,
                                      int32_t number_of_affected_entries) {
                    auto member = clusterService_.get_member(uuid);
                    auto mapEventType = static_cast<EntryEvent::type>(event_type);
                    MapEvent mapEvent(std::move(member).value(), mapEventType, instanceName_, number_of_affected_entries);

                    if (mapEventType == EntryEvent::type::CLEAR_ALL) {
                        listener_.map_cleared_(std::move(mapEvent));
                    } else if (mapEventType == EntryEvent::type::EVICT_ALL) {
                        listener_.map_evicted_(std::move(mapEvent));
                    }
                }

                void fire_entry_event(const boost::optional<serialization::pimpl::Data> &key,
                                    const boost::optional<serialization::pimpl::Data> &value,
                                    const boost::optional<serialization::pimpl::Data> &old_value,
                                    const boost::optional<serialization::pimpl::Data> &merging_value,
                                    int32_t event_type, boost::uuids::uuid uuid,
                                    int32_t number_of_affected_entries) {
                    TypedData eventKey, val, oldVal, mergingVal;
                    if (includeValue_) {
                        if (value) {
                            val = TypedData(*value, serializationService_);
                        }
                        if (old_value) {
                            oldVal = TypedData(*old_value, serializationService_);
                        }
                        if (merging_value) {
                            mergingVal = TypedData(*merging_value, serializationService_);
                        }
                    }
                    if (key) {
                        eventKey = TypedData(*key, serializationService_);
                    }
                    auto member = clusterService_.get_member(uuid);
                    if (!member.has_value()) {
                        member = Member(uuid);
                    }
                    auto type = static_cast<EntryEvent::type>(event_type);
                    EntryEvent entryEvent(instanceName_, std::move(member.value()), type, std::move(eventKey), std::move(val),
                                          std::move(oldVal), std::move(mergingVal));
                    switch(type) {
                        case EntryEvent::type::ADDED:
                            listener_.added_(std::move(entryEvent));
                            break;
                        case EntryEvent::type::REMOVED:
                            listener_.removed_(std::move(entryEvent));
                            break;
                        case EntryEvent::type::UPDATED:
                            listener_.updated_(std::move(entryEvent));
                            break;
                        case EntryEvent::type::EVICTED:
                            listener_.evicted_(std::move(entryEvent));
                            break;
                        case EntryEvent::type::EXPIRED:
                            listener_.expired_(std::move(entryEvent));
                            break;
                        case EntryEvent::type::MERGED:
                            listener_.merged_(std::move(entryEvent));
                            break;
                        default:
                            HZ_LOG(logger_, warning,
                                boost::str(boost::format("Received unrecognized event with type: %1% "
                                                          "Dropping the event!!!")
                                                          % static_cast<int32_t>(type))
                            );
                    }
                }
            private:
                const std::string& instanceName_;
                spi::impl::ClientClusterServiceImpl &clusterService_;
                serialization::pimpl::SerializationService& serializationService_;
                EntryListener listener_;
                bool includeValue_;
                logger &logger_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

