//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_TRY_REMOVE_REQUEST
#define HAZELCAST_MAP_TRY_REMOVE_REQUEST

#include "../serialization/Data.h"
#include "RequestIDs.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class TryRemoveRequest {
            public:
                TryRemoveRequest(const std::string& name, serialization::Data& key, int threadId, long timeout)
                :name(name)
                , key(key)
                , threadId(threadId)
                , timeout(timeout) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return map::RequestIDs::F_ID;
                };

                int getClassId() const {
                    return map::RequestIDs::TRY_REMOVE;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer["t"] << threadId;
                    writer["timeout"] << timeout;
                    writer << key;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader["t"] >> threadId;
                    reader["timeout"] >> timeout;
                    reader >> key;
                };
            private:
                serialization::Data& key;
                std::string name;
                int threadId;
                int timeout;
            };
        }
    }
}

#endif //HAZELCAST_MAP_REMOVE_REQUEST
