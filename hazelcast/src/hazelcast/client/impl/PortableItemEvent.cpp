//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "PortableItemEvent.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            PortableItemEvent::PortableItemEvent(PortableItemEvent::ItemEventType eventType, const serialization::Data& item, const std::string& uuid)
            :eventType(eventType)
            , item(item)
            , uuid(uuid) {

            };

            const serialization::Data& PortableItemEvent::getItem() const {
                return item;
            };

            std::string PortableItemEvent::getUuid() const {
                return uuid;
            };

            PortableItemEvent::ItemEventType PortableItemEvent::getEventType() const {
                return eventType;
            }

            int PortableItemEvent::getSerializerId() const {
                return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
            };

            int PortableItemEvent::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            };

            int PortableItemEvent::getClassId() const {
                return protocol::SpiConstants::ITEM_EVENT;
            };


        }
    }
}