//
// Created by sancar koyunlu on 6/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ILock
#define HAZELCAST_ILock

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {

        /**
         * Re-entrant Lock, Distributed client implementation of Lock.
         *
         */
        class HAZELCAST_API ILock : public DistributedObject {
            friend class HazelcastClient;

        public:

            /**
             * Acquires the lock.
             * If lock is not available waits for unlock indefinitely
             */
            void lock();

            /**
             * Gets the lock for the specified lease time.
             * After lease time, lock will be released..
             *
             * @param leaseTime time to wait before releasing the lock.
             * @param timeUnit unit of time to specify lease time.
             */
            void lock(long leaseTimeInMillis);

            /**
             * Releases the lock
             */
            void unlock();

            /**
             * Releases the lock regardless of the lock owner.
             * It always successfully unlocks, never blocks  and returns immediately.
             */
            void forceUnlock();

            /**
             *
             * @return true if this lock is locked, false otherwise.
             */
            bool isLocked();

            /**
             *
             * @return true if this lock is locked by current thread, false otherwise.
             */
            bool isLockedByCurrentThread();

            /**
             * Returns re-entrant lock hold count, regardless of lock ownership.
             *
             * @return lock hold count.
             */
            int getLockCount();

            /**
             * Returns remaining lease time in milliseconds.
             * If the lock is not locked then -1 will be returned.
             *
             * @return remaining lease time in milliseconds.
             */
            long getRemainingLeaseTime();

            /**
             * Tries to acquire the lock. Returns immediately without blocking.
             *
             * @return true if lock is get, false otherwise.
             */
            bool tryLock();

            /**
             * Tries to acquire the lock. Returns after timeInMillis seconds
             *
             * @param long timeInMillis time to wait
             * @return true if lock is get, false otherwise.
             */

            bool tryLock(long timeInMillis);

        private:

            ILock(const std::string &instanceName, spi::ClientContext *context);

            void onDestroy();

            serialization::pimpl::Data key;

            int partitionId;
        };
    }
}

#endif //HAZELCAST_ILock
