//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/UnlockRequest.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/map/PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            UnlockRequest::UnlockRequest(const std::string& name, serialization::Data& key, int threadId)
            :name(name)
            , key(key)
            , threadId(threadId)
            , force(false) {
            };

            UnlockRequest::UnlockRequest(const std::string& name, serialization::Data& key, int threadId, bool force)
            :name(name)
            , key(key)
            , threadId(threadId)
            , force(force) {
            };

            int UnlockRequest::getFactoryId() const {
                return PortableHook::F_ID;
            };

            int UnlockRequest::getClassId() const {
                return PortableHook::UNLOCK;
            };

            void UnlockRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeInt("tid", threadId);
                writer.writeBoolean("force", force);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };
        }
    }
}
