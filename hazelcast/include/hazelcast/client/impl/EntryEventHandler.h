//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ENTRY_EVENT_HANDLER
#define HAZELCAST_ENTRY_EVENT_HANDLER

#include "PortableEntryEvent.h"
#include "ClusterService.h"
#include "EntryEvent.h"
#include "../serialization/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            template<typename K, typename V, typename L>
            class EntryEventHandler {
            public:
                EntryEventHandler(const std::string& instanceName, spi::ClusterService& clusterService, serialization::SerializationService& serializationService, L& listener, bool includeValue)
                :instanceName(instanceName)
                , clusterService(clusterService)
                , serializationService(serializationService)
                , listener(listener)
                , includeValue(includeValue) {

                };

                void handle(const PortableEntryEvent& event) {

                    V value;
                    V oldValue;
                    if (includeValue) {
                        value = serializationService.toObject<V>(event.getValue());
                        oldValue = serializationService.toObject<V>(event.getOldValue());
                    }
                    K key = serializationService.toObject<K>(event.getKey());
                    connection::Member member = clusterService.getMember(event.getUuid());
                    EntryEventType type = event.getEventType();
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

                };
            private:
                const std::string& instanceName;
                serialization::SerializationService& serializationService;
                spi::ClusterService& clusterService;
                L& listener;
                bool includeValue;
            };
        }
    }
}

#endif //__EntryEventHandler_H_
