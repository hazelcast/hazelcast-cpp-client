/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_ILock
#define HAZELCAST_ILock

#include <string>

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/impl/ClientLockReferenceIdGenerator.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {

        /**
        * Re-entrant Lock, Distributed client implementation of Lock.
        *
        */
        class HAZELCAST_API ILock : public proxy::ProxyImpl {
            friend class impl::HazelcastClientInstanceImpl;

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
            * @param leaseTimeInMillis time to wait before releasing the lock.
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
            * @param timeInMillis time to wait
            * @return true if lock is get, false otherwise.
            */

            bool tryLock(long timeInMillis);

        protected:
            virtual void onInitialize();

        private:

            ILock(const std::string& instanceName, spi::ClientContext *context);

            serialization::pimpl::Data key;
            int partitionId;
            boost::shared_ptr<impl::ClientLockReferenceIdGenerator> referenceIdGenerator;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_ILock

