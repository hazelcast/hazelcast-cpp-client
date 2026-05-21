/*
 * Copyright (c) 2008-2026, Hazelcast, Inc. All Rights Reserved.
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

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>

#include <boost/smart_ptr/atomic_shared_ptr.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/thread/future.hpp>

#include "hazelcast/client/impl/id_batch.h"
#include "hazelcast/util/export.h"

namespace hazelcast {
namespace util {
class hz_thread_pool;
}
namespace client {
namespace impl {

/**
 * Serves IDs from an {@link id_batch} one by one, fetching a new batch when
 * the current one is spent or expired.
 *
 * This is the C++ analogue of com.hazelcast.flakeidgen.impl.AutoBatcher. The
 * Java client's {@code AutoBatcher.newId()} is synchronous: it holds a lock
 * across a blocking batch fetch and loops until it has an ID. The C++ client
 * exposes an async API, so the equivalent guarantees are achieved without
 * blocking the caller or an IO thread:
 *
 *  - lock-free fast path while the current batch has IDs;
 *  - single-flight batch fetch: concurrent callers that hit an exhausted
 *    batch coalesce onto one outstanding request (no thundering herd);
 *  - the remote caller thread that fetches the batch range and its retry loop
 *    run on the user executor, so the caller thread and IO threads are
 *    never blocked;
 *  - Always returns a valid id (different from INT64_MIN)
 *
 * The batch supplier is injected, mirroring Java's IdBatchSupplier "separate
 * class due to testability" so the logic is unit-testable without a cluster.
 */
class HAZELCAST_API auto_batcher
{
public:
    using id_batch_supplier = std::function<boost::future<id_batch>(int32_t)>;

    auto_batcher(int32_t batch_size,
                 std::chrono::milliseconds validity,
                 util::hz_thread_pool& executor,
                 id_batch_supplier supplier);

    /**
     * Returns the next ID from the current batch, or fetches a new batch from
     * the supplier if the current batch is spent or expired. Never resolves
     * to INT64_MIN.
     */
    boost::future<int64_t> new_id();

private:
    class block
    {
    public:
        block(id_batch batch, std::chrono::milliseconds validity);

        /**
         * Returns the next ID, or INT64_MIN if this block is exhausted or
         * expired.
         */
        int64_t next();

        /**
         * Non-consuming check: true if the block is not expired and still has
         * at least one unreturned ID. Used by the fetch loop to decide
         * whether a freshly fetched batch is worth installing.
         */
        bool usable() const;

    private:
        id_batch id_batch_;
        std::chrono::steady_clock::time_point invalid_since_;
        std::atomic<int32_t> num_returned_;
    };

    const int32_t batch_size_;
    const std::chrono::milliseconds validity_;
    util::hz_thread_pool& executor_;
    id_batch_supplier supplier_;

    boost::atomic_shared_ptr<block> block_;

    std::mutex mutex_;
    boost::shared_future<boost::shared_ptr<block>> fetch_in_progress_;
    uint64_t fetch_generation_;
};

} // namespace impl
} // namespace client
} // namespace hazelcast
