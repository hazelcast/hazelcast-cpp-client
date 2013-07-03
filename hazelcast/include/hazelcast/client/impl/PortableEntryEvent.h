//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_PORTABLE_ENTRY_EVENT
#define HAZELCAST_PORTABLE_ENTRY_EVENT

#include "EventObject.h"
#include "../connection/Member.h"
#include "EntryEvent.h"
#include "Data.h"
#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"
#include "Portable.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class PortableEntryEvent : public EventObject, public Portable {
            public:
                enum EntryEventType {
                    ADDED, REMOVED, UPDATED, EVICTED
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

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    out->writeInt(eventType);
                    out->writeUTF(uuid);
                    key.writeData(*out);
                    out->writeBoolean(true);
                    value.writeData(*out);
                    out->writeBoolean(true);
                    oldValue.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
                    int type = in->readInt();
//                    eventType = type;
                    key.readData(*in);
                    bool isNotNull = in->readBoolean();
                    if (isNotNull)
                        value.readData(*in);
                    isNotNull = in->readBoolean();
                    if (isNotNull)
                        oldValue.readData(*in);
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
