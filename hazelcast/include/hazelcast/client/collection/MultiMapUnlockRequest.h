//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MultiMapUnlockRequest
#define HAZELCAST_MultiMapUnlockRequest


#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "CollectionPortableHook.h"
#include "CollectionProxyId.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace collection {
            class MultiMapUnlockRequest : public Portable {
            public:
                MultiMapUnlockRequest(const CollectionProxyId& id, const serialization::Data& key, int threadId)
                :proxyId(id)
                , key(key)
                , threadId(threadId) {

                };

                int getFactoryId() const {
                    return CollectionPortableHook::F_ID;
                };

                int getClassId() const {
                    return CollectionPortableHook::UNLOCK;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeInt("tid", threadId);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                    proxyId.writeData(out);
                };


                void readPortable(serialization::PortableReader& reader) {
                    threadId = reader.readInt("tid");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    key.readData(in);
                    proxyId.readData(in);
                };

            private:
                CollectionProxyId proxyId;
                serialization::Data key;
                int threadId;
            };

        }
    }
}


#endif //HAZELCAST_MultiMapUnlockRequest
