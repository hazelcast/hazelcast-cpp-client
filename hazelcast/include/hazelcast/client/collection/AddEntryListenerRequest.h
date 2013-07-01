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
            class AddEntryListenerRequest {
            public:
                AddEntryListenerRequest(const CollectionProxyId& id, const serialization::Data& key, bool includeValue)
                :proxyId(id)
                , key(key)
                , includeValue(includeValue) {

                };

                int getFactoryId() const {
                    return CollectionPortableHook::F_ID;
                };

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getClassId() const {
                    return CollectionPortableHook::ADD_ENTRY_LISTENER;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["i"] << includeValue;
                    writer << proxyId;
                    writer << true;
                    writer << key;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["i"] >> includeValue;
                    reader >> proxyId;
                    bool isNotNull;
                    reader >> isNotNull;
                    if (isNotNull)
                        reader >> key;
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
