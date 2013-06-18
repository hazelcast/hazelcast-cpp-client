//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_LOCK_REQUEST
#define HAZELCAST_LOCK_REQUEST

#include "../serialization/Data.h"
#include "RequestIDs.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class LockRequest {
            public:
                LockRequest(const std::string& name, serialization::Data& key, int threadId, long ttl, long timeout)
                :name(name)
                , key(key)
                , threadId(threadId)
                , ttl(ttl)
                , timeout(timeout) {
                };

                LockRequest(const std::string& name, serialization::Data& key, int threadId)
                :name(name)
                , key(key)
                , threadId(threadId)
                , ttl(-1)
                , timeout(-1) {
                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return map::RequestIDs::F_ID;
                };

                int getClassId() const {
                    return map::RequestIDs::LOCK;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer["thread"] << threadId;
                    writer["ttl"] << ttl;
                    writer["timeout"] << timeout;
                    writer << key;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader["thread"] >> threadId;
                    reader["ttl"] >> ttl;
                    reader["timeout"] >> timeout;
                    reader >> key;
                };
            private:
                serialization::Data& key;
                std::string name;
                int threadId;
                long ttl;
                long timeout;
            };
        }
    }
}

#endif //HAZELCAST_LOCK_REQUEST
