//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_UNLOCK_REQUEST
#define HAZELCAST_UNLOCK_REQUEST

#include "../serialization/Data.h"
#include "RequestIDs.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class UnlockRequest {
            public:
                UnlockRequest(const std::string& name, serialization::Data& key, int threadId)
                :name(name)
                , key(key)
                , threadId(threadId)
                , force(false) {
                };

                UnlockRequest(const std::string& name, serialization::Data& key, int threadId, bool force)
                :name(name)
                , key(key)
                , threadId(threadId)
                , force(force) {
                };


                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return map::RequestIDs::F_ID;
                };

                int getClassId() const {
                    return map::RequestIDs::UNLOCK;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer["thread"] << threadId;
                    writer["force"] << force;
                    writer << key;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader["thread"] >> threadId;
                    reader["force"] >> force;
                    reader >> key;
                };
            private:
                serialization::Data& key;
                std::string name;
                int threadId;
                bool force;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
