//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_DestroyRequest
#define HAZELCAST_DestroyRequest

#include "CollectionProxyId.h"
#include "CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class DestroyRequest : public CollectionRequest {
            public:
                DestroyRequest(const CollectionProxyId& id)
                : CollectionRequest(id) {

                };

                int getClassId() const {
                    return CollectionPortableHook::DESTROY;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    CollectionRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    CollectionRequest::readPortable(reader);
                };
            };
        }
    }
}

#endif //HAZELCAST_DestroyRequest
