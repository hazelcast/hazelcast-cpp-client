//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/PortableEntryEvent.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            PortableEntryEvent::PortableEntryEvent() {

            };

            PortableEntryEvent::PortableEntryEvent(const std::string& name, const connection::Member& member, EntryEventType eventType, const serialization::Data& key, const serialization::Data& value)
            : EventObject(name)
            , eventType(eventType)
            , key(key)
            , value(value) {

            };

            PortableEntryEvent::PortableEntryEvent(const std::string& name, const connection::Member& member, EntryEventType eventType, const serialization::Data& key, const serialization::Data& value, const serialization::Data& oldValue)
            : EventObject(name)
            , eventType(eventType)
            , key(key)
            , value(value)
            , oldValue(oldValue) {

            };

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

            void PortableEntryEvent::writePortable(serialization::PortableWriter& writer) const {
                int i = eventType;
                writer.writeInt("e", i);
                writer.writeUTF("u", uuid);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
                util::writeNullableData(out, &value);
                util::writeNullableData(out, &oldValue);
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
