//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_LockRequest
#define HAZELCAST_LockRequest


#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "LockPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace lock {
            class LockRequest {
            public:
            public:
                LockRequest(const serialization::Data& key, int threadId)
                :key(key)
                , threadId(threadId)
                , ttl(-1)
                , timeout(-1) {
                };

                LockRequest(const serialization::Data& key, int threadId, long ttl, long timeout)
                :key(key)
                , threadId(threadId)
                , ttl(ttl)
                , timeout(timeout) {
                };

                int getClassId() const {
                    return LockPortableHook::LOCK;
                };

                int getFactoryId() const {
                    return LockPortableHook::FACTORY_ID;
                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["tid"] << threadId;
                    writer["ttl"] << ttl;
                    writer["timeout"] << timeout;
                    writer << key;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["tid"] >> threadId;
                    reader["ttl"] >> ttl;
                    reader["timeout"] >> timeout;
                    reader >> key;
                };
            private:

                serialization::Data key;
                int threadId;
                long ttl;
                long timeout;
            };
        }
    }
}


#endif //HAZELCAST_LockRequest
