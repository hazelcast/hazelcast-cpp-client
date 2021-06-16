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

#include <cassert>
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
 * Interface for entries, records or whatever that can be evicted.
 */
template<typename V>
class Evictable
{
public:
    virtual ~Evictable() = default;

    /**
     * Gets the creation time of this {@link Evictable} in milliseconds.
     *
     * @return the creation time of this {@link Evictable} in milliseconds
     */
    virtual int64_t get_creation_time() const = 0;

    /**
     * Gets the latest access time difference of this {@link Evictable} in milliseconds.
     *
     * @return the latest access time of this {@link Evictable} in milliseconds
     */
    virtual int64_t get_last_access_time() = 0;

    /**
     * Gets the access hit count of this {@link Evictable}.
     *
     * @return the access hit count of this {@link Evictable}
     */
    virtual int32_t get_access_hit() = 0;

    /**
     * Gets the value of this {@link Evictable}.
     *
     * @return the value of this {@link Evictable}
     */
    virtual std::shared_ptr<V> get_value() const
    {
        assert(0);
        return std::shared_ptr<V>();
    }
};
} // namespace eviction
} // namespace internal
} // namespace client
}; // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
