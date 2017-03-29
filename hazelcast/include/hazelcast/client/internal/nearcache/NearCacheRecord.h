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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_NEARCACHERECORD_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_NEARCACHERECORD_H_

#include <assert.h>
#include <boost/shared_ptr.hpp>

#include "hazelcast/client/internal/eviction/Evictable.h"
#include "hazelcast/client/internal/eviction/Expirable.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
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
                template <typename V>
                class NearCacheRecord : public eviction::Expirable, public eviction::Evictable<V> {
                public:
                    static const int64_t TIME_NOT_SET = -1;

                    virtual ~NearCacheRecord() { }

                    /**
                     * Sets the value of this {@link NearCacheRecord}.
                     *
                     * @param value the value for this {@link NearCacheRecord}
                     */
                    virtual void setValue(const boost::shared_ptr<V> &value) {
                        assert(0);
                    }

                    /**
                     * Sets the creation time of this {@link Evictable} in milliseconds.
                     *
                     * @param time the creation time for this {@link Evictable} in milliseconds
                     */
                    virtual void setCreationTime(int64_t time) = 0;

                    /**
                     * Sets the access time of this {@link Evictable} in milliseconds.
                     *
                     * @param time the latest access time of this {@link Evictable} in milliseconds
                     */
                    virtual void setAccessTime(int64_t time) = 0;

                    /**
                     * Sets the access hit count of this {@link Evictable}.
                     *
                     * @param hit the access hit count for this {@link Evictable}
                     */
                    virtual void setAccessHit(int32_t hit) = 0;

                    /**
                     * Increases the access hit count of this {@link Evictable} by {@code 1}.
                     */
                    virtual void incrementAccessHit() = 0;

                    /**
                     * Resets the access hit count of this {@link Evictable} to {@code 0}.
                     */
                    virtual void resetAccessHit() = 0;

                    /**
                     * Checks whether the maximum idle time is passed with respect to the provided time
                     * without any access during this time period as {@code maxIdleSeconds}.
                     *
                     * @param maxIdleMilliSeconds maximum idle time in milliseconds
                     * @param now                 current time in milliseconds
                     * @return {@code true} if exceeds max idle seconds, otherwise {@code false}
                     */
                    virtual bool isIdleAt(int64_t maxIdleMilliSeconds, int64_t now) = 0;

                    /**
                     * @return last known invalidation sequence at time of this records' creation
                     */
                    virtual int64_t getInvalidationSequence() const = 0;

                    /**
                     * @param sequence last known invalidation sequence at time of this records' creation
                     */
                    virtual void setInvalidationSequence(int64_t sequence) = 0;

                    /**
                     * @param uuid last known uuid of invalidation source at time of this records' creation
                     */
                    virtual void setUuid(const boost::shared_ptr<util::UUID> &uuid) = 0;

                    /**
                     * @return {@code true} if supplied uuid equals existing one, otherwise and when one of supplied
                     * or existing is null returns {@code false}
                     */
                    virtual bool hasSameUuid(const boost::shared_ptr<util::UUID> &thatUuid) const = 0;
                };
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_NEARCACHERECORD_H_ */
