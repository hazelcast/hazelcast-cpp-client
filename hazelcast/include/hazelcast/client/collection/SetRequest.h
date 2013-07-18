//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SetRequest
#define HAZELCAST_SetRequest

#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "CollectionPortableHook.h"
#include "CollectionKeyBasedRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class SetRequest : public CollectionKeyBasedRequest {
            public:
                SetRequest(const CollectionProxyId& id, const serialization::Data& key, const serialization::Data& value, int index, int threadId)
                :CollectionKeyBasedRequest(id, key)
                , value(value)
                , threadId(threadId)
                , index(index) {

                };

                int getClassId() const {
                    return CollectionPortableHook::SET;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeInt("i", index);
                    writer.writeInt("t", threadId);
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    value.writeData(*out);
                    CollectionKeyBasedRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    index = reader.readInt("i");
                    threadId = reader.readInt("t");
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
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


#endif //HAZELCAST_SetRequest
