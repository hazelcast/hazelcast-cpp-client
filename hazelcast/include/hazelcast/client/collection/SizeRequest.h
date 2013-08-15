//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SizeRequest
#define HAZELCAST_SizeRequest

#include "CollectionProxyId.h"
#include "CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class SizeRequest : public CollectionRequest {
            public:
                SizeRequest(const CollectionProxyId& id)
                : CollectionRequest(id) {

                };

                int getClassId() const {
                    return CollectionPortableHook::SIZE;
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


#endif //HAZELCAST_SizeRequest
