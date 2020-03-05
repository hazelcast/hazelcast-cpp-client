/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONLISTENER_H_
#define HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONLISTENER_H_

#include <assert.h>
#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                /**
                 * Interface for entries, records or whatever that can be evicted via its accessor (key or id).
                 *
                 * @param <A> Type of the accessor
                 * @param <E> Type of the {@link Evictable} value
                 */
                template <typename A, typename E>
                class EvictionListener {
                public:
                    virtual ~EvictionListener() { }

                    /**
                     * Empty {@link} EvictionListener
                     */
                    static const std::shared_ptr<EvictionListener<A, E> > NO_LISTENER;

                    /**
                     * Called when an {@link Evictable} entry is evicted.
                     *
                     * @param evictedEntryAccessor Accessor of the {@link Evictable} entry that is evicted.
                     * @param evictedEntry         {@link Evictable} entry that is evicted.
                     * @param wasExpired           {@code true} if the entry was evicted due to expiration, {@code false} otherwise
                     */
                    virtual void onEvict(const std::shared_ptr<A> &evictedEntryAccessor, const std::shared_ptr<E> &evictedEntry, bool wasExpired) {
                        assert(0);
                    }
                };
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONLISTENER_H_ */
