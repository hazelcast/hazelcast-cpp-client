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
#ifndef HAZELCAST_ICOUNT_DOWN_LATCH
#define HAZELCAST_ICOUNT_DOWN_LATCH

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/exception/IException.h"
#include <string>
#include <stdexcept>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {

        namespace spi {
            class ClientContext;
        }

        /**
         * ICountDownLatch is a backed-up distributed alternative to the
         * java.util.concurrent.CountDownLatch java.util.concurrent.CountDownLatch
         *
         * ICountDownLatch is a cluster-wide synchronization aid
         * that allows one or more threads to wait until a set of operations being
         * performed in other threads completes.
         *
         * There are a few differences compared to the ICountDownLatch :
         * <ol>
         *    <li>
         *         the ICountDownLatch count can be re-set using trySetCount(int) after a countdown
         *         has finished but not during an active count. This allows the same latch instance to be reused.
         *    </li>
         *    <li>
         *         there is no await() method to do an unbound wait since this is undesirable in a distributed
         *         application: it can happen that for example a cluster is split or that the master and
         *         replica's all die. So in most cases it is best to configure an explicit timeout so have the ability
         *         to deal with these situations.
         *    </li>
         * </ol>
         *
         */
        class HAZELCAST_API ICountDownLatch : public proxy::ProxyImpl {
            friend class HazelcastClient;

        public:

            /**
             * Causes the current thread to wait until the latch has counted down to
             * zero, an exception is thrown, or the specified waiting time elapses.
             *
             * <p>If the current count is zero then this method returns immediately
             * with the value true.
             *
             * <p>If the current count is greater than zero then the current
             * thread becomes disabled for thread scheduling purposes and lies
             * dormant until one of five things happen:
             * <ul>
             * <li>The count reaches zero due to invocations of the
             * #countDown method;
             * <li>This ICountDownLatch instance is destroyed;
             * <li>The countdown owner becomes disconnected;
             * <li>Some other thread interrupts
             * the current thread; or
             * <li>The specified waiting time elapses.
             * </ul>
             *
             * <p>If the count reaches zero then the method returns with the
             * value true.
             *
             * If the countdown owner becomes disconnected while waiting then
             * MemberLeftException will be thrown.
             * <p>If the current thread:
             * <ul>
             * <li>has its interrupted status set on entry to this method; or
             * <li>is interrupted while waiting,
             * </ul>
             * then InterruptedException is thrown and the current thread's
             * interrupted status is cleared.
             * <p>If the specified waiting time elapses then the value false
             * is returned.  If the time is less than or equal to zero, the method
             * will not wait at all.
             *
             * @param timeoutInMillis the maximum time to wait
             * @return true if the count reached zero and false
             *         if the waiting time elapsed before the count reached zero
             */
            bool await(long timeoutInMillis);

            /**
             * Decrements the count of the latch, releasing all waiting threads if
             * the count reaches zero.
             *
             * If the current count is greater than zero then it is decremented.
             * If the new count is zero:
             * <ul>
             * <li>All waiting threads are re-enabled for thread scheduling purposes
             * </ul>
             *
             * If the current count equals zero then nothing happens.
             */
            void countDown();

            /**
             * Returns the current count.
             *
             * @return current count
             */
            int getCount();

            /**
             * Sets the count to the given value if the current count is zero. The calling
             * cluster member becomes the owner of the countdown and is responsible for
             * staying connected to the cluster until the count reaches zero.
             * If the owner becomes disconnected before the count reaches zero:
             * <ul>
             * <li>Count will be set to zero;
             * <li>All awaiting threads will be thrown a MemberLeftException.
             * </ul>
             * If count is not zero then this method does nothing and returns false.
             *
             * @param count the number of times #countDown must be invoked
             *              before threads can pass through #await
             * @return true if the new count was set or false if the current
             *         count is not zero
             * @throws IllegalArgumentException if count is negative
             */
            bool trySetCount(int count);

        private:
            ICountDownLatch(const std::string &objectName, spi::ClientContext *clientContext);

            serialization::pimpl::Data key;

            int partitionId;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_ICOUNT_DOWN_LATCH */
