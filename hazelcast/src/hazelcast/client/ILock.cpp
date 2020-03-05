/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"

#include "hazelcast/util/Util.h"
#include "hazelcast/client/impl/ClientLockReferenceIdGenerator.h"

namespace hazelcast {
    namespace client {
        ILock::ILock(const std::string &instanceName, spi::ClientContext *context)
                : proxy::ProxyImpl("hz:impl:lockService", instanceName, context),
                  key(toData<std::string>(instanceName)) {
            partitionId = getPartitionId(key);

            // TODO: remove this line once the client instance getDistributedObject works as expected in Java for this proxy type
            referenceIdGenerator = context->getLockReferenceIdGenerator();
        }

        void ILock::lock() {
            lock(-1);
        }

        void ILock::lock(long leaseTimeInMillis) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockLockCodec::encodeRequest(getName(), leaseTimeInMillis,
                                                                  util::getCurrentThreadId(),
                                                                  referenceIdGenerator->getNextReferenceId());

            invokeOnPartition(request, partitionId);
        }

        void ILock::unlock() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockUnlockCodec::encodeRequest(getName(), util::getCurrentThreadId(),
                                                                    referenceIdGenerator->getNextReferenceId());

            invokeOnPartition(request, partitionId);
        }

        void ILock::forceUnlock() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockForceUnlockCodec::encodeRequest(getName(),
                                                                         referenceIdGenerator->getNextReferenceId());

            invokeOnPartition(request, partitionId);
        }

        bool ILock::isLocked() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockIsLockedCodec::encodeRequest(getName());

            return invokeAndGetResult<bool, protocol::codec::LockIsLockedCodec::ResponseParameters>(request,
                                                                                                    partitionId);
        }

        bool ILock::isLockedByCurrentThread() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockIsLockedByCurrentThreadCodec::encodeRequest(getName(),
                                                                                     util::getCurrentThreadId());

            return invokeAndGetResult<bool, protocol::codec::LockIsLockedByCurrentThreadCodec::ResponseParameters>(
                    request, partitionId);
        }

        int ILock::getLockCount() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockGetLockCountCodec::encodeRequest(getName());

            return invokeAndGetResult<int, protocol::codec::LockGetLockCountCodec::ResponseParameters>(request,
                                                                                                       partitionId);
        }

        long ILock::getRemainingLeaseTime() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockGetRemainingLeaseTimeCodec::encodeRequest(getName());

            return invokeAndGetResult<long, protocol::codec::LockGetRemainingLeaseTimeCodec::ResponseParameters>(
                    request, partitionId);
        }

        bool ILock::tryLock() {
            return tryLock(0);
        }

        bool ILock::tryLock(long timeInMillis) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockTryLockCodec::encodeRequest(getName(), util::getCurrentThreadId(), LONG_MAX,
                                                                     timeInMillis,
                                                                     referenceIdGenerator->getNextReferenceId());

            return invokeAndGetResult<bool, protocol::codec::LockTryLockCodec::ResponseParameters>(request,
                                                                                                   partitionId);
        }

        void ILock::onInitialize() {
            ProxyImpl::onInitialize();

            referenceIdGenerator = getContext().getLockReferenceIdGenerator();
        }
    }
}
