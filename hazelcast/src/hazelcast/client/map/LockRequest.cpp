//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/LockRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {
            LockRequest::LockRequest(const std::string &name, serialization::pimpl::Data &key, long threadId, long ttl, long timeout)
            :name(name)
            , key(key)
            , threadId(threadId)
            , ttl(ttl)
            , timeout(timeout) {
            };

            LockRequest::LockRequest(const std::string &name, serialization::pimpl::Data &key, long threadId)
            :name(name)
            , key(key)
            , threadId(threadId)
            , ttl(-1)
            , timeout(-1) {
            };

            int LockRequest::getFactoryId() const {
                return PortableHook::F_ID;
            };

            int LockRequest::getClassId() const {
                return PortableHook::LOCK;
            };

            void LockRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("tid", threadId);
                writer.writeLong("ttl", ttl);
                writer.writeLong("timeout", timeout);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                key.writeData(out);
            };
        }
    }
}

