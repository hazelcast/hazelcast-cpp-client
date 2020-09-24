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
                EntryEventHandler(const std::string &instanceName, spi::impl::ClientClusterServiceImpl &clusterService,
                                  serialization::pimpl::SerializationService &serializationService,
                                  EntryListener &&listener, bool includeValue, util::ILogger &log)
                : instance_name_(instanceName), cluster_service_(clusterService), serialization_service_(serializationService)
                , listener_(std::move(listener)), include_value_(includeValue), logger_(log) {}

                void handle_entry(const boost::optional<serialization::pimpl::Data> &key,
                                  const boost::optional<serialization::pimpl::Data> &value,
                                  const boost::optional<serialization::pimpl::Data> &oldValue,
                                  const boost::optional<serialization::pimpl::Data> &mergingValue,
                                  int32_t eventType, boost::uuids::uuid uuid,
                                  int32_t numberOfAffectedEntries) override {
                    if (eventType == static_cast<int32_t>(EntryEvent::type::EVICT_ALL) || eventType == static_cast<int32_t>(EntryEvent::type::CLEAR_ALL)) {
                        fireMapWideEvent(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }

                    fireEntryEvent(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                }

            private:
                void fireMapWideEvent(const boost::optional<serialization::pimpl::Data> &key,
                                      const boost::optional<serialization::pimpl::Data> &value,
                                      const boost::optional<serialization::pimpl::Data> &oldValue,
                                      const boost::optional<serialization::pimpl::Data> &mergingValue,
                                      int32_t eventType, boost::uuids::uuid uuid,
                                      int32_t numberOfAffectedEntries) {
                    auto member = cluster_service_.getMember(uuid);
                    auto mapEventType = static_cast<EntryEvent::type>(eventType);
                    MapEvent mapEvent(std::move(member).value(), mapEventType, instance_name_, numberOfAffectedEntries);

                    if (mapEventType == EntryEvent::type::CLEAR_ALL) {
                        listener_.map_cleared_(std::move(mapEvent));
                    } else if (mapEventType == EntryEvent::type::EVICT_ALL) {
                        listener_.map_evicted_(std::move(mapEvent));
                    }
                }

                void fireEntryEvent(const boost::optional<serialization::pimpl::Data> &key,
                                    const boost::optional<serialization::pimpl::Data> &value,
                                    const boost::optional<serialization::pimpl::Data> &oldValue,
                                    const boost::optional<serialization::pimpl::Data> &mergingValue,
                                    int32_t eventType, boost::uuids::uuid uuid,
                                    int32_t numberOfAffectedEntries) {
                    TypedData eventKey, val, oldVal, mergingVal;
                    if (include_value_) {
                        if (value) {
                            val = TypedData(*value, serialization_service_);
                        }
                        if (oldValue) {
                            oldVal = TypedData(*oldValue, serialization_service_);
                        }
                        if (mergingValue) {
                            mergingVal = TypedData(*mergingValue, serialization_service_);
                        }
                    }
                    if (key) {
                        eventKey = TypedData(*key, serialization_service_);
                    }
                    auto member = cluster_service_.getMember(uuid);
                    if (!member.has_value()) {
                        member = Member(uuid);
                    }
                    auto type = static_cast<EntryEvent::type>(eventType);
                    EntryEvent entryEvent(instance_name_, std::move(member.value()), type, std::move(eventKey), std::move(val),
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
                            logger_.warning("Received unrecognized event with type: ", static_cast<int32_t>(type),
                                           " Dropping the event!!!");
                    }
                }
            private:
                const std::string& instance_name_;
                spi::impl::ClientClusterServiceImpl &cluster_service_;
                serialization::pimpl::SerializationService& serialization_service_;
                EntryListener listener_;
                bool include_value_;
                util::ILogger &logger_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

