//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetRequest
#define HAZELCAST_GetRequest


#include "CollectionKeyBasedRequest.h"
#include "CollectionProxyId.h"
#include "../serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class GetRequest : public CollectionKeyBasedRequest {
            public:
                GetRequest(const CollectionProxyId& id, const serialization::Data& key, int index)
                : CollectionKeyBasedRequest(id, key), index(index) {

                };

                int getClassId() const {
                    return CollectionPortableHook::GET;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeInt("i", index);
                    CollectionRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    index = reader.readInt("i");
                    CollectionRequest::readPortable(reader);
                };

            private:
                int index;
            };
        }
    }
}


#endif //HAZELCAST_GetRequest
