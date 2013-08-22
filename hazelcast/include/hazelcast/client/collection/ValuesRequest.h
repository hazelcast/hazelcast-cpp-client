//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ValuesRequest
#define HAZELCAST_ValuesRequest

#include "CollectionProxyId.h"
#include "CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class ValuesRequest : public CollectionRequest ,public RetryableRequest{
            public:
                ValuesRequest(const CollectionProxyId& id)
                : CollectionRequest(id) {

                };

                int getClassId() const {
                    return CollectionPortableHook::VALUES;
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


#endif //HAZELCAST_ValuesRequest
