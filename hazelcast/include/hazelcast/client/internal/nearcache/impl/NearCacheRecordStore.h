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

#include <assert.h>
#include <memory>

#include "hazelcast/client/spi/InitializingObject.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {
class data;
}
} // namespace serialization

namespace monitor {
class near_cache_stats;
}

namespace internal {
namespace nearcache {
namespace impl {
/**
 * {@link NearCacheRecordStore} is the contract point to store keys and values
 * as
 * {@link NearCacheRecord} internally and to serve them.
 *
 * @param <K> the type of the key
 * @param <V> the type of the value
 */
template<typename K, typename V>
class NearCacheRecordStore : public spi::InitializingObject
{
public:
    ~NearCacheRecordStore() override = default;

    /**
     * Gets the value associated with the given {@code key}.
     *
     * @param key the key from which to get the associated value.
     * @return the value associated with the given {@code key}.
     */
    virtual std::shared_ptr<V> get(const std::shared_ptr<K>& key)
    {
        assert(0);
        return std::shared_ptr<V>();
    }

    /**
     * Puts (associates) a value with the given {@code key}.
     *
     * @param key   the key to which the given value will be associated.
     * @param value the value that will be associated with the key.
     */
    virtual void put(const std::shared_ptr<K>& key,
                     const std::shared_ptr<V>& value)
    {
        assert(0);
    }

    /**
     * Puts (associates) a value with the given {@code key}.
     *
     * @param key   the key to which the given value will be associated.
     * @param value the value that will be associated with the key.
     */
    /*
                            virtual void put(const std::shared_ptr<K> &key,
                                             const
       std::shared_ptr<serialization::pimpl::data> &value) { assert(0);
                            }
    */

    /**
     * Removes the value associated with the given {@code key}
     * and increases the invalidation statistics.
     *
     * @param key the key of the value will be invalidated
     */
    virtual bool invalidate(const std::shared_ptr<K>& key)
    {
        assert(0);
        return false;
    }

    /**
     * Removes all stored values.
     */
    virtual void clear() { assert(0); }

    /**
     * Clears the record store and destroys it.
     */
    virtual void destroy() { assert(0); }

    /**
     * Get the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor
     * this record store.
     *
     * @return the {@link com.hazelcast.monitor.NearCacheStats} instance to
     * monitor this record store.
     */
    virtual std::shared_ptr<monitor::near_cache_stats> get_near_cache_stats()
      const = 0;

    /**
     * Selects the best candidate object to store from the given {@code
     * candidates}.
     *
     * @param candidates the candidates from which the best candidate object
     * will be selected.
     * @return the best candidate object to store, selected from the given
     * {@code candidates}.
     */
    virtual const std::shared_ptr<V> select_to_save(
      const std::shared_ptr<V>& value,
      const std::shared_ptr<serialization::pimpl::data>& value_data) const
    {
        assert(0);
        return std::shared_ptr<V>();
    }

    /**
     * Gets the number of stored records.
     *
     * @return the number of stored records.
     */
    virtual int size() const
    {
        assert(0);
        return -1;
    }

    /**
     * Performs expiration and evicts expired records.
     */
    virtual void do_expiration() { assert(0); }

    /**
     * Does eviction as specified configuration {@link
     * com.hazelcast.config.EvictionConfig} in {@link
     * com.hazelcast.config.NearCacheConfig}.
     */
    virtual void do_eviction_if_required() { assert(0); }

    /**
     * Does eviction as specified configuration {@link
     * com.hazelcast.config.EvictionConfig} in {@link
     * com.hazelcast.config.NearCacheConfig} regardless from the max-size
     * policy.
     */
    virtual void do_eviction() { assert(0); }

    /**
     * Persists the key set of the Near Cache.
     */
    virtual void store_keys() { assert(0); }
};
} // namespace impl
} // namespace nearcache
} // namespace internal
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
