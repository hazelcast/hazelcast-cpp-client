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
#ifndef HAZELCAST_CLIENT_INTERNAL_EVICTION_IMPL_STRATEGY_SAMPLING_H_
#define HAZELCAST_CLIENT_INTERNAL_EVICTION_IMPL_STRATEGY_SAMPLING_H_

#include <assert.h>
#include <memory>
#include <stdint.h>

#include "hazelcast/client/internal/eviction/EvictableStore.h"
#include "hazelcast/util/Iterable.h"

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
                             * Interface for sampleable store implementations that holds {@link Evictable} entries to evict.
                             */
                            template <typename K, typename V, typename A, typename E>
                            class SampleableEvictableStore : public EvictableStore<K, V, A, E> {
                                /**
                                 * The sample method is used to sample a number of entries (defined by the samples parameter) from
                                 * the internal data structure. This method should be executed in a constant time to deliver predictable
                                 * timing results of the eviction system.
                                 *
                                 * @param sampleCount Entry count to be sampled
                                 *
                                 * @return Multiple {@link EvictionCandidate} to be evicted
                                 */
                                std::auto_ptr<util::Iterable<EvictionCandidate<K, V, A, E> > > sample(int32_t sampleCount) const {
                                    assert(0);
                                    return std::auto_ptr<util::Iterable<EvictionCandidate<K, V, A, E> > >();
                                }
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

#endif /* HAZELCAST_CLIENT_INTERNAL_EVICTION_IMPL_STRATEGY_SAMPLING_H_ */
