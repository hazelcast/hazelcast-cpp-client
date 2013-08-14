//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AddEntryListenerRequest
#define HAZELCAST_AddEntryListenerRequest


#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "CollectionPortableHook.h"
#include "CollectionProxyId.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace collection {
            class AddEntryListenerRequest : public Portable {
            public:
                AddEntryListenerRequest(const CollectionProxyId& id, const serialization::Data& key, bool includeValue)
                :proxyId(id)
                , key(key)
                , includeValue(includeValue) {

                };

                int getFactoryId() const {
                    return CollectionPortableHook::F_ID;
                };

                int getClassId() const {
                    return CollectionPortableHook::ADD_ENTRY_LISTENER;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeInt("i", includeValue);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    proxyId.writeData(out);
                    out.writeBoolean(true);
                    key.writeData(out);
                };


                void readPortable(serialization::PortableReader& reader) {
                    includeValue = reader.readInt("i");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    proxyId.readData(in);
                    bool isNotNull = in.readBoolean();
                    if (isNotNull)
                        key.readData(in);
                };

            private:
                CollectionProxyId proxyId;
                serialization::Data key;
                bool includeValue;
            };

        }
    }
}


#endif //HAZELCAST_AddEntryListenerRequest
