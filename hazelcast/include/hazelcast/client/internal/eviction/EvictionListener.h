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

#include <assert.h>
#include <memory>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace eviction {
/**
 * Interface for entries, records or whatever that can be evicted via its
 * accessor (key or id).
 *
 * @param <A> Type of the accessor
 * @param <E> Type of the {@link Evictable} value
 */
template<typename A, typename E>
class EvictionListener
{
public:
    virtual ~EvictionListener() = default;

    /**
     * Empty {@link} EvictionListener
     */
    static const std::shared_ptr<EvictionListener<A, E>> NO_LISTENER;

    /**
     * Called when an {@link Evictable} entry is evicted.
     *
     * @param evictedEntryAccessor Accessor of the {@link Evictable} entry that
     * is evicted.
     * @param evictedEntry         {@link Evictable} entry that is evicted.
     * @param wasExpired           {@code true} if the entry was evicted due to
     * expiration, {@code false} otherwise
     */
    virtual void on_evict(const std::shared_ptr<A>& evicted_entry_accessor,
                          const std::shared_ptr<E>& evicted_entry,
                          bool was_expired)
    {
        assert(0);
    }
};
} // namespace eviction
} // namespace internal
} // namespace client
}; // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
