//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PutRequest
#define HAZELCAST_PutRequest

#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "CollectionPortableHook.h"
#include "CollectionKeyBasedRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class PutRequest : public CollectionKeyBasedRequest {
            public:
                PutRequest(const CollectionProxyId& id, const serialization::Data& key, const serialization::Data& value, int index, int threadId)
                :CollectionKeyBasedRequest(id, key)
                , value(value)
                , threadId(threadId)
                , index(index) {

                };

                int getClassId() const {
                    return CollectionPortableHook::PUT;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["i"] << index;
                    writer["t"] << threadId;
                    writer << value;
                    CollectionKeyBasedRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["i"] >> index;
                    reader["t"] >> threadId;
                    reader >> value;
                    CollectionKeyBasedRequest::readPortable(reader);
                };

            private:
                serialization::Data value;
                int threadId;
                int index;
            };

        }
    }
}


#endif //HAZELCAST_PutRequest
