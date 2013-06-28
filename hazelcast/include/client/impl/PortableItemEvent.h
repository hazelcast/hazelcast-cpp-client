//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PORTABLE_ITEM_EVENT
#define HAZELCAST_PORTABLE_ITEM_EVENT

#include "EventObject.h"
#include "../connection/Member.h"
#include "EntryEvent.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class PortableItemEvent {
            public:
                enum ItemEventType {
                    ADDED, REMOVED
                };

                PortableItemEvent(ItemEventType eventType, const serialization::Data& item, const std::string& uuid);

                const serialization::Data&  getItem() const;

                std::string getUuid() const;

                ItemEventType getEventType() const;

                int getTypeSerializerId() const;

                int getFactoryId() const;

                int getClassId() const;

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["e"] << (int) eventType;
                    writer["u"] << uuid;
                    writer << true;
                    writer << item;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    int type;
                    reader["e"] >> type;
//                    eventType = type;
                    reader["u"] >> uuid;
                    bool isNotNull;
                    reader >> isNotNull;
                    if (isNotNull)
                        reader >> item;
                };

            private:
                serialization::Data item;
                ItemEventType eventType;
                std::string uuid;

            };
        }
    }
}

#endif //HAZELCAST_PORTABLE_ITEM_EVENT
