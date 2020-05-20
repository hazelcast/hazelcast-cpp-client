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

#include "hazelcast/util/Comparator.h"
#include "hazelcast/client/internal/eviction/EvictableEntryView.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                /**
                 * A kind of {@link Comparator} to be used while comparing
                 * entries to be evicted.
                 */
                template<typename K, typename V>
                class EvictionPolicyComparator : util::Comparator<EvictableEntryView<K, V> > {
                public:
                    virtual ~EvictionPolicyComparator() {
                    }

                    /**
                     * Integer constant for representing behaviour for giving higher priority to first entry to be evicted.
                     */
                    static const int FIRST_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED = -1;

                    /**
                     * Integer constant for representing behaviour for giving higher priority to second entry to be evicted.
                     */
                    static const int SECOND_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED = +1;

                    /**
                     * Integer constant for representing behaviour for giving same priority to both of entry to be evicted.
                     */
                    static const int BOTH_OF_ENTRIES_HAVE_SAME_PRIORITY_TO_BE_EVICTED = 0;

                    /**
                     * Compares the given {@link EvictableEntryView} instances and returns the result.
                     * The result should be one of the
                     * <ul>
                     *   <li>#FIRST_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED</li>
                     *   <li>#SECOND_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED</li>
                     *   <li>#BOTH_OF_ENTRIES_HAVE_SAME_PRIORITY_TO_BE_EVICTED</li>
                     * </ul>
                     *
                     * @param e1 the first {@link EvictableEntryView} instance to be compared
                     * @param e2 the second {@link EvictableEntryView} instance to be compared
                     *
                     * @return the result of comparison
                     */
                    //@Override
                    virtual int compare(const EvictableEntryView<K, V> *e1, const EvictableEntryView<K, V> *e2) const {
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

