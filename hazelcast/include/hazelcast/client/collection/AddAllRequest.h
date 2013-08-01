//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_AddAllRequest
#define HAZELCAST_AddAllRequest

#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "CollectionPortableHook.h"
#include "CollectionKeyBasedRequest.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace collection {
            class AddAllRequest : public CollectionKeyBasedRequest {
            public:
                AddAllRequest(const CollectionProxyId& id, const serialization::Data& key, int threadId, const std::vector<serialization::Data>& dataList)
                :CollectionKeyBasedRequest(id, key)
                , threadId(threadId)
                , dataList(dataList)
                , index(-1) {

                };

                AddAllRequest(const CollectionProxyId& id, const serialization::Data& key, int threadId, const std::vector<serialization::Data>& dataList, int index)
                :CollectionKeyBasedRequest(id, key)
                , threadId(threadId)
                , dataList(dataList)
                , index(index) {
                };

                int getClassId() const {
                    return CollectionPortableHook::ADD_ALL;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeInt("i", index);
                    writer.writeInt("t", threadId);
                    writer.writeInt("s", dataList.size());
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    for (int i = 0; i < dataList.size(); ++i) {
                        dataList[i].writeData(*out);
                    }
                    CollectionKeyBasedRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    index = reader.readInt("i");
                    threadId = reader.readInt("t");
                    int size = reader.readInt("s");
                    dataList.resize(size);
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    for (int i = 0; i < size; ++i) {
                        dataList[i].readData(*in);
                    }
                    CollectionKeyBasedRequest::readPortable(reader);
                };

            private:
                std::vector<serialization::Data> dataList;
                int threadId;
                int index;
            };

        }
    }
}


#endif //HAZELCAST_AddAllRequest
