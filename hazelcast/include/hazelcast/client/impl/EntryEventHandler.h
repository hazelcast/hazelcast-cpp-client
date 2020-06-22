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
#include <assert.h>

#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/MapEvent.h"
#include "hazelcast/client/EntryListener.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            template<typename Listener, typename BaseType>
            class EntryEventHandler : public BaseType {
            public:
                EntryEventHandler(const std::string &instanceName, spi::ClientClusterService &clusterService,
                                  serialization::pimpl::SerializationService &serializationService,
                                  Listener &&listener, bool includeValue, util::ILogger &log)
                : instanceName(instanceName), clusterService(clusterService), serializationService(serializationService)
                , listener(listener), includeValue(includeValue), logger(log) {}

                virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                         std::unique_ptr<serialization::pimpl::Data> &value,
                                         std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                         std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                         const int32_t &eventType, const std::string &uuid,
                                         const int32_t &numberOfAffectedEntries) {
                    if (eventType == static_cast<int32_t>(EntryEvent::type::EVICT_ALL) || eventType == static_cast<int32_t>(EntryEvent::type::CLEAR_ALL)) {
                        fireMapWideEvent(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }

                    fireEntryEvent(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                }

            private:
                void fireMapWideEvent(std::unique_ptr<serialization::pimpl::Data> &key,
                                      std::unique_ptr<serialization::pimpl::Data> &value,
                                      std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                      std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                      const int32_t &eventType, const std::string &uuid,
                                      const int32_t &numberOfAffectedEntries) {
                    auto member = clusterService.getMember(uuid);
                    auto mapEventType = static_cast<EntryEvent::type>(eventType);
                    MapEvent mapEvent(std::move(member).value(), mapEventType, instanceName, numberOfAffectedEntries);

                    if (mapEventType == EntryEvent::type::CLEAR_ALL) {
                        listener.mapCleared(mapEvent);
                    } else if (mapEventType == EntryEvent::type::EVICT_ALL) {
                        listener.mapEvicted(mapEvent);
                    }
                }

                void fireEntryEvent(std::unique_ptr<serialization::pimpl::Data> &key,
                                    std::unique_ptr<serialization::pimpl::Data> &value,
                                    std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                    std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                    const int32_t &eventType, const std::string &uuid,
                                    const int32_t &numberOfAffectedEntries) {
                    TypedData eventKey, val, oldVal, mergingVal;
                    if (includeValue) {
                        if (value) {
                            val = TypedData(std::move(*value), serializationService);
                        }
                        if (oldValue) {
                            oldVal = TypedData(std::move(*oldValue), serializationService);
                        }
                        if (mergingValue) {
                            mergingVal = TypedData(std::move(*mergingValue), serializationService);
                        }
                    }
                    if (key) {
                        eventKey = TypedData(std::move(*key), serializationService);
                    }
                    auto member = clusterService.getMember(uuid);
                    if (!member.has_value()) {
                        member = Member(uuid);
                    }
                    auto type = static_cast<EntryEvent::type>(eventType);
                    EntryEvent entryEvent(instanceName, member.value(), type, std::move(eventKey), std::move(val),
                                          std::move(oldVal), std::move(mergingVal));
                    switch(type) {
                        case EntryEvent::type::ADDED:
                            listener.entryAdded(entryEvent);
                            break;
                        case EntryEvent::type::REMOVED:
                            listener.entryRemoved(entryEvent);
                            break;
                        case EntryEvent::type::UPDATED:
                            listener.entryUpdated(entryEvent);
                            break;
                        case EntryEvent::type::EVICTED:
                            listener.entryEvicted(entryEvent);
                            break;
                        case EntryEvent::type::EXPIRED:
                            listener.entryExpired(entryEvent);
                            break;
                        case EntryEvent::type::MERGED:
                            listener.entryMerged(entryEvent);
                            break;
                        default:
                            logger.warning("Received unrecognized event with type: ", static_cast<int32_t>(type),
                                           " Dropping the event!!!");
                    }
                }
            private:
                const std::string& instanceName;
                spi::ClientClusterService &clusterService;
                serialization::pimpl::SerializationService& serializationService;
                Listener listener;
                bool includeValue;
                util::ILogger &logger;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

