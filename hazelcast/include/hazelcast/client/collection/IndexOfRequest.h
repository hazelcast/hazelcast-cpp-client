//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_IndexOfRequest
#define HAZELCAST_IndexOfRequest


#include "CollectionKeyBasedRequest.h"
#include "CollectionProxyId.h"
#include "../serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class IndexOfRequest : public CollectionKeyBasedRequest {
            public:
                IndexOfRequest(const CollectionProxyId& id, const serialization::Data& key, serialization::Data& value, bool last)
                : CollectionKeyBasedRequest(id, key)
                , value(value)
                , last(last) {

                };

                int getClassId() const {
                    return CollectionPortableHook::INDEX_OF;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeBoolean("l", last);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    value.writeData(out);
                    CollectionRequest::writePortable(writer);
                };


                void readPortable(serialization::PortableReader& reader) {
                    last = reader.readBoolean("l");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    value.readData(in);
                    CollectionRequest::readPortable(reader);
                };

            private:
                serialization::Data& value;
                bool last;
            };
        }
    }
}

#endif //HAZELCAST_IndexOfRequest
