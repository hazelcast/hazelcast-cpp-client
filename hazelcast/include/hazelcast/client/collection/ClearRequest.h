//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ClearRequest
#define HAZELCAST_ClearRequest

#include "CollectionRequest.h"
#include "CollectionProxyId.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class ClearRequest : public CollectionRequest {
            public:
                ClearRequest(const CollectionProxyId& id)
                : CollectionRequest(id) {

                };

                int getClassId() const {
                    return CollectionPortableHook::CLEAR;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    CollectionRequest::writePortable(writer);
                };


                void readPortable(serialization::PortableReader& reader) {
                    CollectionRequest::readPortable(reader);
                };

            };
        }
    }
}

#endif //HAZELCAST_ClearRequest
