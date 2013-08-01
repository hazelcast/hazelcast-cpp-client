//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_AddItemListenerRequest
#define HAZELCAST_AddItemListenerRequest

#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "CollectionPortableHook.h"
#include "CollectionKeyBasedRequest.h"
#include "CollectionRequest.h"
#include "CollectionProxyId.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace collection {
            class AddItemListenerRequest : public Portable{
            public:
                AddItemListenerRequest(const CollectionProxyId& id, const serialization::Data& key, bool includeValue)
                :proxyId(id)
                , key(key)
                , includeValue(includeValue) {

                };

                int getFactoryId() const {
                    return CollectionPortableHook::F_ID;
                };

                int getClassId() const {
                    return CollectionPortableHook::ADD_ITEM_LISTENER;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeInt("i", includeValue);
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    proxyId.writeData(*out);
                    out->writeBoolean(true);
                    key.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    includeValue = reader.readInt("i");
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    proxyId.readData(*in);
                    bool isNotNull = in->readBoolean();
                    if (isNotNull)
                        key.readData(*in);
                };

            private:
                CollectionProxyId proxyId;
                serialization::Data key;
                bool includeValue;
            };

        }
    }
}


#endif //HAZELCAST_AddItemListenerRequest
