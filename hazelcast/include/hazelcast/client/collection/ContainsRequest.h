//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ContainsRequest
#define HAZELCAST_ContainsRequest


#include "CollectionProxyId.h"
#include "CollectionRequest.h"
#include "../serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class ContainsRequest : public CollectionRequest {
            public:
                ContainsRequest(const CollectionProxyId& id, const serialization::Data& value)
                : CollectionRequest(id)
                , value(value) {

                };

                int getClassId() const {
                    return CollectionPortableHook::CONTAINS;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    value.writeData(*out);
                    CollectionRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
                    value.readData(*in);
                    CollectionRequest::readPortable(reader);
                };

            private:
                serialization::Data value;
            };
        }
    }
}

#endif //HAZELCAST_ContainsRequest
