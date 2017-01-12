/*
 * Copyright (c) 2008-2016, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONSTRATEGY_H_
#define HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONSTRATEGY_H_

#include <assert.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                template<typename MAPKEY, typename MAPVALUE, typename A, typename E>
                class EvictionPolicyEvaluator;

                template <typename A, typename E>
                class EvictionListener;

                class EvictionChecker;

                /**
                 * Interface for eviction implementations to evict {@link EvictableStore} implementations as specified
                 * {@link EvictionPolicyEvaluator}.
                 *
                 * @param <A> Type of the accessor (id) of the {@link com.hazelcast.internal.eviction.EvictionCandidate}
                 * @param <E> Type of the {@link com.hazelcast.internal.eviction.Evictable} value of
                 *            {@link com.hazelcast.internal.eviction.EvictionCandidate}
                 * @param <S> Type of the {@link com.hazelcast.internal.eviction.EvictableStore}
                 */
                template <typename MAPKEY, typename MAPVALUE,typename A, typename E, typename S>
                class EvictionStrategy {
                public:
                    virtual ~EvictionStrategy() {
                    }

                    /**
                     * Does eviction if eviction is required by given {@link EvictionChecker}.
                     *
                     * @param evictableStore            Store that holds {@link Evictable} entries
                     * @param evictionPolicyEvaluator   {@link EvictionPolicyEvaluator} to evaluate
                     *                                  {@link com.hazelcast.config.EvictionPolicy} on entries
                     * @param evictionChecker           {@link EvictionChecker} to make a decision about if eviction is
                     *                                  required or not. If you want evict anyway,
                     *                                  you can use {@link EvictionChecker#EVICT_ALWAYS}
                     * @param evictionListener          {@link EvictionListener} to listen evicted entries
                     *
                     * @return evicted entry count
                     */
                    virtual int evict(S *evictableStore,
                              EvictionPolicyEvaluator<MAPKEY, MAPVALUE, A, E> *evictionPolicyEvaluator,
                              EvictionChecker *evictionChecker,
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

#endif /* HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONSTRATEGY_H_ */
