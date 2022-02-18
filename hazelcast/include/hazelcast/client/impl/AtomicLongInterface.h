/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#pragma once

#include <stdint.h>

#include "hazelcast/client/distributed_object.h"

namespace hazelcast {
namespace client {
namespace impl {
class HAZELCAST_API AtomicLongInterface : public virtual distributed_object
{
public:
    /**
     * adds the given value to the current value.
     *
     * @param delta the value to add
     * @return the updated value
     */
    virtual int64_t addAndGet(int64_t delta) = 0;

    /**
     * sets the value to the given updated value
     * only if the current value is equal to the expected value.
     *
     * @param expect the expected value
     * @param update the new value
     * @return true if successful; or false if the actual value
     *         was not equal to the expected value.
     */
    virtual bool compareAndSet(int64_t expect, int64_t update) = 0;

    /**
     * decrements the current value by one.
     *
     * @return the updated value
     */
    virtual int64_t decrementAndGet() = 0;

    /**
     * Gets the current value.
     *
     * @return the current value
     */
    virtual int64_t get() = 0;

    /**
     * adds the given value to the current value.
     *
     * @param delta the value to add
     * @return the old value before the add
     */
    virtual int64_t getAndAdd(int64_t delta) = 0;

    /**
     * sets the given value and returns the old value.
     *
     * @param newValue the new value
     * @return the old value
     */
    virtual int64_t getAndSet(int64_t newValue) = 0;

    /**
     * increments the current value by one.
     *
     * @return the updated value
     */
    virtual int64_t incrementAndGet() = 0;

    /**
     * increments the current value by one.
     *
     * @return the old value
     */
    virtual int64_t getAndIncrement() = 0;

    /**
     * sets the given value.
     *
     * @param newValue the new value
     */
    virtual void set(int64_t newValue) = 0;

    /**
     * Atomically adds the given value to the current value.
     * <p>
     * This method will dispatch a request and return immediately an
     * {@link future}.
     * <p>
     * The operations result can be obtained in a blocking way, or a callback
     * can be provided for execution upon completion, as demonstrated in the
     * following examples:
     * <pre><code>
     * std::shared_ptr<boost::future<int64_t> > future =
     * atomicLong.addAndGetAsync(13);
     * // do something else, then read the result
     *
     * // this method will block until the result is available
     * int64_t result = future.get();
     * </code></pre>
     * <pre><code>
     *   future->andThen(std::shared_ptr<execution_callback<V> >(new
     * MyExecutionCallback()));
     * </code></pre>
     *
     * @param delta the value to add
     * @return an {@link future} bearing the response
     * @since cluster version 3.7
     */
    virtual boost::future<std::shared_ptr<int64_t>> addAndGetAsync(
      int64_t delta) = 0;

    /**
     * Atomically sets the value to the given updated value
     * only if the current value {@code ==} the expected value.
     * <p>
     * This method will dispatch a request and return immediately an
     * {@link future}.
     *
     * @param expect the expected value
     * @param update the new value
     * @return an {@link future} with value {@code true} if successful;
     * or {@code false} if the actual value was not equal to the expected value
     * @since cluster version 3.7
     */
    virtual boost::future<std::shared_ptr<bool>> compareAndSetAsync(
      int64_t expect,
      int64_t update) = 0;

    /**
     * Atomically decrements the current value by one.
     * <p>
     * This method will dispatch a request and return immediately an
     * {@link future}.
     *
     * @return an {@link future} with the updated value
     * @since cluster version 3.7
     */
    virtual boost::future<std::shared_ptr<int64_t>> decrementAndGetAsync() = 0;

    /**
     * Gets the current value. This method will dispatch a request and return
     * immediately an {@link future}.
     *
     * @return an {@link future} with the current value
     * @since cluster version 3.7
     */
    virtual boost::future<std::shared_ptr<int64_t>> getAsync() = 0;

    /**
     * Atomically adds the given value to the current value.
     * <p>
     * This method will dispatch a request and return immediately an
     * {@link future}.
     *
     * @param delta the value to add
     * @return an {@link future} with the old value before the addition
     * @since cluster version 3.7
     */
    virtual boost::future<std::shared_ptr<int64_t>> getAndAddAsync(
      int64_t delta) = 0;

    /**
     * Atomically sets the given value and returns the old value.
     * <p>
     * This method will dispatch a request and return immediately an
     * {@link future}.
     *
     * @param newValue the new value
     * @return an {@link future} with the old value
     * @since cluster version 3.7
     */
    virtual boost::future<std::shared_ptr<int64_t>> getAndSetAsync(
      int64_t newValue) = 0;

    /**
     * Atomically increments the current value by one.
     * <p>
     * This method will dispatch a request and return immediately an
     * {@link future}.
     *
     * @return an {@link future} with the updated value
     * @since cluster version 3.7
     */
    virtual boost::future<std::shared_ptr<int64_t>> incrementAndGetAsync() = 0;

    /**
     * Atomically increments the current value by one.
     * <p>
     * This method will dispatch a request and return immediately an
     * {@link future}.
     *
     * @return an {@link future} with the old value
     * @since cluster version 3.7
     */
    virtual boost::future<std::shared_ptr<int64_t>> getAndIncrementAsync() = 0;

    /**
     * Atomically sets the given value.
     * <p>
     * This method will dispatch a request and return immediately an
     * {@link future}.
     *
     * @param newValue the new value
     * @return an {@link future}
     * @since cluster version 3.7
     */
    virtual boost::future<void> setAsync(int64_t newValue) = 0;
};
} // namespace impl
} // namespace client
} // namespace hazelcast
