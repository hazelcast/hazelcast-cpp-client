//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_OFFER_REQUEST
#define HAZELCAST_QUEUE_OFFER_REQUEST

#include "../serialization/Data.h"
#include "RequestIDs.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            class AddAllRequest {
            public:

                AddAllRequest(const std::string& name, std::vector<serialization::Data>& data)
                :name(name)
                , dataList(dataList) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return queue::RequestIDs::F_ID;
                }

                int getClassId() const {
                    return queue::RequestIDs::ADD_ALL;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
//                    writer << dataList; TODO add vector<Data> to serialization
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
//                    reader >> dataList; TODO add vector<Data> to serialization
                };
            private:
                std::vector<serialization::Data>& dataList;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_OFFER_REQUEST
