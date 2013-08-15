//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_RemoveAllRequest
#define HAZELCAST_RemoveAllRequest

#include "CollectionKeyBasedRequest.h"
#include "CollectionProxyId.h"
#include "../serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class RemoveAllRequest : public CollectionKeyBasedRequest {
            public:
                RemoveAllRequest(const CollectionProxyId& id, const serialization::Data& key, int threadId)
                : CollectionKeyBasedRequest(id, key)
                , threadId(threadId) {

                };

                int getClassId() const {
                    return CollectionPortableHook::REMOVE_ALL;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeInt("t", threadId);
                    CollectionRequest::writePortable(writer);
                };


                void readPortable(serialization::PortableReader& reader) {
                    threadId = reader.readInt("t");
                    CollectionRequest::readPortable(reader);
                };

            private:
                int threadId;
            };
        }
    }
}


#endif //HAZELCAST_RemoveAllRequest
