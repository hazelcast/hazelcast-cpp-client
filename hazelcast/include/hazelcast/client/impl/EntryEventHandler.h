/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 6/24/13.

#ifndef HAZELCAST_ENTRY_EVENT_HANDLER
#define HAZELCAST_ENTRY_EVENT_HANDLER

#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/MapEvent.h"
#include "hazelcast/client/EntryListener.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/protocol/codec/MapAddEntryListenerCodec.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {

            template<typename K, typename V, typename BaseType>
            class EntryEventHandler : public BaseType {
            public:
                EntryEventHandler(const std::string &instanceName, spi::ClusterService &clusterService,
                                  serialization::pimpl::SerializationService &serializationService,
                                  EntryListener<K, V> &listener, bool includeValue)
                : instanceName(instanceName)
                , clusterService(clusterService)
                , serializationService(serializationService)
                , listener(listener)
                , includeValue(includeValue) {
                }

                virtual void handleEntry(std::auto_ptr<serialization::pimpl::Data> key,
                                         std::auto_ptr<serialization::pimpl::Data> value,
                                         std::auto_ptr<serialization::pimpl::Data> oldValue,
                                         std::auto_ptr<serialization::pimpl::Data> mergingValue,
                                         const int32_t &eventType, const std::string &uuid,
                                         const int32_t &numberOfAffectedEntries) {
                    if (eventType == EntryEventType::EVICT_ALL || eventType == EntryEventType::CLEAR_ALL) {
                        fireMapWideEvent(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }

                    fireEntryEvent(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                }

            private:
                void fireMapWideEvent(std::auto_ptr<serialization::pimpl::Data> key,
                                      std::auto_ptr<serialization::pimpl::Data> value,
                                      std::auto_ptr<serialization::pimpl::Data> oldValue,
                                      std::auto_ptr<serialization::pimpl::Data> mergingValue,
                                      const int32_t &eventType, const std::string &uuid,
                                      const int32_t &numberOfAffectedEntries) {
                    std::auto_ptr<Member> member = clusterService.getMember(uuid);

                    MapEvent mapEvent(*member, (EntryEventType::Type)eventType, instanceName, numberOfAffectedEntries);

                    if (eventType == EntryEventType::CLEAR_ALL) {
                        listener.mapCleared(mapEvent);
                    } else if (eventType == EntryEventType::EVICT_ALL) {
                        listener.mapEvicted(mapEvent);
                    }
                }

                void fireEntryEvent(std::auto_ptr<serialization::pimpl::Data> key,
                                    std::auto_ptr<serialization::pimpl::Data> value,
                                    std::auto_ptr<serialization::pimpl::Data> oldValue,
                                    std::auto_ptr<serialization::pimpl::Data> mergingValue,
                                    const int32_t &eventType, const std::string &uuid,
                                    const int32_t &numberOfAffectedEntries) {
                    EntryEventType type((EntryEventType::Type)eventType);
                    std::auto_ptr<V> val;
                    std::auto_ptr<V> oldVal;
                    std::auto_ptr<V> mergingVal;
                    if (includeValue) {
                        if (NULL != value.get()) {
                            val = serializationService.toObject<V>(*value);
                        }
                        if (NULL != oldValue.get()) {
                            oldVal = serializationService.toObject<V>(*oldValue);
                        }
                        if (NULL != mergingValue.get()) {
                            mergingVal = serializationService.toObject<V>(*mergingValue);
                        }
                    }
                    std::auto_ptr<K> eventKey;
                    if (NULL != key.get()) {
                        eventKey = serializationService.toObject<K>(*key);
                    }
                    std::auto_ptr<Member> member = clusterService.getMember(uuid);
                    EntryEvent<K, V> entryEvent(instanceName, *member, type, eventKey, val, oldVal, mergingVal);
                    if (type == EntryEventType::ADDED) {
                        listener.entryAdded(entryEvent);
                    } else if (type == EntryEventType::REMOVED) {
                        listener.entryRemoved(entryEvent);
                    } else if (type == EntryEventType::UPDATED) {
                        listener.entryUpdated(entryEvent);
                    } else if (type == EntryEventType::EVICTED) {
                        listener.entryEvicted(entryEvent);
                    } else if (type == EntryEventType::EXPIRED) {
                        listener.entryExpired(entryEvent);
                    } else if (type == EntryEventType::MERGED) {
                        listener.entryMerged(entryEvent);
                    }
                }

            private:
                const std::string& instanceName;
                spi::ClusterService& clusterService;
                serialization::pimpl::SerializationService& serializationService;
                EntryListener<K, V>& listener;
                bool includeValue;
            };

            template<typename BaseType>
            class MixedEntryEventHandler : public BaseType {
            public:
                MixedEntryEventHandler(const std::string &instanceName, spi::ClusterService &clusterService,
                                  serialization::pimpl::SerializationService &serializationService,
                                  MixedEntryListener &listener, bool includeValue)
                        : instanceName(instanceName)
                        , clusterService(clusterService)
                        , serializationService(serializationService)
                        , listener(listener)
                        , includeValue(includeValue) {
                }

                virtual void handleEntry(std::auto_ptr<serialization::pimpl::Data> key,
                                         std::auto_ptr<serialization::pimpl::Data> value,
                                         std::auto_ptr<serialization::pimpl::Data> oldValue,
                                         std::auto_ptr<serialization::pimpl::Data> mergingValue,
                                         const int32_t &eventType, const std::string &uuid,
                                         const int32_t &numberOfAffectedEntries) {
                    if (eventType == EntryEventType::EVICT_ALL || eventType == EntryEventType::CLEAR_ALL) {
                        fireMapWideEvent(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }

                    fireEntryEvent(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                }

            private:
                void fireMapWideEvent(std::auto_ptr<serialization::pimpl::Data> key,
                                      std::auto_ptr<serialization::pimpl::Data> value,
                                      std::auto_ptr<serialization::pimpl::Data> oldValue,
                                      std::auto_ptr<serialization::pimpl::Data> mergingValue,
                                      const int32_t &eventType, const std::string &uuid,
                                      const int32_t &numberOfAffectedEntries) {
                    std::auto_ptr<Member> member = clusterService.getMember(uuid);

                    MapEvent mapEvent(*member, (EntryEventType::Type)eventType, instanceName, numberOfAffectedEntries);

                    if (eventType == EntryEventType::CLEAR_ALL) {
                        listener.mapCleared(mapEvent);
                    } else if (eventType == EntryEventType::EVICT_ALL) {
                        listener.mapEvicted(mapEvent);
                    }
                }

                void fireEntryEvent(std::auto_ptr<serialization::pimpl::Data> key,
                                    std::auto_ptr<serialization::pimpl::Data> value,
                                    std::auto_ptr<serialization::pimpl::Data> oldValue,
                                    std::auto_ptr<serialization::pimpl::Data> mergingValue,
                                    const int32_t &eventType, const std::string &uuid,
                                    const int32_t &numberOfAffectedEntries) {
                    EntryEventType type((EntryEventType::Type)eventType);
                    std::auto_ptr<Member> member = clusterService.getMember(uuid);
                    MixedEntryEvent entryEvent(instanceName, *member, type, TypedData(key, serializationService),
                                               TypedData(value, serializationService),
                                               TypedData(oldValue, serializationService),
                                               TypedData(mergingValue, serializationService));
                    if (type == EntryEventType::ADDED) {
                        listener.entryAdded(entryEvent);
                    } else if (type == EntryEventType::REMOVED) {
                        listener.entryRemoved(entryEvent);
                    } else if (type == EntryEventType::UPDATED) {
                        listener.entryUpdated(entryEvent);
                    } else if (type == EntryEventType::EVICTED) {
                        listener.entryEvicted(entryEvent);
                    } else if (type == EntryEventType::EXPIRED) {
                        listener.entryExpired(entryEvent);
                    } else if (type == EntryEventType::MERGED) {
                        listener.entryMerged(entryEvent);
                    }
                }

            private:
                const std::string& instanceName;
                spi::ClusterService& clusterService;
                serialization::pimpl::SerializationService& serializationService;
                MixedEntryListener& listener;
                bool includeValue;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //__EntryEventHandler_H_

