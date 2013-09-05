//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PORTABLE_ITEM_EVENT
#define HAZELCAST_PORTABLE_ITEM_EVENT

#include "EventObject.h"
#include "EntryEvent.h"
#include "Data.h"
#include "Portable.h"
#include "ItemEvent.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace impl {
            class PortableItemEvent : public Portable {
            public:

                PortableItemEvent();

                PortableItemEvent(ItemEventType eventType, const serialization::Data& item, const std::string& uuid);

                const serialization::Data&  getItem() const;

                std::string getUuid() const;

                ItemEventType getEventType() const;

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data item;
                ItemEventType eventType;
                std::string uuid;

            };
        }
    }
}

#endif //HAZELCAST_PORTABLE_ITEM_EVENT
