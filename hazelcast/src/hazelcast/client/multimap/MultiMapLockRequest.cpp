//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/MultiMapLockRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace multimap {
            MultiMapLockRequest::MultiMapLockRequest(const std::string &name, const serialization::Data &key, int threadId)
            :KeyBasedRequest(name, key)
            , threadId(threadId)
            , ttl(-1)
            , timeout(-1) {

            };

            MultiMapLockRequest::MultiMapLockRequest(const std::string &name, const serialization::Data &key, int threadId, long ttl, long timeout)
            :KeyBasedRequest(name, key)
            , threadId(threadId)
            , ttl(ttl)
            , timeout(timeout) {

            };

            int MultiMapLockRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            };

            int MultiMapLockRequest::getClassId() const {
                return MultiMapPortableHook::LOCK;
            };

            void MultiMapLockRequest::write(serialization::PortableWriter &writer) const {
                writer.writeInt("tid", threadId);
                writer.writeLong("ttl", ttl);
                writer.writeLong("timeout", timeout);
                KeyBasedRequest::write(writer);
            };

        }
    }
}