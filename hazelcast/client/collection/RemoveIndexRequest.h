//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_RemoveIndexRequest
#define HAZELCAST_RemoveIndexRequest

#include "CollectionKeyBasedRequest.h"
#include "CollectionProxyId.h"
#include "../serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class RemoveIndexRequest : public CollectionKeyBasedRequest {
            public:
                RemoveIndexRequest(const CollectionProxyId& id, const serialization::Data& key, int threadId, int index)
                : CollectionKeyBasedRequest(id, key)
                , threadId(threadId)
                , index(index) {

                };

                int getClassId() const {
                    return CollectionPortableHook::REMOVE_INDEX;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["i"] << index;
                    writer["t"] << threadId;
                    CollectionRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["i"] >> index;
                    reader["t"] >> threadId;
                    CollectionRequest::readPortable(reader);
                };

            private:
                int threadId;
                int index;
            };
        }
    }
}


#endif //HAZELCAST_RemoveIndexRequest
