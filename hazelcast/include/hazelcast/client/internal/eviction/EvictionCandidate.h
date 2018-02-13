/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONCANDIDATE_H_
#define HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONCANDIDATE_H_

#include <assert.h>
#include <boost/shared_ptr.hpp>

#include "hazelcast/client/internal/eviction/EvictableEntryView.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                /**
                 * Interface for entries, records or whatever that can be evictable via its accessor (key or id).
                 *
                 * @param <A> Type of the accessor
                 * @param <E> Type of the {@link Evictable} value
                 */
                template <typename MAPKEY, typename MAPVALUE, typename A, typename E>
                class EvictionCandidate : public EvictableEntryView<MAPKEY, MAPVALUE> {
                public:
                    /**
                     * The accessor (key or id) of {@link Evictable} entry or record or whatever.
                     *
                     * @return the accessor (key or id) of {@link Evictable} entry or record or whatever
                     */
                    virtual boost::shared_ptr<A> getAccessor() const {
                        assert(0);
                        return boost::shared_ptr<A>();
                    }

                    /**
                     * The value of {@link Evictable} entry or record or whatever.
                     *
                     * @return the value of {@link Evictable} entry or record or whatever
                     */
                    virtual boost::shared_ptr<E> getEvictable() const {
                        assert(0);
                        return boost::shared_ptr<E>();
                    }
                };
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONCANDIDATE_H_ */
