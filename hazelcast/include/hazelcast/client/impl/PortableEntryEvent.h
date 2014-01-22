//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_PORTABLE_ENTRY_EVENT
#define HAZELCAST_PORTABLE_ENTRY_EVENT

#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API PortableEntryEvent : public Portable {
            public:
                PortableEntryEvent();

                PortableEntryEvent(const std::string &name, const Member &member, EntryEventType eventType, const serialization::Data &key, const serialization::Data &value);

                PortableEntryEvent(const std::string &name, const Member &member, EntryEventType eventType, const serialization::Data &key, const serialization::Data &value, const serialization::Data &oldValue);

                const serialization::Data &getKey() const;

                const serialization::Data &getOldValue() const;

                const serialization::Data &getValue() const;

                std::string getUuid() const;

                EntryEventType getEventType() const;

                std::string getName() const;

                int getFactoryId() const;

                int getClassId() const;


                void writePortable(serialization::PortableWriter &writer) const;

                void readPortable(serialization::PortableReader &reader);

            private:
                serialization::Data key;
                serialization::Data value;
                serialization::Data oldValue;
                EntryEventType eventType;
                std::string uuid;
                std::string name;

            };
        }
    }
}

#endif //__PortableEntryEvent_H_
