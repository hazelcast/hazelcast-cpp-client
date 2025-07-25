/*
 * Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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

#include <stdint.h>

#include "hazelcast/client/internal/eviction/EvictionPolicyComparator.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace eviction {
namespace impl {
namespace comparator {
/**
 * {@link com.hazelcast.config.EvictionPolicy#LFU} policy based {@link
 * EvictionPolicyComparator}.
 */
template<typename A, typename E>
class LFUEvictionPolicyComparator : public EvictionPolicyComparator<A, E>
{
public:
    int compare(const EvictableEntryView<A, E>* e1,
                const EvictableEntryView<A, E>* e2) const override
    {
        int64_t hits1 = e1->get_access_hit();
        int64_t hits2 = e2->get_access_hit();
        if (hits2 < hits1) {
            return EvictionPolicyComparator<A, E>::
              SECOND_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED;
        } else if (hits1 < hits2) {
            return EvictionPolicyComparator<A, E>::
              FIRST_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED;
        } else {
            int64_t creationTime1 = e1->get_creation_time();
            int64_t creationTime2 = e2->get_creation_time();
            // if hits are same, we select the oldest entry to evict
            if (creationTime2 < creationTime1) {
                return EvictionPolicyComparator<A, E>::
                  SECOND_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED;
            } else if (creationTime2 > creationTime1) {
                return EvictionPolicyComparator<A, E>::
                  FIRST_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED;
            } else {
                return EvictionPolicyComparator<A, E>::
                  BOTH_OF_ENTRIES_HAVE_SAME_PRIORITY_TO_BE_EVICTED;
            }
        }
    }
};
} // namespace comparator
} // namespace impl
} // namespace eviction
} // namespace internal
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
