//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/PortableEntryEvent.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            const serialization::pimpl::Data& PortableEntryEvent::getKey() const {
                return key;
            }

            const serialization::pimpl::Data& PortableEntryEvent::getOldValue() const {
                return oldValue;
            }

            const serialization::pimpl::Data& PortableEntryEvent::getValue() const {
                return value;
            }

            std::string PortableEntryEvent::getUuid() const {
                return uuid;
            }

            EntryEventType PortableEntryEvent::getEventType() const {
                return eventType;
            }

            std::string PortableEntryEvent::getName() const {
                return name;
            }

            int PortableEntryEvent::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            };

            int PortableEntryEvent::getClassId() const {
                return protocol::SpiConstants::ENTRY_EVENT;
            };


            void PortableEntryEvent::readPortable(serialization::PortableReader& reader) {
                eventType = reader.readInt("e");
                uuid = reader.readUTF("u");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
                util::readNullableData(in, &value);
                util::readNullableData(in, &oldValue);
            };
        }
    }
}

