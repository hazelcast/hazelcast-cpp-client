//
// Created by sancar koyunlu on 6/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/ILock.h"
#include "hazelcast/client/lock/LockRequest.h"
#include "hazelcast/client/lock/UnlockRequest.h"
#include "hazelcast/client/lock/IsLockedRequest.h"
#include "hazelcast/client/lock/GetLockCountRequest.h"
#include "hazelcast/client/lock/GetRemainingLeaseRequest.h"

namespace hazelcast {
    namespace client {


        ILock::ILock(const std::string &instanceName, spi::ClientContext *context)
        : DistributedObject("hz:impl:lockService", instanceName, context)
        , key(context->getSerializationService().toData<std::string>(&instanceName)) {

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
            boost::shared_ptr<int> response = invoke<int>(request);
            return *response;
        };

        long ILock::getRemainingLeaseTime() {
            lock::GetRemainingLeaseRequest request(key);
            boost::shared_ptr<long> response = invoke<long>(request);
            return *response;
        };

        bool ILock::tryLock() {
            try {
                return tryLock(0);
            } catch (exception::IException &e) {
                return false;
            }
        };

        bool ILock::tryLock(long timeInMillis) {
            lock::LockRequest request(key, util::getThreadId(), -1, timeInMillis);
            boost::shared_ptr<bool> response = invoke<bool>(request);
            return *response;
        };

        void ILock::onDestroy() {
        };

    }
}