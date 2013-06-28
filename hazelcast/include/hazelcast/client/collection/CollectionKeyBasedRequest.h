//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CollectionKeyBasedRequest
#define HAZELCAST_CollectionKeyBasedRequest

#include "../serialization/Data.h"
#include "CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class CollectionKeyBasedRequest : public CollectionRequest {
            public:
                CollectionKeyBasedRequest(const CollectionProxyId& id, const serialization::Data& key)
                :CollectionRequest(id)
                , key(key) {

                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer << key;
                    CollectionRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader >> key;
                    CollectionRequest::readPortable(reader);
                };
            private:
                serialization::Data key;
            };
        }
    }
}

#endif //HAZELCAST_CollectionKeyBasedRequest
