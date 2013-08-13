//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_RemoveRequest
#define HAZELCAST_RemoveRequest

#include "CollectionKeyBasedRequest.h"
#include "CollectionProxyId.h"
#include "../serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class RemoveRequest : public CollectionKeyBasedRequest {
            public:
                RemoveRequest(const CollectionProxyId& id, const serialization::Data& key, const serialization::Data& value, int threadId)
                : CollectionKeyBasedRequest(id, key)
                , threadId(threadId) {

                };

                int getClassId() const {
                    return CollectionPortableHook::REMOVE;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeInt("t", threadId);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    value.writeData(out);
                    CollectionRequest::writePortable(writer);
                };


                void readPortable(serialization::PortableReader& reader) {
                    threadId = reader.readInt("t");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    value.readData(in);
                    CollectionRequest::readPortable(reader);
                };

            private:
                int threadId;
                serialization::Data value;
            };
        }
    }
}


#endif //HAZELCAST_RemoveRequest
