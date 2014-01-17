//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/lock/GetLockCountRequest.h"
#include "hazelcast/client/lock/LockPortableHook.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace lock {
            GetLockCountRequest::GetLockCountRequest(serialization::Data& key)
            :key(key) {
            };

            int GetLockCountRequest::getClassId() const {
                return LockPortableHook::GET_LOCK_COUNT;
            };

            int GetLockCountRequest::getFactoryId() const {
                return LockPortableHook::FACTORY_ID;
            };


            bool GetLockCountRequest::isRetryable() const {
                return true;
            }

            void GetLockCountRequest::write(serialization::PortableWriter& writer) const {
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };
        }
    }
}
