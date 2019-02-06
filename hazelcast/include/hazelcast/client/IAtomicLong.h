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
#ifndef HAZELCAST_CLIENT_IATOMICLONG_H_
#define HAZELCAST_CLIENT_IATOMICLONG_H_

#include <boost/shared_ptr.hpp>

#include "hazelcast/client/impl/AtomicLongInterface.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            class HazelcastClientInstanceImpl;
        }

        /**
         * IAtomicLong is a redundant and highly available distributed alternative to
         * the {@link java.util.concurrent.atomic.AtomicLong}.
         * <p>
         * Asynchronous variants of all methods have been introduced in Hazelcast version 3.7.
         * Async methods return immediately an {@link ICompletableFuture} from which
         * the operation's result can be obtained either in a blocking manner or by
         * registering a callback to be executed upon completion. For example:
         * <pre><code>
         * boost::shared_ptr<ICompletableFuture<int64_t> > future = atomicLong.addAndGetAsync(13);
         * future->andThen(boost::shared_ptr<ExecutionCallback<V> >(new  MyExecutionCallback()));
         * </code></pre>
         * During a network partition event it is possible for the {@link IAtomicLong}
         * to exist in each of the partitioned clusters or to not exist at all. Under
         * these circumstances the values held in the {@link IAtomicLong} may diverge.
         * Once the network partition heals, Hazelcast will use the configured
         * split-brain merge policy to resolve conflicting values.
         * <p>
         * Supports Quorum since 3.10 in cluster versions 3.10 and higher.
         *
         */
        class HAZELCAST_API IAtomicLong : public impl::AtomicLongInterface {
            friend class impl::HazelcastClientInstanceImpl;
        public:
            /**
            * adds the given value to the current value.
            *
            * @param delta the value to add
            * @return the updated value
            */
            int64_t addAndGet(int64_t delta);

            /**
            * sets the value to the given updated value
            * only if the current value is equal to the expected value.
            *
            * @param expect the expected value
            * @param update the new value
            * @return true if successful; or false if the actual value
            *         was not equal to the expected value.
            */
            bool compareAndSet(int64_t expect, int64_t update);

            /**
            * decrements the current value by one.
            *
            * @return the updated value
            */
            int64_t decrementAndGet();

            /**
            * Gets the current value.
            *
            * @return the current value
            */
            int64_t get();

            /**
            * adds the given value to the current value.
            *
            * @param delta the value to add
            * @return the old value before the add
            */
            int64_t getAndAdd(int64_t delta);

            /**
            * sets the given value and returns the old value.
            *
            * @param newValue the new value
            * @return the old value
            */
            int64_t getAndSet(int64_t newValue);

            /**
            * increments the current value by one.
            *
            * @return the updated value
            */
            int64_t incrementAndGet();

            /**
            * increments the current value by one.
            *
            * @return the old value
            */
            int64_t getAndIncrement();

            /**
            * sets the given value.
            *
            * @param newValue the new value
            */
            void set(int64_t newValue);

            virtual const std::string &getServiceName() const;

            virtual const std::string &getName() const;

            virtual void destroy();

            /**
             * Atomically adds the given value to the current value.
             * <p>
             * This method will dispatch a request and return immediately an
             * {@link ICompletableFuture}.
             * <p>
             * The operations result can be obtained in a blocking way, or a callback
             * can be provided for execution upon completion, as demonstrated in the
             * following examples:
             * <pre><code>
             * boost::shared_ptr<ICompletableFuture<int64_t> > future = atomicLong.addAndGetAsync(13);
             * // do something else, then read the result
             *
             * // this method will block until the result is available
             * int64_t result = future.get();
             * </code></pre>
             * <pre><code>
             *   future->andThen(boost::shared_ptr<ExecutionCallback<V> >(new  MyExecutionCallback()));
             * </code></pre>
             *
             * @param delta the value to add
             * @return an {@link ICompletableFuture} bearing the response
             * @since cluster version 3.7
             */
            boost::shared_ptr<ICompletableFuture<int64_t> > addAndGetAsync(int64_t delta);

            /**
             * Atomically sets the value to the given updated value
             * only if the current value {@code ==} the expected value.
             * <p>
             * This method will dispatch a request and return immediately an
             * {@link ICompletableFuture}.
             *
             * @param expect the expected value
             * @param update the new value
             * @return an {@link ICompletableFuture} with value {@code true} if successful;
             * or {@code false} if the actual value was not equal to the expected value
             * @since cluster version 3.7
             */
            boost::shared_ptr<ICompletableFuture<bool> > compareAndSetAsync(int64_t expect, int64_t update);

            /**
             * Atomically decrements the current value by one.
             * <p>
             * This method will dispatch a request and return immediately an
             * {@link ICompletableFuture}.
             *
             * @return an {@link ICompletableFuture} with the updated value
             * @since cluster version 3.7
             */
            boost::shared_ptr<ICompletableFuture<int64_t> > decrementAndGetAsync();

            /**
             * Gets the current value. This method will dispatch a request and return
             * immediately an {@link ICompletableFuture}.
             *
             * @return an {@link ICompletableFuture} with the current value
             * @since cluster version 3.7
             */
            boost::shared_ptr<ICompletableFuture<int64_t> > getAsync();

            /**
             * Atomically adds the given value to the current value.
             * <p>
             * This method will dispatch a request and return immediately an
             * {@link ICompletableFuture}.
             *
             * @param delta the value to add
             * @return an {@link ICompletableFuture} with the old value before the addition
             * @since cluster version 3.7
             */
            boost::shared_ptr<ICompletableFuture<int64_t> > getAndAddAsync(int64_t delta);

            /**
             * Atomically sets the given value and returns the old value.
             * <p>
             * This method will dispatch a request and return immediately an
             * {@link ICompletableFuture}.
             *
             * @param newValue the new value
             * @return an {@link ICompletableFuture} with the old value
             * @since cluster version 3.7
             */
            boost::shared_ptr<ICompletableFuture<int64_t> > getAndSetAsync(int64_t newValue);

            /**
             * Atomically increments the current value by one.
             * <p>
             * This method will dispatch a request and return immediately an
             * {@link ICompletableFuture}.
             *
             * @return an {@link ICompletableFuture} with the updated value
             * @since cluster version 3.7
             */
            boost::shared_ptr<ICompletableFuture<int64_t> > incrementAndGetAsync();

            /**
             * Atomically increments the current value by one.
             * <p>
             * This method will dispatch a request and return immediately an
             * {@link ICompletableFuture}.
             *
             * @return an {@link ICompletableFuture} with the old value
             * @since cluster version 3.7
             */
            boost::shared_ptr<ICompletableFuture<int64_t> > getAndIncrementAsync();

            /**
             * Atomically sets the given value.
             * <p>
             * This method will dispatch a request and return immediately an
             * {@link ICompletableFuture}.
             *
             * @param newValue the new value
             * @return an {@link ICompletableFuture}
             * @since cluster version 3.7
             */
            boost::shared_ptr<ICompletableFuture<void> > setAsync(int64_t newValue);

        private:
            IAtomicLong(const boost::shared_ptr<impl::AtomicLongInterface> &impl);

            boost::shared_ptr<impl::AtomicLongInterface> impl;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_IATOMICLONG_H_ */
