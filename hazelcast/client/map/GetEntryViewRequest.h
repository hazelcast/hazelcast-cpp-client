//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_ENTRY_VIEW_REQUEST
#define HAZELCAST_MAP_GET_ENTRY_VIEW_REQUEST

#include "../serialization/Data.h"
#include "RequestIDs.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class GetEntryViewRequest {
            public:
                GetEntryViewRequest(const std::string& name, serialization::Data& key)
                :name(name)
                , key(key) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return map::RequestIDs::F_ID;
                }

                int getClassId() const {
                    return map::RequestIDs::GET_ENTRY_VIEW;
                }


                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer << key;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader >> key;
                };
            private:
                serialization::Data& key;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_ENTRY_VIEW_REQUEST
