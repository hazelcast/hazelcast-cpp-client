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
            class MultiMapLockRequest : public Portable{
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

                int getClassId() const {
                    return CollectionPortableHook::LOCK;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeInt("tid", threadId);
                    writer.writeLong("ttl", ttl);
                    writer.writeLong("timeout", timeout);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                    proxyId.writeData(out);
                };


                void readPortable(serialization::PortableReader& reader) {
                    threadId = reader.readInt("tid");
                    ttl = reader.readLong("ttl");
                    timeout = reader.readLong("timeout");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    key.readData(in);
                    proxyId.readData(in);
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
