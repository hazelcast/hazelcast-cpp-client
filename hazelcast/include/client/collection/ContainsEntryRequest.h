//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ContainsEntryRequest
#define HAZELCAST_ContainsEntryRequest

#include "CollectionProxyId.h"
#include "CollectionRequest.h"
#include "../serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class ContainsEntryRequest : public CollectionRequest {
            public:
                ContainsEntryRequest(const CollectionProxyId& id, const serialization::Data& key, const serialization::Data& value)
                : CollectionRequest(id)
                , hasKey(true)
                , key(key)
                , value(value) {

                };

                ContainsEntryRequest(const CollectionProxyId& id, const serialization::Data& value)
                : CollectionRequest(id)
                , hasKey(false)
                , value(value) {

                };

                int getClassId() const {
                    return CollectionPortableHook::CONTAINS_ENTRY;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer << hasKey;
                    if (hasKey) {
                        writer << key;
                    }
                    writer << true;
                    writer << value;
                    CollectionRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader >> hasKey;
                    if (hasKey)
                        reader >> key;
                    bool isNotNull;
                    reader >> isNotNull;
                    if (isNotNull)
                        reader >> value;
                    CollectionRequest::readPortable(reader);
                };

            private:
                bool hasKey;
                serialization::Data key;
                serialization::Data value;
            };
        }
    }
}

#endif //HAZELCAST_ContainsEntryRequest
