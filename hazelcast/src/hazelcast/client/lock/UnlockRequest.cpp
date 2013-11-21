//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "hazelcast/client/lock/UnlockRequest.h"
#include "LockPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace lock {
            UnlockRequest::UnlockRequest(serialization::Data& key, int threadId)
            :key(key)
            , threadId(threadId)
            , force(false) {
            };

            UnlockRequest::UnlockRequest(serialization::Data& key, int threadId, bool force)
            :key(key)
            , threadId(threadId)
            , force(force) {
            };

            int UnlockRequest::getClassId() const {
                return LockPortableHook::UNLOCK;
            };

            int UnlockRequest::getFactoryId() const {
                return LockPortableHook::FACTORY_ID;
            };

            void UnlockRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeInt("tid", threadId);
                writer.writeBoolean("force", force);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };


            void UnlockRequest::readPortable(serialization::PortableReader& reader) {
                threadId = reader.readInt("tid");
                force = reader.readBoolean("force");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
            };
        }
    }
}
