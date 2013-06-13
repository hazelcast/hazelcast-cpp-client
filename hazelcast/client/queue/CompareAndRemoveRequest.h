//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_COMPARE_AND_REMOVE_REQUEST
#define HAZELCAST_QUEUE_COMPARE_AND_REMOVE_REQUEST

#include "../serialization/Data.h"
#include "RequestIDs.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            class CompareAndRemoveRequest {
            public:

                CompareAndRemoveRequest(const std::string& name, std::vector<serialization::Data>& dataList, bool retain)
                :name(name)
                , dataList(dataList)
                , retain(retain) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return queue::RequestIDs::F_ID;
                }

                int getClassId() const {
                    return queue::RequestIDs::COMPARE_AND_REMOVE;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer["r"] << retain;
                    writer["s"] << dataList.size();
                    std::vector<serialization::Data>::iterator it;
                    for (it = dataList.begin(); it != dataList.end(); ++it) {
                        writer << (*it);
                    }
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader["r"] >> retain;
                    int size;
                    reader["s"] >> size;
                    dataList.resize(size);
                    for (int i = 0; i < size; i++) {
                        reader >> dataList[i];
                    }
                };
            private:
                std::vector<serialization::Data>& dataList;
                std::string name;
                bool retain;
            };
        }
    }
}

#endif //HAZELCAST_OFFER_REQUEST
