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

#include <sstream>
#include <memory>

#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/client/internal/eviction/impl/comparator/LRUEvictionPolicyComparator.h"
#include "hazelcast/client/internal/eviction/impl/comparator/LFUEvictionPolicyComparator.h"
#include "hazelcast/client/internal/eviction/impl/comparator/RandomEvictionPolicyComparator.h"
#include "hazelcast/client/internal/eviction/eviction_configuration.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyEvaluator.h"
#include "hazelcast/client/internal/eviction/impl/evaluator/DefaultEvictionPolicyEvaluator.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                /**
                 * Provider to get any kind ({@link EvictionPolicyType}) of {@link EvictionPolicyEvaluator}.
                 */
                class EvictionPolicyEvaluatorProvider {
                public:
                    EvictionPolicyEvaluatorProvider() = delete;

                    /**
                     * Gets the {@link EvictionPolicyEvaluator} implementation specified with {@code evictionPolicy}.
                     *
                     * @param E is a type that extends Evictable
                     * @param evictionConfig {@link eviction_configuration} for requested {@link EvictionPolicyEvaluator} implementation
                     * @return the requested {@link EvictionPolicyEvaluator} implementation
                     */
                    template<typename MAPKEY, typename MAPVALUE, typename A, typename E>
                    static std::unique_ptr<EvictionPolicyEvaluator<MAPKEY, MAPVALUE, A, E> > get_eviction_policy_evaluator(
                            const client::config::eviction_config &eviction_config) {

                        std::shared_ptr<EvictionPolicyComparator<MAPKEY, MAPVALUE> > evictionPolicyComparator;

                        auto evictionPolicyType = eviction_config.get_eviction_policy();
                        evictionPolicyComparator = create_eviction_policy_comparator<MAPKEY, MAPVALUE>(evictionPolicyType);

                        return std::unique_ptr<EvictionPolicyEvaluator<MAPKEY, MAPVALUE, A, E> >(
                                new impl::evaluator::DefaultEvictionPolicyEvaluator<MAPKEY, MAPVALUE, A, E>(
                                        evictionPolicyComparator));
                    }

                private:
                    template<typename A, typename E>
                    static std::shared_ptr<EvictionPolicyComparator<A, E> > create_eviction_policy_comparator(
                            ::hazelcast::client::config::eviction_policy eviction_policy) {
                        switch (eviction_policy) {
                            case ::hazelcast::client::config::eviction_policy::LRU:
                                return std::shared_ptr<EvictionPolicyComparator<A, E> >(
                                        new impl::comparator::LRUEvictionPolicyComparator<A, E>());
                            case ::hazelcast::client::config::eviction_policy::LFU:
                                return std::shared_ptr<EvictionPolicyComparator<A, E> >(
                                        new impl::comparator::LFUEvictionPolicyComparator<A, E>());
                            case ::hazelcast::client::config::eviction_policy::RANDOM:
                                return std::shared_ptr<EvictionPolicyComparator<A, E> >(
                                        new impl::comparator::RandomEvictionPolicyComparator<A, E>());
                            case ::hazelcast::client::config::eviction_policy::NONE:
                                return std::shared_ptr<EvictionPolicyComparator<A, E> >();
                            default:
                                std::ostringstream out;
                                out << "Unsupported eviction policy type: " << (int) eviction_policy;
                                BOOST_THROW_EXCEPTION(exception::illegal_argument(out.str()));
                        }
                    }
                };

            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


