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
#include <stdint.h>

#include "hazelcast/client/internal/eviction/EvictionPolicyComparator.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                namespace impl {
                    namespace comparator {
                        /**
                         * {@link com.hazelcast.config.EvictionPolicy#LRU} policy based {@link EvictionPolicyComparator}.
                         */
                        template<typename A, typename E>
                        class LRUEvictionPolicyComparator : public EvictionPolicyComparator<A, E> {
                        public:
                            //@Override
                            int compare(const EvictableEntryView<A, E> *e1, const EvictableEntryView<A, E> *e2) const {
                                int64_t accessTime1 = e1->getLastAccessTime();
                                int64_t accessTime2 = e2->getLastAccessTime();
                                if (accessTime2 < accessTime1) {
                                    return EvictionPolicyComparator<A, E>::SECOND_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED;
                                } else if (accessTime1 < accessTime2) {
                                    return EvictionPolicyComparator<A, E>::FIRST_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED;
                                } else {
                                    int64_t creationTime1 = e1->getCreationTime();
                                    int64_t creationTime2 = e2->getCreationTime();
                                    // if access times are same, we select the oldest entry to evict
                                    if (creationTime2 < creationTime1) {
                                        return EvictionPolicyComparator<A, E>::SECOND_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED;
                                    } else if (creationTime2 > creationTime1) {
                                        return EvictionPolicyComparator<A, E>::FIRST_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED;
                                    } else {
                                        return EvictionPolicyComparator<A, E>::BOTH_OF_ENTRIES_HAVE_SAME_PRIORITY_TO_BE_EVICTED;
                                    }
                                }
                            }
                        };
                    }
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

