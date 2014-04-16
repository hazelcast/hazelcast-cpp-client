//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ITEM_EVENT_HANDLER
#define HAZELCAST_ITEM_EVENT_HANDLER

#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/impl/PortableItemEvent.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/ItemEvent.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/impl/BaseEventHandler.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            template<typename E, typename L>
            class HAZELCAST_API ItemEventHandler : public impl::BaseEventHandler {
            public:
                ItemEventHandler(const std::string &instanceName, spi::ClusterService &clusterService, serialization::pimpl::SerializationService &serializationService, L &listener, bool includeValue)
                :instanceName(instanceName)
                , clusterService(clusterService)
                , serializationService(serializationService)
                , listener(listener)
                , includeValue(includeValue) {

                };

                void handle(const client::serialization::pimpl::Data &data) {
                    boost::shared_ptr<PortableItemEvent> event = serializationService.toObject<PortableItemEvent>(data);
                    handle(*event);
                }

                void handle(const PortableItemEvent &event) {
                    boost::shared_ptr<E> item;
                    if (includeValue) {
                        item = serializationService.toObject<E>(event.getItem());
                    }
                    Member member = clusterService.getMember(event.getUuid());
                    ItemEventType type = event.getEventType();
                    ItemEvent<E> itemEvent(instanceName, type, *item, member);
                    if (type == EntryEventType::ADDED) {
                        listener.itemAdded(itemEvent);
                    } else if (type == EntryEventType::REMOVED) {
                        listener.itemRemoved(itemEvent);
                    }

                };
            private:
                const std::string &instanceName;
                spi::ClusterService &clusterService;
                serialization::pimpl::SerializationService &serializationService;
                L &listener;
                bool includeValue;
            };
        }
    }
}

#endif //HAZELCAST_ITEM_EVENT_HANDLER

