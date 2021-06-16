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
#include <vector>
#include <memory>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace eviction {
template<typename K, typename V>
class EvictionPolicyComparator;

template<typename MAPKEY, typename MAPVALUE, typename A, typename E>
class EvictionCandidate;

/**
 * Interface for evaluation implementations of {@link com.hazelcast.config.EvictionPolicy}.
 *
 * @param <A> Type of the accessor (id) of the {@link
 * com.hazelcast.internal.eviction.EvictionCandidate}
 * @param <E> Type of the {@link com.hazelcast.internal.eviction.Evictable} value of
 *            {@link com.hazelcast.internal.eviction.EvictionCandidate}
 */
template<typename MAPKEY, typename MAPVALUE, typename A, typename E>
class EvictionPolicyEvaluator
{
public:
    virtual ~EvictionPolicyEvaluator() = default;

    /**
     * Gets the underlying {@link EvictionPolicyComparator}.
     *
     * @return the underlying {@link EvictionPolicyComparator}
     */
    virtual const std::shared_ptr<EvictionPolicyComparator<MAPKEY, MAPVALUE>>
    get_eviction_policy_comparator() const
    {
        assert(0);
        return std::shared_ptr<EvictionPolicyComparator<MAPKEY, MAPVALUE>>();
    }

    /**
     * The evaluate method implements the actual policy rules and is called on every eviction to
     * select one or more candidates to be evicted from the given input set of candidates. The
     * selection algorithm should execute in a constant time to deliver a predictable timing results
     * of the eviction system.
     *
     * @param evictionCandidates Multiple {@link EvictionCandidate} to be evicted
     *
     * @return multiple {@link EvictionCandidate}s that are available to be evicted
     */
    virtual std::unique_ptr<
      std::vector<std::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E>>>>
    evaluate(util::Iterable<EvictionCandidate<MAPKEY, MAPVALUE, A, E>>& eviction_candidates) const
    {
        assert(0);
        return std::unique_ptr<
          std::vector<std::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E>>>>();
    };
};
} // namespace eviction
} // namespace internal
} // namespace client
}; // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
