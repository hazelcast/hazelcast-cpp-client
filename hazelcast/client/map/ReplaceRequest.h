//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_REPLACE_REQUEST
#define HAZELCAST_MAP_REPLACE_REQUEST

#include "../serialization/Data.h"
#include "RequestIDs.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class ReplaceRequest {
            public:
                ReplaceRequest(const std::string& name, serialization::Data& key, serialization::Data& value, int threadId)
                :name(name)
                , key(key)
                , value(value)
                , threadId(threadId) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return map::RequestIDs::F_ID;
                }

                int getClassId() const {
                    return map::RequestIDs::REPLACE;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer["t"] << threadId;
                    writer << key;
                    writer << value;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader["t"] >> threadId;
                    reader >> key;
                    reader >> value;
                };
            private:
                serialization::Data& key;
                serialization::Data& value;
                std::string name;
                int threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_REMOVE_IF_SAME_REQUEST
