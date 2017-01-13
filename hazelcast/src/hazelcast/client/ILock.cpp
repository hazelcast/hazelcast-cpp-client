/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 6/27/13.



#include <limits.h>
#include "hazelcast/client/ILock.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/LockIsLockedCodec.h"
#include "hazelcast/client/protocol/codec/LockIsLockedByCurrentThreadCodec.h"
#include "hazelcast/client/protocol/codec/LockGetLockCountCodec.h"
#include "hazelcast/client/protocol/codec/LockGetRemainingLeaseTimeCodec.h"
#include "hazelcast/client/protocol/codec/LockLockCodec.h"
#include "hazelcast/client/protocol/codec/LockUnlockCodec.h"
#include "hazelcast/client/protocol/codec/LockForceUnlockCodec.h"
#include "hazelcast/client/protocol/codec/LockTryLockCodec.h"

#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        ILock::ILock(const std::string& instanceName, spi::ClientContext *context)
        : proxy::ProxyImpl("hz:impl:lockService", instanceName, context)
        , key(toData<std::string>(instanceName)) {
            partitionId = getPartitionId(key);
        }

        void ILock::lock() {
            lock(-1);
        }

        void ILock::lock(long leaseTimeInMillis) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockLockCodec::RequestParameters::encode(getName(), leaseTimeInMillis, util::getThreadId());

            invoke(request, partitionId);
        }

        void ILock::unlock() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockUnlockCodec::RequestParameters::encode(getName(), util::getThreadId());

            invoke(request, partitionId);
        }

        void ILock::forceUnlock() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockForceUnlockCodec::RequestParameters::encode(getName());

            invoke(request, partitionId);
        }

        bool ILock::isLocked() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockIsLockedCodec::RequestParameters::encode(getName());

            return invokeAndGetResult<bool, protocol::codec::LockIsLockedCodec::ResponseParameters>(request, partitionId);
        }

        bool ILock::isLockedByCurrentThread() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockIsLockedByCurrentThreadCodec::RequestParameters::encode(getName(), util::getThreadId());

            return invokeAndGetResult<bool, protocol::codec::LockIsLockedByCurrentThreadCodec::ResponseParameters>(request, partitionId);
        }

        int ILock::getLockCount() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockGetLockCountCodec::RequestParameters::encode(getName());

            return invokeAndGetResult<int,  protocol::codec::LockGetLockCountCodec::ResponseParameters>(request, partitionId);
        }

        long ILock::getRemainingLeaseTime() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockGetRemainingLeaseTimeCodec::RequestParameters::encode(getName());

            return invokeAndGetResult<long, protocol::codec::LockGetRemainingLeaseTimeCodec::ResponseParameters>(request, partitionId);
        }

        bool ILock::tryLock() {
            return tryLock(0);
        }

        bool ILock::tryLock(long timeInMillis) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockTryLockCodec::RequestParameters::encode(getName(), util::getThreadId(), LONG_MAX, timeInMillis);

            return invokeAndGetResult<bool, protocol::codec::LockTryLockCodec::ResponseParameters>(request, partitionId);
        }
    }
}
