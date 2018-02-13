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
#ifndef HAZELCAST_CLIENT_INTERNAL_EVICTION_IMPL_EVALUATOR_DEFAULTEVICTIONPOLICYEVALUATOR_H_
#define HAZELCAST_CLIENT_INTERNAL_EVICTION_IMPL_EVALUATOR_DEFAULTEVICTIONPOLICYEVALUATOR_H_

#include <boost/shared_ptr.hpp>

#include "hazelcast/util/Util.h"
#include "hazelcast/client/internal/eviction/Evictable.h"
#include "hazelcast/client/internal/eviction/Expirable.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyEvaluator.h"
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
                namespace impl {
                    namespace evaluator {
                        /**
                         * Default {@link EvictionPolicyEvaluator} implementation.
                         *
                         * @param <A> Type of the accessor (id) of the {@link com.hazelcast.internal.eviction.EvictionCandidate}
                         * @param <E> Type of the {@link com.hazelcast.internal.eviction.Evictable} value of
                         *            {@link com.hazelcast.internal.eviction.EvictionCandidate}
                         */
                        template<typename MAPKEY, typename MAPVALUE, typename A, typename E>
                        class DefaultEvictionPolicyEvaluator : public EvictionPolicyEvaluator<MAPKEY, MAPVALUE, A, E> {
                        public:
                            DefaultEvictionPolicyEvaluator(const boost::shared_ptr<EvictionPolicyComparator<MAPKEY, MAPVALUE> > &comparator)
                                    : evictionPolicyComparator(comparator) {
                            }

                            //@Override
                            const boost::shared_ptr<EvictionPolicyComparator<MAPKEY, MAPVALUE> > getEvictionPolicyComparator() const {
                                return evictionPolicyComparator;
                            }

                            /**
                             * The evaluate method implements the {@link com.hazelcast.config.EvictionPolicy} rule
                             * on the given input set of candidates.
                             *
                             * @param evictionCandidates Multiple {@link com.hazelcast.internal.eviction.EvictionCandidate} to be evicted
                             *
                             * @return multiple {@link com.hazelcast.internal.eviction.EvictionCandidate} these are available to be evicted
                             */
                            //@Override
                            std::auto_ptr<std::vector<boost::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E> > > > evaluate(
                                    util::Iterable<EvictionCandidate<MAPKEY, MAPVALUE, A, E> > &evictionCandidates) const {
                                boost::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E> > selectedEvictionCandidate;
                                int64_t now = util::currentTimeMillis();
                                util::Iterator<EvictionCandidate<MAPKEY, MAPVALUE, A, E> > *iterator = evictionCandidates.iterator();
                                while (iterator->hasNext()) {
                                    boost::shared_ptr<EvictionCandidate<MAPKEY, MAPVALUE, A, E> > currentEvictionCandidate = iterator->next();
                                    if (selectedEvictionCandidate.get() == NULL) {
                                        selectedEvictionCandidate = currentEvictionCandidate;
                                    } else {
                                        boost::shared_ptr<E> evictable = currentEvictionCandidate->getEvictable();

                                        if (isExpired(now, evictable.get())) {
                                            return returnEvictionCandidate(currentEvictionCandidate);
                                        }

                                        int comparisonResult = evictionPolicyComparator->compare(
                                                selectedEvictionCandidate.get(), currentEvictionCandidate.get());
                                        if (comparisonResult == EvictionPolicyComparator<A, E>::SECOND_ENTRY_HAS_HIGHER_PRIORITY_TO_BE_EVICTED) {
                                            selectedEvictionCandidate = currentEvictionCandidate;
                                        }
                                    }
                                }
                                return returnEvictionCandidate(selectedEvictionCandidate);
                            }

                        private:
                            std::auto_ptr<std::vector<boost::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E> > > > returnEvictionCandidate(
                                    const boost::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E> > &evictionCandidate) const {
                                if (evictionCandidate.get() == NULL) {
                                    return std::auto_ptr<std::vector<boost::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E> > > >();
                                } else {
                                    std::auto_ptr<std::vector<boost::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E> > > > result(
                                            new std::vector<boost::shared_ptr<eviction::EvictionCandidate<MAPKEY, MAPVALUE, A, E> > >());
                                    result->push_back(evictionCandidate);
                                    return result;
                                }
                            }

                            template <typename V>
                            bool isExpired(int64_t now, const Evictable<V> *evictable) const {
                                bool expired = false;
                                if (evictable != NULL) {
                                    // If evictable is also an expirable
                                    const Expirable *expirable = dynamic_cast<const Expirable *>(evictable);
                                    if (expirable != NULL) {
                                        // If there is an expired candidate, let's evict that one immediately
                                        expired = (const_cast<Expirable *>(expirable))->isExpiredAt(now);
                                    }
                                }
                                return expired;
                            }

                            const boost::shared_ptr<EvictionPolicyComparator<MAPKEY, MAPVALUE> > evictionPolicyComparator;
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

#endif /* HAZELCAST_CLIENT_INTERNAL_EVICTION_IMPL_EVALUATOR_DEFAULTEVICTIONPOLICYEVALUATOR_H_ */
