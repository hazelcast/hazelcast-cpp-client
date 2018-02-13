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
#ifndef HAZELCAST_CLIENT_INTERNAL_EVICTION_IMPL_STRATEGY_ABSTRACTEVICTIONSTRATEGY_H_
#define HAZELCAST_CLIENT_INTERNAL_EVICTION_IMPL_STRATEGY_ABSTRACTEVICTIONSTRATEGY_H_

#include <assert.h>

#include "hazelcast/client/internal/eviction/EvictionStrategy.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyEvaluator.h"
#include "hazelcast/client/internal/eviction/EvictionChecker.h"
#include "hazelcast/client/internal/eviction/EvictionListener.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                namespace impl {
                    namespace strategy {
                        /**
                         * Base class for eviction implementations to evict {@link EvictableStore} implementations as specified
                         * {@link EvictionPolicyEvaluator}.
                         * @param S implements the EvictableStore<A, E>
                         */
                        template<typename MAPKEY, typename MAPVALUE, typename A, typename E, typename S>
                        class AbstractEvictionStrategy : public EvictionStrategy<MAPKEY, MAPVALUE, A, E, S> {
                        public:
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
                            //@Override
                            int evict(S *evictableStore, EvictionPolicyEvaluator<MAPKEY, MAPVALUE, A, E> *evictionPolicyEvaluator,
                                      EvictionChecker *evictionChecker, EvictionListener<A, E> *evictionListener) {
                                if (evictionChecker != NULL) {
                                    if (evictionChecker->isEvictionRequired()) {
                                        return evictInternal(evictableStore, evictionPolicyEvaluator, evictionListener);
                                    } else {
                                        return 0;
                                    }
                                } else {
                                    return evictInternal(evictableStore, evictionPolicyEvaluator, evictionListener);
                                }
                            }

                            /**
                             * Does eviction internally.
                             *
                             * @param evictableStore            Store that holds {@link Evictable} entries
                             * @param evictionPolicyEvaluator   {@link EvictionPolicyEvaluator} to evaluate
                             *                                  {@link com.hazelcast.config.EvictionPolicy} on entries
                             * @param evictionListener          {@link EvictionListener} to listen evicted entries
                             *
                             * @return evicted entry count
                             */
                        protected:
                            virtual int evictInternal(S *evictableStore,
                                                      EvictionPolicyEvaluator<MAPKEY, MAPVALUE, A, E> *evictionPolicyEvaluator,
                                                      EvictionListener<A, E> *evictionListener) {
                                assert(0);
                                return 0;
                            }
                        };

                    }
                }
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_EVICTION_IMPL_STRATEGY_ABSTRACTEVICTIONSTRATEGY_H_ */
