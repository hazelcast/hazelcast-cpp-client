//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_PORTABLE_ENTRY_EVENT
#define HAZELCAST_PORTABLE_ENTRY_EVENT

#include "EventObject.h"
#include "../connection/Member.h"
#include "EntryEvent.h"
#include "Data.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"
#include "Portable.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class PortableEntryEvent : public EventObject, public Portable {
            public:


                PortableEntryEvent() {

                };

                PortableEntryEvent(const std::string& name, const connection::Member& member, EntryEventType eventType, const serialization::Data& key, const serialization::Data& value)
                : EventObject(name)
                , eventType(eventType)
                , key(key)
                , value(value) {

                };

                PortableEntryEvent(const std::string& name, const connection::Member& member, EntryEventType eventType, const serialization::Data& key, const serialization::Data& value, const serialization::Data& oldValue)
                : EventObject(name)
                , eventType(eventType)
                , key(key)
                , value(value)
                , oldValue(oldValue) {

                };

                const serialization::Data&  getKey() const;

                const serialization::Data&  getOldValue() const;

                const serialization::Data&  getValue() const;

                std::string getUuid() const;

                EntryEventType getEventType() const;

                std::string getName() const;

                int getFactoryId() const;

                int getClassId() const;


                void writePortable(serialization::PortableWriter& writer) const {
                    int i = eventType;
                    writer.writeInt("e", i);
                    writer.writeUTF("u", uuid);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                    util::writeNullableData(out, &value);
                    util::writeNullableData(out, &oldValue);
                };


                void readPortable(serialization::PortableReader& reader) {
                    eventType = reader.readInt("e");
                    uuid = reader.readUTF("u");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    key.readData(in);
                    util::readNullableData(in, &value);
                    util::readNullableData(in, &oldValue);
                };

            private:
                serialization::Data key;
                serialization::Data value;
                serialization::Data oldValue;
                EntryEventType eventType;
                std::string uuid;

            };
        }
    }
}

#endif //__PortableEntryEvent_H_
