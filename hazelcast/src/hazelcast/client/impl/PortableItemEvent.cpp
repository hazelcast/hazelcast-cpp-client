//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/PortableItemEvent.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            PortableItemEvent::PortableItemEvent() {

            }

            PortableItemEvent::PortableItemEvent(ItemEventType eventType, const serialization::Data& item, const std::string& uuid)
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

            ItemEventType PortableItemEvent::getEventType() const {
                return eventType;
            }

            int PortableItemEvent::getFactoryId() const {
                return protocol::SpiConstants::SPI_PORTABLE_FACTORY;
            };

            int PortableItemEvent::getClassId() const {
                return protocol::SpiConstants::ITEM_EVENT;
            };

            void PortableItemEvent::writePortable(serialization::PortableWriter& writer) const {
                writer.writeInt("e", eventType);
                writer.writeUTF("u", uuid);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                util::writeNullableData(out, &item);
            };

            void PortableItemEvent::readPortable(serialization::PortableReader& reader) {
                eventType = reader.readInt("e");;
                uuid = reader.readUTF("u");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                util::readNullableData(in, &item);
            };

        }
    }
}