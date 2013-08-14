//
// Created by sancar koyunlu on 6/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/lock/LockRequest.h"
#include "hazelcast/client/lock/UnlockRequest.h"
#include "hazelcast/client/lock/IsLockedRequest.h"
#include "hazelcast/client/lock/GetLockCountRequest.h"
#include "hazelcast/client/lock/GetRemainingLeaseRequest.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/lock/DestroyRequest.h"
#include "hazelcast/client/spi/DistributedObjectListenerService.h"

namespace hazelcast {
    namespace client {


        ILock::ILock() {

        };

        void ILock::init(const std::string& instanceName, spi::ClientContext *clientContext) {
            this->context = clientContext;
            this->instanceName = instanceName;
            key = context->getSerializationService().toData<std::string>(&instanceName);
        };

        void ILock::lock() {
            lock(-1);
        };

        void ILock::lock(long leaseTimeInMillis) {
            lock::LockRequest request(key, util::getThreadId(), leaseTimeInMillis, -1);
            invoke<bool>(request);
        };

        void ILock::unlock() {
            lock::UnlockRequest request(key, util::getThreadId(), false);
            invoke<bool>(request);
        };

        void ILock::forceUnlock() {
            lock::UnlockRequest request(key, util::getThreadId(), true);
            invoke<bool>(request);
        };

        bool ILock::isLocked() {
            lock::IsLockedRequest request(key);
            return invoke<bool>(request);
        };

        bool ILock::isLockedByCurrentThread() {
            lock::IsLockedRequest request(key, util::getThreadId());
            return invoke<bool>(request);
        };

        int ILock::getLockCount() {
            lock::GetLockCountRequest request(key);
            return invoke<int>(request);
        };

        long ILock::getRemainingLeaseTime() {
            lock::GetRemainingLeaseRequest request(key);
            return invoke<long>(request);
        };

        bool ILock::tryLock() {
            try {
                return tryLock(0);
            } catch (exception::IException& e) {
                return false;
            }
        };

        bool ILock::tryLock(long timeInMillis) {
            lock::LockRequest request(key, util::getThreadId(), -1, timeInMillis);
            return invoke<bool>(request);
        };

        void ILock::destroy() {
            lock::DestroyRequest request(key);
            invoke<bool>(request);
            context->getDistributedObjectListenerService().removeDistributedObject(instanceName);
        };

    }
}