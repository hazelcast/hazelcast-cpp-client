/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/heap/priority_queue.hpp>
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/export.h"


namespace hazelcast {
namespace client {
namespace sql {
namespace impl {
 
/**
 * Implementation of an LRU cache optimized for read-heavy use cases.
 * <p>
 * It stores the entries in a {SynchronizedMap}, along with the last
 * access time. It allows the size to grow beyond the capacity, up to
 * `cleanupThreshold`, at which point the inserting thread will remove a batch
 * of the eldest items in two passes.
 * <p>
 * The cleanup process isn't synchronized to guarantee that the capacity is not
 * exceeded. The cache is available during the cleanup for reads and writes. If
 * there's a large number of writes by many threads, the one thread doing the
 * cleanup might not be quick enough and there's no upper bound on the actual
 * size of the cache. This is done to optimize the happy path when the keys fit
 * into the cache.
 */
template<typename K, typename V>    
class HAZELCAST_API read_optimized_lru_cache
{    
    public:

    /**
     * @param cleanup_threshold The size at which the cache will clean up oldest
     *     entries in batch. `cleanup_threshold - capacity` entries will be removed.
     */
    explicit read_optimized_lru_cache(int32_t capacity, int32_t cleanup_threshold) {
        if (capacity <= 0) {            
            BOOST_THROW_EXCEPTION( client::exception::illegal_argument("capacity <= 0"))
        }
        if (cleanup_threshold <= capacity) {
            BOOST_THROW_EXCEPTION( client::exception::illegal_argument("cleanupThreshold <= capacity") )            
        }

        capacity_ = capacity;
        cleanup_threshold_ = cleanup_threshold;
    }

    std::shared_ptr<V> get_or_default(K key, V default_value) {
        auto existing_value = get(key);
        return existing_value != nullptr ? existing_value : default_value;
    }

    std::shared_ptr<V> get(K key) {
        auto value_from_cache = cache.get(key);
        if (value_from_cache == nullptr) {
            return nullptr;
        }
        value_from_cache->touch();
        return value_from_cache->value;
    }

    void put(K key, std::shared_ptr<V> value) {
        if (value == nullptr) {
            BOOST_THROW_EXCEPTION(client::exception::illegal_argument("Null values are disallowed"));            
        }

        auto old_value = cache.put(key, make_shared<V>(ValueAndTimestamp(value)));
        if (old_value == nullptr && cache.size() > cleanup_threshold_) {
            do_cleanup();
        }
    }

    void remove(K key) {
        cache.remove(key);
    }

private:
    void do_cleanup() {
        // if no thread is cleaning up, we'll do it
        if (!cleanup_lock_.compare_and_set(false, true)) {
            return;
        }

        try {
            int entries_to_remove = cache.size() - capacity;
            if (entries_to_remove <= 0) {
                // this can happen if the cache is concurrently modified
                return;
            }
            std::priority_queue<int64_t, std::vector<int64_t>, std::greater<int64_t>> oldest_timestamps;                    

            // 1st pass
            auto values = cache.values();
            for (ValueAndTimestamp<V> valueAndTimestamp : values) {
                oldest_timestamps.push(valueAndTimestamp.timestamp);
                if (oldest_timestamps.size() > entriesToRemove) {
                    oldest_timestamps.pop();
                }
            }

            // find out the highest value in the queue - the value, below which entries will be removed
            if (oldest_timestamps.empty()) {
                // this can happen if the cache is concurrently modified
                return;
            }
            int64_t remove_threshold = oldest_timestamps.top();
            oldest_timestamps.pop();

            // 2nd pass
            cache.values().removeIf(v -> v.timestamp <= remove_threshold);
        } finally {
            cleanupLock.set(false);
        }
    }

    SynchronizedMap<K, ValueAndTimestamp<V>> cache_;
    std::atomic<bool> cleanup_lock_;
    int32_t capacity_;
    int32_t cleanup_threshold_;    

    // package-visible for tests
    static class ValueAndTimestamp<V> {
        private static final AtomicLongFieldUpdater<ValueAndTimestamp> TIMESTAMP_UPDATER =
                AtomicLongFieldUpdater.newUpdater(ValueAndTimestamp.class, "timestamp");

        final V value;
        volatile long timestamp;

        ValueAndTimestamp(V value) {
            this.value = value;
            touch();
        }

        public void touch() {
            TIMESTAMP_UPDATER.lazySet(this, System.nanoTime());
        }
    }
};

} // namespace impl
} // namespace sql
} // namespace client
} // namespace hazelcast
