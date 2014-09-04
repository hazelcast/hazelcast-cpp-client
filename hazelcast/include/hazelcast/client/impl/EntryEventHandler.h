//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ENTRY_EVENT_HANDLER
#define HAZELCAST_ENTRY_EVENT_HANDLER

#include "hazelcast/client/impl/PortableEntryEvent.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/IMapEvent.h"
#include "hazelcast/client/EntryListener.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            template<typename K, typename V>
            class HAZELCAST_API EntryEventHandler : public BaseEventHandler {
            public:
                EntryEventHandler(const std::string& instanceName, spi::ClusterService& clusterService, serialization::pimpl::SerializationService& serializationService, EntryListener<K, V>& listener, bool includeValue)
                : instanceName(instanceName)
                , clusterService(clusterService)
                , serializationService(serializationService)
                , listener(listener)
                , includeValue(includeValue) {

                };

                void handle(const client::serialization::pimpl::Data& data) {
                    boost::shared_ptr<PortableEntryEvent> event = serializationService.toObject<PortableEntryEvent>(data);
                    handle(*event);
                }

                void handle(const PortableEntryEvent& event) {
                    EntryEventType type = event.getEventType();
                    if (type == EntryEventType::EVICT_ALL || type == EntryEventType::CLEAR_ALL) {
                        fireMapEvent(event);
                        return;
                    }
                    fireEntryEvent(event);
                }

                void fireMapEvent(const PortableEntryEvent& event) {
                    Member member = clusterService.getMember(event.getUuid());
                    EntryEventType type = event.getEventType();
                    IMapEvent mapEvent(member, type, instanceName, event.getNumberOfAffectedEntries());
                    if (type == EntryEventType::CLEAR_ALL) {
                        listener.mapCleared(mapEvent);
                    } else if (type == EntryEventType::EVICT_ALL) {
                        listener.mapEvicted(mapEvent);
                    }
                }

                void fireEntryEvent(const PortableEntryEvent& event) {
                    EntryEventType type = event.getEventType();
                    boost::shared_ptr<V> value;
                    boost::shared_ptr<V> oldValue;
                    if (includeValue) {
                        value = serializationService.toObject<V>(event.getValue());
                        oldValue = serializationService.toObject<V>(event.getOldValue());
                    }
                    boost::shared_ptr<K> key = serializationService.toObject<K>(event.getKey());
                    Member member = clusterService.getMember(event.getUuid());
                    EntryEvent<K, V> entryEvent(instanceName, member, type, key, value, oldValue);
                    if (type == EntryEventType::ADDED) {
                        listener.entryAdded(entryEvent);
                    } else if (type == EntryEventType::REMOVED) {
                        listener.entryRemoved(entryEvent);
                    } else if (type == EntryEventType::UPDATED) {
                        listener.entryUpdated(entryEvent);
                    } else if (type == EntryEventType::EVICTED) {
                        listener.entryEvicted(entryEvent);
                    }
                }

            private:
                const std::string& instanceName;
                spi::ClusterService& clusterService;
                serialization::pimpl::SerializationService& serializationService;
                EntryListener<K, V>& listener;
                bool includeValue;
            };
        }
    }
}

#endif //__EntryEventHandler_H_

