//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MultiMapLockRequest
#define HAZELCAST_MultiMapLockRequest


#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "CollectionPortableHook.h"
#include "CollectionProxyId.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace collection {
            class MultiMapLockRequest {
            public:
                MultiMapLockRequest(const CollectionProxyId& id, const serialization::Data& key, int threadId)
                :proxyId(id)
                , key(key)
                , threadId(threadId)
                , ttl(-1)
                , timeout(-1) {

                };

                MultiMapLockRequest(const CollectionProxyId& id, const serialization::Data& key, int threadId, long ttl, long timeout)
                :proxyId(id)
                , key(key)
                , threadId(threadId)
                , ttl(ttl)
                , timeout(timeout) {

                };

                int getFactoryId() const {
                    return CollectionPortableHook::F_ID;
                };

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getClassId() const {
                    return CollectionPortableHook::LOCK;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["tid"] << threadId;
                    writer["ttl"] << ttl;
                    writer["timeout"] << timeout;
                    writer << key;
                    writer << proxyId;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["tid"] >> threadId;
                    reader["ttl"] >> ttl;
                    reader["timeout"] >> timeout;
                    reader >> key;
                    reader >> proxyId;
                };

            private:
                CollectionProxyId proxyId;
                serialization::Data key;
                int threadId;
                long ttl;
                long timeout;
            };

        }
    }
}

#endif //HAZELCAST_MultiMapLockRequest
