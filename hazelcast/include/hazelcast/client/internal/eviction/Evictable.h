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
#ifndef HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTABLE_H_
#define HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTABLE_H_

#include <assert.h>
#include "hazelcast/util/SharedPtr.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                /**
                 * Interface for entries, records or whatever that can be evicted.
                 */
                template <typename V>
                class Evictable {
                public:
                    /**
                     * Gets the creation time of this {@link Evictable} in milliseconds.
                     *
                     * @return the creation time of this {@link Evictable} in milliseconds
                     */
                    virtual int64_t getCreationTime() const = 0;

                    /**
                     * Gets the latest access time difference of this {@link Evictable} in milliseconds.
                     *
                     * @return the latest access time of this {@link Evictable} in milliseconds
                     */
                    virtual int64_t getLastAccessTime() = 0;

                    /**
                     * Gets the access hit count of this {@link Evictable}.
                     *
                     * @return the access hit count of this {@link Evictable}
                     */
                    virtual int32_t getAccessHit() = 0;

                    /**
                     * Gets the value of this {@link Evictable}.
                     *
                     * @return the value of this {@link Evictable}
                     */
                    virtual hazelcast::util::SharedPtr<V> getValue() const {
                        assert(0);
                        return hazelcast::util::SharedPtr<V>();
                    }
                };
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTABLE_H_ */
