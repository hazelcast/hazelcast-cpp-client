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

#include <memory>

#include "hazelcast/util/Util.h"
#include "hazelcast/client/internal/eviction/Evictable.h"
#include "hazelcast/client/internal/eviction/Expirable.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyEvaluator.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyComparator.h"
#include "hazelcast/client/internal/eviction/EvictionCandidate.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace eviction {
namespace impl {
namespace evaluator {
/**
 * Default {@link EvictionPolicyEvaluator} implementation.
 *
 * @param <A> Type of the accessor (id) of the {@link
 * com.hazelcast.internal.eviction.EvictionCandidate}
 * @param <E> Type of the {@link com.hazelcast.internal.eviction.Evictable} value of
 *            {@link com.hazelcast.internal.eviction.EvictionCandidate}
 */
template<typename MAPKEY, typename MAPVALUE, typename A, typename E>
class DefaultEvictionPolicyEvaluator : public EvictionPolicyEvaluator<MAPKEY, MAPVALUE, A, E>
{
public:
    DefaultEvictionPolicyEvaluator(
      const std::shared_ptr<EvictionPolicyComparator<MAPKEY, MAPVALUE>>& comparator)
      : eviction_policy_comparator_(comparator)
    {}

    const std::shared_ptr<EvictionPolicyComparator<MAPKEY, MAPVALUE>>
    get_eviction_policy_comparator() const override
    {
        return eviction_policy_comparator_;
    }

    /**
     * The evaluate method implements the {@link com.hazelcast.config.EvictionPolicy} rule
     * on the given input set of candidates.
     *
     * @param evictionCandidates Multiple {@link com.hazelcast.internal.eviction.EvictionCandidate}
     * to be evicted
     *
     * @return multiple {@link com.hazelcast.internal.eviction.EvictionCandidate} these are
     * available to be evicted
     */
    std::unique_ptr<
      std::vector<std::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E>>>>
    evaluate(
      util::Iterable<EvictionCandidate<MAPKEY, MAPVALUE, A, E>>& eviction_candidates) const override
    {
        std::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E>>
          selectedEvictionCandidate;
        int64_t now = util::current_time_millis();
        util::Iterator<EvictionCandidate<MAPKEY, MAPVALUE, A, E>>* iterator =
          eviction_candidates.iterator();
        while (iterator->has_next()) {
            std::shared_ptr<EvictionCandidate<MAPKEY, MAPVALUE, A, E>> currentEvictionCandidate =
              iterator->next();
            if (selectedEvictionCandidate.get() == NULL) {
                selectedEvictionCandidate = currentEvictionCandidate;
            } else {
                std::shared_ptr<E> evictable = currentEvictionCandidate->get_evictable();

                if (is_expired(now, evictable.get())) {
                    return return_eviction_candidate(currentEvictionCandidate);
                }

                int comparisonResult = eviction_policy_comparator_->compare(
                  selectedEvictionCandidate.get(), currentEvictionCandidate.get());
                if (comparisonResult ==
                    EvictionPolicyComparator<A,
                                             E>::SECOND_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED) {
                    selectedEvictionCandidate = currentEvictionCandidate;
                }
            }
        }
        return return_eviction_candidate(selectedEvictionCandidate);
    }

private:
    std::unique_ptr<
      std::vector<std::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E>>>>
    return_eviction_candidate(
      const std::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E>>&
        eviction_candidate) const
    {
        if (eviction_candidate.get() == NULL) {
            return std::unique_ptr<
              std::vector<std::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E>>>>();
        } else {
            std::unique_ptr<
              std::vector<std::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E>>>>
              result(new std::vector<
                     std::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E>>>());
            result->push_back(eviction_candidate);
            return result;
        }
    }

    template<typename V>
    bool is_expired(int64_t now, const Evictable<V>* evictable) const
    {
        bool expired = false;
        if (evictable != NULL) {
            // If evictable is also an expirable
            const Expirable* expirable = dynamic_cast<const Expirable*>(evictable);
            if (expirable != NULL) {
                // If there is an expired candidate, let's evict that one immediately
                expired = expirable->is_expired_at(now);
            }
        }
        return expired;
    }

    const std::shared_ptr<EvictionPolicyComparator<MAPKEY, MAPVALUE>> eviction_policy_comparator_;
};
} // namespace evaluator
} // namespace impl
} // namespace eviction
} // namespace internal
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
