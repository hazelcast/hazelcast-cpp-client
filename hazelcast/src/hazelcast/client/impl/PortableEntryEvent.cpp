//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/PortableEntryEvent.h"

namespace hazelcast {
    namespace client {
        namespace impl {


            const serialization::Data& PortableEntryEvent::getKey() const {
                return key;
            }

            const serialization::Data& PortableEntryEvent::getOldValue() const {
                return oldValue;
            }

            const serialization::Data& PortableEntryEvent::getValue() const {
                return value;
            }

            std::string PortableEntryEvent::getUuid() const {
                return uuid;
            }

            EntryEventType PortableEntryEvent::getEventType() const {
                return eventType;
            }

            std::string PortableEntryEvent::getName() const {
                return getSource();
            }

            int PortableEntryEvent::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            };

            int PortableEntryEvent::getClassId() const {
                return protocol::SpiConstants::ENTRY_EVENT;
            };
        }
    }
}
