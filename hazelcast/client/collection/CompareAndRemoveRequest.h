//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CompareAndRemoveRequest
#define HAZELCAST_CompareAndRemoveRequest

#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "CollectionPortableHook.h"
#include "CollectionKeyBasedRequest.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace collection {
            class CompareAndRemoveRequest : public CollectionKeyBasedRequest {
            public:
                CompareAndRemoveRequest(const CollectionProxyId& id, const serialization::Data& key, int threadId, bool retain, const std::vector<serialization::Data>& dataList)
                :CollectionKeyBasedRequest(id, key)
                , threadId(threadId)
                , dataList(dataList)
                , retain(retain) {

                };

                int getClassId() const {
                    return CollectionPortableHook::COMPARE_AND_REMOVE;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["r"] << retain;
                    writer["t"] << threadId;
                    writer["s"] << dataList.size();
                    for (int i = 0; i < dataList.size(); ++i) {
                        writer << dataList[i];
                    }
                    CollectionKeyBasedRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["r"] >> retain;
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
                bool retain;
                int threadId;
            };

        }
    }
}


#endif //HAZELCAST_CompareAndRemoveRequest
