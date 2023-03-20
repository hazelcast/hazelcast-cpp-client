/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include <queue>
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
 * `cleanup_threshold_`, at which point the inserting thread will remove a batch
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
     * @param capacity Capacity of the cache
     * @param cleanup_threshold The size at which the cache will clean up oldest
     *    entries in batch. `cleanup_threshold - capacity` entries will be
     * removed
     * @throws exception::illegal_argument if capacity is smaller or equal to 0,
     * or if the cleanup_threshold is smaller than capacity
     */
    explicit read_optimized_lru_cache(const uint32_t capacity,
                                      const uint32_t cleanup_threshold)
    {
        if (capacity == 0) {
            BOOST_THROW_EXCEPTION(
              client::exception::illegal_argument("capacity == 0"));
        }
        if (cleanup_threshold <= capacity) {
            BOOST_THROW_EXCEPTION(client::exception::illegal_argument(
              "cleanupThreshold <= capacity"));
        }

        capacity_ = capacity;
        cleanup_threshold_ = cleanup_threshold;
    }

    /**
     * @param key the key of the cache entry
     * @param default_value the default value if the key is not cached.
     * @returns Returns the value to which the specified key is cached,
     * or default value if this cache contains no mapping for the key.
     */
    std::shared_ptr<V> get_or_default(const K& key,
                                      const std::shared_ptr<V>& default_value)
    {
        const auto existing_value = get(key);
        return (existing_value != nullptr) ? existing_value : default_value;
    }

    /**
     * @param key the key of the cache entry
     * Returns the value to which the specified key is cached,
     * or {@code null} if this cache contains no mapping for the key.
     * @returns Returns the value to which the specified key is cached
     */
    std::shared_ptr<V> get(const K& key)
    {
        auto value_from_cache = cache_.get(key);
        if (value_from_cache == nullptr) {
            return nullptr;
        }
        value_from_cache->touch();
        return std::make_shared<int32_t>(value_from_cache->value_);
    }

    /**
     * @param key the key of the cache entry
     * @param value the value of the cache entry
     * @throws exception::illegal_argument if the value equals to nullptr
     */
    void put(const K& key, const std::shared_ptr<V>& value)
    {
        if (value == nullptr) {
            BOOST_THROW_EXCEPTION(client::exception::illegal_argument(
              "Null values are disallowed"));
        }

        auto old_value =
          cache_.put(key, std::make_shared<value_and_timestamp<V>>(*value));
        if (old_value == nullptr && cache_.size() > cleanup_threshold_) {
            do_cleanup();
        }
    }

    /**
     * @param key the key of the cache entry
     * Removes the cached value for the given key
     */
    void remove(const K& key) { cache_.remove(key); }

protected:
    /**
     * Helper class to for simulation atomic lock with RAII
     */
    class custom_atomic_lock
    {
    public:
        explicit custom_atomic_lock(bool initial_value = false)
          : lock_(initial_value)
        {
        }

        custom_atomic_lock(const custom_atomic_lock&) = delete;
        const custom_atomic_lock& operator=(const custom_atomic_lock&) = delete;

        bool try_lock()
        {
            bool expected = false;
            return lock_.compare_exchange_strong(expected, true);
        }

        void release() { lock_.store(false); }

        ~custom_atomic_lock() { release(); }

    private:
        std::atomic<bool> lock_;
    };

    /**
     * Helper class to hold the value with timestamp.
     */
    template<typename T>
    class value_and_timestamp
    {
    public:
        const T value_;
        int64_t timestamp_;

        value_and_timestamp(T value)
          : value_(value)
        {
            touch();
        }

        void touch() { timestamp_ = util::current_time_nanos(); }
    };

    util::SynchronizedMap<K, value_and_timestamp<V>> cache_;

private:
    /**
     * Cleans the cache
     */
    void do_cleanup()
    {
        // if no thread is cleaning up, we'll do it
        if (!cleanup_lock_.try_lock()) {
            return;
        }

        try {
            if (capacity_ >= cache_.size()) {
                // this can happen if the cache is concurrently modified
                return;
            }
            auto entries_to_remove = cache_.size() - capacity_;

            /*max heap*/
            std::priority_queue<int64_t> oldest_timestamps;

            // 1st pass
            const auto values = cache_.values();
            for (const auto& value_and_timestamp : values) {
                oldest_timestamps.push(value_and_timestamp->timestamp_);
                if (oldest_timestamps.size() > entries_to_remove) {
                    oldest_timestamps.pop();
                }
            }

            // find out the highest value in the queue - the value, below which
            // entries will be removed
            if (oldest_timestamps.empty()) {
                // this can happen if the cache is concurrently modified
                return;
            }
            int64_t remove_threshold = oldest_timestamps.top();
            oldest_timestamps.pop();

            // 2nd pass
            cache_.remove_values_if(
              [remove_threshold](const value_and_timestamp<V>& v) -> bool {
                  return (v.timestamp_ <= remove_threshold);
              });

            cleanup_lock_.release();
        } catch (std::exception& e) {
            throw;
        }
    }

    custom_atomic_lock cleanup_lock_;
    uint32_t capacity_;
    uint32_t cleanup_threshold_;
};

} // namespace impl
} // namespace sql
} // namespace client
} // namespace hazelcast
