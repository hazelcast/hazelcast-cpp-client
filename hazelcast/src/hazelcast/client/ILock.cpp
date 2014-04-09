//
// Created by sancar koyunlu on 6/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/ILock.h"
#include "hazelcast/client/lock/LockRequest.h"
#include "hazelcast/client/lock/UnlockRequest.h"
#include "hazelcast/client/lock/IsLockedRequest.h"
#include "hazelcast/client/lock/GetLockCountRequest.h"
#include "hazelcast/client/lock/GetRemainingLeaseRequest.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {


        ILock::ILock(const std::string &instanceName, spi::ClientContext *context)
        : DistributedObject("hz:impl:lockService", instanceName, context)
        , key(context->getSerializationService().toData<std::string>(&instanceName)) {
            partitionId = getPartitionId(key);
        };

        void ILock::lock() {
            lock(-1);
        };

        void ILock::lock(long leaseTimeInMillis) {
            lock::LockRequest *request = new lock::LockRequest(key, util::getThreadId(), leaseTimeInMillis, -1);
            invoke<serialization::pimpl::Void>(request, partitionId);
        };

        void ILock::unlock() {
            lock::UnlockRequest *request = new lock::UnlockRequest(key, util::getThreadId(), false);
            invoke<serialization::pimpl::Void>(request, partitionId);
        };

        void ILock::forceUnlock() {
            lock::UnlockRequest *request = new lock::UnlockRequest(key, util::getThreadId(), true);
            invoke<serialization::pimpl::Void>(request, partitionId);
        };

        bool ILock::isLocked() {
            lock::IsLockedRequest *request = new lock::IsLockedRequest(key);
            return *(invoke<bool>(request, partitionId));
        };

        bool ILock::isLockedByCurrentThread() {
            lock::IsLockedRequest *request = new lock::IsLockedRequest(key, util::getThreadId());
            return *(invoke<bool>(request, partitionId));
        };

        int ILock::getLockCount() {
            lock::GetLockCountRequest *request = new lock::GetLockCountRequest(key);
            boost::shared_ptr<int> response = invoke<int>(request, partitionId);
            return *response;
        };

        long ILock::getRemainingLeaseTime() {
            lock::GetRemainingLeaseRequest *request = new lock::GetRemainingLeaseRequest(key);
            boost::shared_ptr<long> response = invoke<long>(request, partitionId);
            return *response;
        };

        bool ILock::tryLock() {
            return tryLock(0);
        };

        bool ILock::tryLock(long timeInMillis) {
            lock::LockRequest *request = new lock::LockRequest(key, util::getThreadId(), -1, timeInMillis);
            boost::shared_ptr<bool> response = invoke<bool>(request, partitionId);
            return *response;
        };

        void ILock::onDestroy() {
        };

    }
}
