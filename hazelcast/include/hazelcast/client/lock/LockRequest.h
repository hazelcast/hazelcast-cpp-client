//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_LockRequest
#define HAZELCAST_LockRequest


#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "LockPortableHook.h"
#include "Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace lock {
            class LockRequest : public Portable {
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

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeInt("tid", threadId);
                    writer.writeLong("ttl", ttl);
                    writer.writeLong("timeout", timeout);
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    key.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    threadId = reader.readInt("tid");
                    ttl = reader.readLong("ttl");
                    timeout = reader.readLong("timeout");
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
                    key.readData(*in);
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
