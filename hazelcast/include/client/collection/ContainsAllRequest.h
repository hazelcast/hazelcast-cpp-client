//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ContainsAllRequest
#define HAZELCAST_ContainsAllRequest

#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "CollectionPortableHook.h"
#include "CollectionKeyBasedRequest.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace collection {
            class ContainsAllRequest : public CollectionKeyBasedRequest {
            public:
                ContainsAllRequest(const CollectionProxyId& id, const serialization::Data& key, const std::vector<serialization::Data>& dataList)
                :CollectionKeyBasedRequest(id, key)
                , dataList(dataList) {

                };

                int getClassId() const {
                    return CollectionPortableHook::CONTAINS_ALL;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["s"] << dataList.size();
                    for (int i = 0; i < dataList.size(); ++i) {
                        writer << dataList[i];
                    }
                    CollectionKeyBasedRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
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
            };

        }
    }
}


#endif //HAZELCAST_ContainsAllRequest
