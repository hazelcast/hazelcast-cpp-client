//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "hazelcast/client/lock/IsLockedRequest.h"
#include "hazelcast/client/lock/LockPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace lock {
            IsLockedRequest::IsLockedRequest(serialization::pimpl::Data &key)
            :key(key)
            , threadId(0) {
            };

            IsLockedRequest::IsLockedRequest(serialization::pimpl::Data &key, long threadId)
            :key(key)
            , threadId(threadId) {
            };

            int IsLockedRequest::getClassId() const {
                return LockPortableHook::IS_LOCKED;
            };

            int IsLockedRequest::getFactoryId() const {
                return LockPortableHook::FACTORY_ID;
            };

            void IsLockedRequest::write(serialization::PortableWriter &writer) const {
                writer.writeLong("tid", threadId);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                key.writeData(out);
            };


            bool IsLockedRequest::isRetryable() const {
                return true;
            }

        }
    }
}

