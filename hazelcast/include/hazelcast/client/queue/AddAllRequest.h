//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_ADD_ALL_REQUEST
#define HAZELCAST_QUEUE_ADD_ALL_REQUEST

#include "../serialization/Data.h"
#include "QueuePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            class AddAllRequest {
            public:

                AddAllRequest(const std::string& name, std::vector<serialization::Data>& dataList)
                :name(name)
                , dataList(dataList) {

                };

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return queue::QueuePortableHook::F_ID;
                }

                int getClassId() const {
                    return queue::QueuePortableHook::ADD_ALL;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer["s"] << dataList.size();
                    std::vector<serialization::Data>::iterator it;
                    for (it = dataList.begin(); it != dataList.end(); ++it) {
                        writer << (*it);
                    }
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
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
            };
        }
    }
}

#endif //HAZELCAST_QUEUE_ADD_ALL_REQUEST
