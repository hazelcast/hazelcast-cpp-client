//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/lock/LockRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/lock/LockPortableHook.h"


namespace hazelcast {
    namespace client {
        namespace lock {

            LockRequest::LockRequest(serialization::Data &key, long threadId, long ttl, long timeout)
            :key(key)
            , threadId(threadId)
            , ttl(ttl)
            , timeout(timeout) {
            };

            int LockRequest::getClassId() const {
                return LockPortableHook::LOCK;
            };

            int LockRequest::getFactoryId() const {
                return LockPortableHook::FACTORY_ID;
            };


            void LockRequest::write(serialization::PortableWriter &writer) const {
                writer.writeLong("tid", threadId);
                writer.writeLong("ttl", ttl);
                writer.writeLong("timeout", timeout);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                key.writeData(out);
            };

        }
    }
}
