//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/UnlockRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/map/PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {

            UnlockRequest::UnlockRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId, bool force)
            :name(name)
            , key(key)
            , threadId(threadId)
            , force(force) {
            }

            int UnlockRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int UnlockRequest::getClassId() const {
                return PortableHook::UNLOCK;
            }

            void UnlockRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("tid", threadId);
                writer.writeBoolean("force", force);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeData(&key);
            }
        }
    }
}

