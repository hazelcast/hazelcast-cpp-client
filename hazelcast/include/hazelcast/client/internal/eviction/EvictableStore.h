/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include <vector>
#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                template<typename MAPKEY, typename MAPVALUE, typename A, typename E>
                class EvictionCandidate;

                template<typename A, typename E>
                class EvictionListener;

                /**
                 * Interface for store implementations that holds {@link Evictable} entries to evict.
                 *
                 * @param <A> Type of the accessor (id) of the {@link com.hazelcast.internal.eviction.EvictionCandidate}
                 * @param <E> Type of the {@link com.hazelcast.internal.eviction.Evictable} value of
                 *            {@link com.hazelcast.internal.eviction.EvictionCandidate}
                 */
                template<typename MAPKEY, typename MAPVALUE, typename A, typename E>
                class EvictableStore {
                public:
                    virtual ~EvictableStore() { }

                    /**
                     * The evict method is called by the {@link EvictionStrategy} to eventually evict, by the policy, selected
                     * candidates from the internal data structures.
                     *
                     * @param evictionCandidates Multiple {@link EvictionCandidate} to be evicted
                     * @param evictionListener   {@link EvictionListener} to listen evicted entries
                     *
                     * @return evicted entry count
                     */
                    virtual int evict(
                            std::vector<std::shared_ptr<EvictionCandidate<MAPKEY, MAPVALUE, A, E> > > *evictionCandidates,
                            EvictionListener<A, E> *evictionListener) {
                        assert(0);
                        return 0;
                    }
                };
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

