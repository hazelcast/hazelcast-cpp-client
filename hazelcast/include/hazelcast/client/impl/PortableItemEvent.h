//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PORTABLE_ITEM_EVENT
#define HAZELCAST_PORTABLE_ITEM_EVENT

#include "EventObject.h"
#include "../connection/Member.h"
#include "EntryEvent.h"
#include "Data.h"
#include "Portable.h"
#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class PortableItemEvent : public Portable {
            public:
                enum ItemEventType {
                    ADDED, REMOVED
                };

                PortableItemEvent(ItemEventType eventType, const serialization::Data& item, const std::string& uuid);

                const serialization::Data&  getItem() const;

                std::string getUuid() const;

                ItemEventType getEventType() const;

                int getFactoryId() const;

                int getClassId() const;

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeInt("e", eventType);
                    writer.writeUTF("u", uuid);
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    out->writeBoolean(true);
                    item.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    int type = reader.readInt("e");
//                    eventType = type;
                    uuid = reader.readUTF("u");
                    bool isNotNull;
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
                    isNotNull = in->readBoolean();
                    if (isNotNull)
                        item.readData(*in);
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
