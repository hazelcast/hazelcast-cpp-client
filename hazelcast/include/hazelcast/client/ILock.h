//
// Created by sancar koyunlu on 6/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ILock
#define HAZELCAST_ILock

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/serialization/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        class HAZELCAST_API ILock : public DistributedObject {
            friend class HazelcastClient;

        public:

            void lock();

            /**
             * Acquires the lock for the specified lease time.
             * <p>After lease time, lock will be released..
             *
             * <p>If the lock is not available then
             * the current thread becomes disabled for thread scheduling
             * purposes and lies dormant until the lock has been acquired.
             *
             *
             * @param leaseTime time to wait before releasing the lock.
             * @param timeUnit unit of time to specify lease time.
             */
            void lock(long leaseTimeInMillis);

            void unlock();

            /**
             * Releases the lock regardless of the lock owner.
             * It always successfully unlocks, never blocks  and returns immediately.
             */
            void forceUnlock();

            bool isLocked();

            bool isLockedByCurrentThread();

            int getLockCount();

            /**
             * Returns remaining lease time in milliseconds.
             * If the lock is not locked then -1 will be returned.
             *
             * @return remaining lease time in milliseconds.
             */
            long getRemainingLeaseTime();

            bool tryLock();

            bool tryLock(long timeInMillis);

        private:

            ILock(const std::string &instanceName, spi::ClientContext *context);

            void onDestroy();

            serialization::Data key;

            int partitionId;
        };
    }
}

#endif //HAZELCAST_ILock
