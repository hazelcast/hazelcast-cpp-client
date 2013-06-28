//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MultiMapIsLockedRequest
#define HAZELCAST_MultiMapIsLockedRequest


#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "CollectionPortableHook.h"
#include "CollectionProxyId.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace collection {
            class MultiMapIsLockedRequest {
            public:
                MultiMapIsLockedRequest(const CollectionProxyId& id, const serialization::Data& key, int threadId)
                :proxyId(id)
                , key(key)
                , threadId(threadId) {

                };

                int getFactoryId() const {
                    return CollectionPortableHook::F_ID;
                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getClassId() const {
                    return CollectionPortableHook::IS_LOCKED;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["tid"] << threadId;
                    writer << key;
                    writer << proxyId;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["tid"] >> threadId;
                    reader >> key;
                    reader >> proxyId;
                };

            private:
                CollectionProxyId proxyId;
                serialization::Data key;
                int threadId;
            };

        }
    }
}
#endif //HAZELCAST_MultiMapIsLockedRequest
