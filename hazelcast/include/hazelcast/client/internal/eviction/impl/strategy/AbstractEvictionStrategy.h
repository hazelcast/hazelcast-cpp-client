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

#include <assert.h>

#include "hazelcast/client/internal/eviction/EvictionStrategy.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyEvaluator.h"
#include "hazelcast/client/internal/eviction/EvictionChecker.h"
#include "hazelcast/client/internal/eviction/EvictionListener.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace eviction {
namespace impl {
namespace strategy {
/**
 * Base class for eviction implementations to evict {@link EvictableStore}
 * implementations as specified
 * {@link EvictionPolicyEvaluator}.
 * @param S implements the EvictableStore<A, E>
 */
template<typename MAPKEY, typename MAPVALUE, typename A, typename E, typename S>
class AbstractEvictionStrategy
  : public EvictionStrategy<MAPKEY, MAPVALUE, A, E, S>
{
public:
    /**
     * Does eviction if eviction is required by given {@link EvictionChecker}.
     *
     * @param evictableStore            Store that holds {@link Evictable}
     * entries
     * @param evictionPolicyEvaluator   {@link EvictionPolicyEvaluator} to
     * evaluate
     *                                  {@link
     * com.hazelcast.config.EvictionPolicy} on entries
     * @param evictionChecker           {@link EvictionChecker} to make a
     * decision about if eviction is required or not. If you want evict anyway,
     *                                  you can use {@link
     * EvictionChecker#EVICT_ALWAYS}
     * @param evictionListener          {@link EvictionListener} to listen
     * evicted entries
     *
     * @return evicted entry count
     */
    int evict(S* evictable_store,
              EvictionPolicyEvaluator<MAPKEY, MAPVALUE, A, E>*
                eviction_policy_evaluator,
              EvictionChecker* eviction_checker,
              EvictionListener<A, E>* eviction_listener) override
    {
        if (eviction_checker != NULL) {
            if (eviction_checker->is_eviction_required()) {
                return evict_internal(evictable_store,
                                      eviction_policy_evaluator,
                                      eviction_listener);
            } else {
                return 0;
            }
        } else {
            return evict_internal(
              evictable_store, eviction_policy_evaluator, eviction_listener);
        }
    }

    /**
     * Does eviction internally.
     *
     * @param evictableStore            Store that holds {@link Evictable}
     * entries
     * @param evictionPolicyEvaluator   {@link EvictionPolicyEvaluator} to
     * evaluate
     *                                  {@link
     * com.hazelcast.config.EvictionPolicy} on entries
     * @param evictionListener          {@link EvictionListener} to listen
     * evicted entries
     *
     * @return evicted entry count
     */
protected:
    virtual int evict_internal(S* evictable_store,
                               EvictionPolicyEvaluator<MAPKEY, MAPVALUE, A, E>*
                                 eviction_policy_evaluator,
                               EvictionListener<A, E>* eviction_listener)
    {
        assert(0);
        return 0;
    }
};

} // namespace strategy
} // namespace impl
} // namespace eviction
} // namespace internal
} // namespace client
}; // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
