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
                    writer["i"] << index;
                    writer["t"] << threadId;
                    writer["s"] << dataList.size();
                    for (int i = 0; i < dataList.size(); ++i) {
                        writer << dataList[i];
                    }
                    CollectionKeyBasedRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["i"] >> index;
                    reader["t"] >> threadId;
                    int size;
                    reader["s"] >> size;
                    dataList.resize(size);
                    for (int i = 0; i < size; ++i) {
                        reader >> dataList[i];
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
