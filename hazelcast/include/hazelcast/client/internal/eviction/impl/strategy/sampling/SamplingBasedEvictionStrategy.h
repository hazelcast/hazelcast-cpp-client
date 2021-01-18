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

#include <vector>
#include <memory>

#include "hazelcast/client/internal/eviction/EvictableStore.h"
#include "hazelcast/client/internal/eviction/EvictionCandidate.h"
#include "hazelcast/client/internal/eviction/impl/strategy/AbstractEvictionStrategy.h"

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
                        namespace sampling {
                            /**
                             * Sampling based {@link EvictionStrategy} implementation.
                             * This strategy select sample {@link Evictable} entries from {@link SampleableEvictableStore}.
                             */
                            template<typename MAPKEY, typename MAPVALUE, typename A, typename E, typename S>
                            class SamplingBasedEvictionStrategy
                                    : public AbstractEvictionStrategy<MAPKEY, MAPVALUE, A, E, S> {
                            protected:
                                /**
                                 * Processes sampling based eviction logic on {@link SampleableEvictableStore}.
                                 *
                                 * @param sampleableEvictableStore  {@link SampleableEvictableStore} that holds {@link Evictable} entries
                                 * @param evictionPolicyEvaluator   {@link EvictionPolicyEvaluator} to evaluate
                                 * @param evictionListener          {@link EvictionListener} to listen evicted entries
                                 *
                                 * @return evicted entry count
                                 */
                                int evict_internal(S *sampleable_evictable_store,
                                                  EvictionPolicyEvaluator<MAPKEY, MAPVALUE, A, E> *eviction_policy_evaluator,
                                                  EvictionListener<A, E> *eviction_listener) override {
                                    std::unique_ptr<util::Iterable<EvictionCandidate<MAPKEY, MAPVALUE, A, E> > > samples = sampleable_evictable_store->sample(SAMPLE_COUNT);
                                    std::unique_ptr<std::vector<std::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E> > > > evictionCandidates =
                                            eviction_policy_evaluator->evaluate(*samples);
                                    return sampleable_evictable_store->evict(evictionCandidates.get(), eviction_listener);
                                }
                            private:
                                static const int SAMPLE_COUNT = 15;
                            };
                        }
                    }
                }
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


