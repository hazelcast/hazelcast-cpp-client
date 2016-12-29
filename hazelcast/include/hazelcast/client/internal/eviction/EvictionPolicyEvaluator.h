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
#ifndef HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONPOLICYEVALUATOR_H_
#define HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONPOLICYEVALUATOR_H_

#include <assert.h>
#include <vector>
#include <memory>
#include <boost/shared_ptr.hpp>

#include "hazelcast/client/internal/eviction/EvictionPolicyComparator.h"
#include "hazelcast/client/internal/eviction/EvictionCandidate.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                /**
                 * Interface for evaluation implementations of {@link com.hazelcast.config.EvictionPolicy}.
                 *
                 * @param <A> Type of the accessor (id) of the {@link com.hazelcast.internal.eviction.EvictionCandidate}
                 * @param <E> Type of the {@link com.hazelcast.internal.eviction.Evictable} value of
                 *            {@link com.hazelcast.internal.eviction.EvictionCandidate}
                 */
                template<typename MAPKEY, typename MAPVALUE, typename A, typename E>
                class EvictionPolicyEvaluator {
                public:
                    virtual ~EvictionPolicyEvaluator() {
                    }

                    /**
                     * Gets the underlying {@link EvictionPolicyComparator}.
                     *
                     * @return the underlying {@link EvictionPolicyComparator}
                     */
                    virtual const boost::shared_ptr<EvictionPolicyComparator<MAPKEY, MAPVALUE> > getEvictionPolicyComparator() const {
                        assert(0);
                        return boost::shared_ptr<EvictionPolicyComparator<MAPKEY, MAPVALUE> >();
                    }

                    /**
                     * The evaluate method implements the actual policy rules and is called on every eviction to select one or
                     * more candidates to be evicted from the given input set of candidates.
                     * The selection algorithm should execute in a constant time to deliver a predictable timing results of
                     * the eviction system.
                     *
                     * @param evictionCandidates Multiple {@link EvictionCandidate} to be evicted
                     *
                     * @return multiple {@link EvictionCandidate}s that are available to be evicted
                     */
                    virtual std::auto_ptr<std::vector<boost::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E> > > > evaluate(
                            const std::vector<boost::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E> > > &evictionCandidates) const {
                        assert(0);
                        return std::auto_ptr<std::vector<boost::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E> > > >();
                    };
                };
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONPOLICYEVALUATOR_H_ */
