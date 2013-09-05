//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/lock/LockRequest.h"
#include "PortableWriter.h"
#include "PortableReader.h"
#include "Data.h"
#include "LockPortableHook.h"


namespace hazelcast {
    namespace client {
        namespace lock {

            LockRequest::LockRequest(serialization::Data& key, int threadId)
            :key(key)
            , threadId(threadId)
            , ttl(-1)
            , timeout(-1) {
            };

            LockRequest::LockRequest(serialization::Data& key, int threadId, long ttl, long timeout)
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


            void LockRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeInt("tid", threadId);
                writer.writeLong("ttl", ttl);
                writer.writeLong("timeout", timeout);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };


            void LockRequest::readPortable(serialization::PortableReader& reader) {
                threadId = reader.readInt("tid");
                ttl = reader.readLong("ttl");
                timeout = reader.readLong("timeout");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
            };
        }
    }
}
