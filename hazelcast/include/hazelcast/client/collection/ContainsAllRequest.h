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
                    writer.writeInt("s", dataList.size());
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    for (int i = 0; i < dataList.size(); ++i) {
                        dataList[i].writeData(*out);
                    }
                    CollectionKeyBasedRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    int size = reader.readInt("s");
                    dataList.resize(size);
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
                    for (int i = 0; i < size; ++i) {
                        dataList[i].readData(*in);
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
