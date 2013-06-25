//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetAllRequest
#define HAZELCAST_GetAllRequest

#include "CollectionKeyBasedRequest.h"
#include "CollectionProxyId.h"
#include "../serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class GetAllRequest : public CollectionKeyBasedRequest {
            public:
                GetAllRequest(const CollectionProxyId& id, const serialization::Data& key)
                : CollectionKeyBasedRequest(id, key) {

                };

                int getClassId() const {
                    return CollectionPortableHook::GET_ALL;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    CollectionRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    CollectionRequest::readPortable(reader);
                };

            private:
            };
        }
    }
}

#endif //HAZELCAST_GetAllRequest
