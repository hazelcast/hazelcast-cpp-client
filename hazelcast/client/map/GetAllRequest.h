//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_ALL_REQUEST
#define HAZELCAST_MAP_GET_ALL_REQUEST

#include "../serialization/Data.h"
#include "RequestIDs.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class GetAllRequest {
            public:
                GetAllRequest(const std::string& name, std::vector<serialization::Data>& keys)
                :name(name)
                , keys(keys) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return map::RequestIDs::F_ID;
                }

                int getClassId() const {
                    return map::RequestIDs::GET_ALL;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer["size"] << keys.size();
                    std::vector<serialization::Data>::iterator it;
                    for (it = keys.begin(); it != keys.end(); ++it) {
                        writer << (*it);
                    }
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    int size;
                    reader["size"] >> size;
                    keys.resize(size);
                    for (int i = 0; i < size; i++) {
                        reader >> keys[i];
                    }

                };
            private:
                std::vector<serialization::Data> keys;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_ALL_REQUEST
