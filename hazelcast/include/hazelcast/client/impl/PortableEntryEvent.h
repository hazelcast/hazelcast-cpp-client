//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_PORTABLE_ENTRY_EVENT
#define HAZELCAST_PORTABLE_ENTRY_EVENT

#include "EventObject.h"
#include "../connection/Member.h"
#include "EntryEvent.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class PortableEntryEvent : public EventObject {
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


                int getSerializerId() const;

                int getFactoryId() const;

                int getClassId() const;

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer << (int) eventType;
                    writer << uuid;
                    writer << key;
                    writer << true;
                    writer << value;
                    writer << true;
                    writer << oldValue;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    int type;
                    reader >> type;
//                    eventType = type;
                    reader >> key;
                    bool isNotNull;
                    reader >> isNotNull;
                    if (isNotNull)
                        reader >> value;
                    reader >> isNotNull;
                    if (isNotNull)
                        reader >> oldValue;
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
