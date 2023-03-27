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

#include <assert.h>
#include <memory>
#include <boost/uuid/uuid.hpp>

#include "hazelcast/client/internal/eviction/Evictable.h"
#include "hazelcast/client/internal/eviction/Expirable.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace util {
class UUID;
}

namespace client {
namespace internal {
namespace nearcache {
/**
 * An expirable and evictable data object which represents a Near Cache entry.
 * <p>
 * Record of {@link NearCacheRecordStore}.
 *
 * @param <V> the type of the value stored by this {@link NearCacheRecord}
 * @see com.hazelcast.internal.eviction.Expirable
 * @see com.hazelcast.internal.eviction.Evictable
 */
template<typename V>
class NearCacheRecord
  : public eviction::Expirable
  , public eviction::Evictable<V>
{
public:
    static const int TIME_NOT_SET = -1;

    ~NearCacheRecord() override = default;

    /**
     * Sets the value of this {@link NearCacheRecord}.
     *
     * @param value the value for this {@link NearCacheRecord}
     */
    virtual void set_value(const std::shared_ptr<V>& /* value */) { assert(0); }

    /**
     * Sets the creation time of this {@link Evictable} in milliseconds.
     *
     * @param time the creation time for this {@link Evictable} in milliseconds
     */
    virtual void set_creation_time(int64_t /* time */) = 0;

    /**
     * Sets the access time of this {@link Evictable} in milliseconds.
     *
     * @param time the latest access time of this {@link Evictable} in
     * milliseconds
     */
    virtual void set_access_time(int64_t /* time */) = 0;

    /**
     * Sets the access hit count of this {@link Evictable}.
     *
     * @param hit the access hit count for this {@link Evictable}
     */
    virtual void set_access_hit(int32_t /* hit */) = 0;

    /**
     * Increases the access hit count of this {@link Evictable} by {@code 1}.
     */
    virtual void increment_access_hit() = 0;

    /**
     * Resets the access hit count of this {@link Evictable} to {@code 0}.
     */
    virtual void reset_access_hit() = 0;

    /**
     * Checks whether the maximum idle time is passed with respect to the
     * provided time without any access during this time period as {@code
     * maxIdleSeconds}.
     *
     * @param maxIdleMilliSeconds maximum idle time in milliseconds
     * @param now                 current time in milliseconds
     * @return {@code true} if exceeds max idle seconds, otherwise {@code false}
     */
    virtual bool is_idle_at(int64_t /* max_idle_milli_seconds */, int64_t /* now */) = 0;

    /**
     * @return last known invalidation sequence at time of this records'
     * creation
     */
    virtual int64_t get_invalidation_sequence() const = 0;

    /**
     * @param sequence last known invalidation sequence at time of this records'
     * creation
     */
    virtual void set_invalidation_sequence(int64_t /* sequence */) = 0;

    /**
     * @param uuid last known uuid of invalidation source at time of this
     * records' creation
     */
    virtual void set_uuid(boost::uuids::uuid /* uuid */) = 0;

    /**
     * @return {@code true} if supplied uuid equals existing one, otherwise and
     * when one of supplied or existing is null returns {@code false}
     */
    virtual bool has_same_uuid(boost::uuids::uuid /* that_uuid */) const = 0;
};
} // namespace nearcache
} // namespace internal
} // namespace client
}; // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
