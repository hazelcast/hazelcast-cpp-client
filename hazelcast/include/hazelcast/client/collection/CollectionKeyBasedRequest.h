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


                void writePortable(serialization::PortableWriter& writer) const {
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                    CollectionRequest::writePortable(writer);
                };


                void readPortable(serialization::PortableReader& reader) {
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    key.readData(in);
                    CollectionRequest::readPortable(reader);
                };
            private:
                serialization::Data key;
            };
        }
    }
}

#endif //HAZELCAST_CollectionKeyBasedRequest
