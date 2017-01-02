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
#ifndef HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONSTRATEGYPROVIDER_H_
#define HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONSTRATEGYPROVIDER_H_

#include <assert.h>
#include <map>

#include "hazelcast/client/internal/eviction/EvictionStrategyType.h"
#include "hazelcast/client/internal/eviction/EvictionStrategy.h"
#include "hazelcast/client/internal/eviction/EvictionConfiguration.h"
#include "hazelcast/client/internal/eviction/impl/strategy/sampling/SamplingBasedEvictionStrategy.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                /**
                 * Provider to get any kind ({@link EvictionStrategyType}) of {@link EvictionStrategy}.
                 */
                template<typename MAPKEY, typename MAPVALUE, typename A, typename E, typename S>
                class EvictionStrategyProvider {
                public:
                    /**
                     * Gets the {@link EvictionStrategy} implementation specified with <code>evictionStrategyType</code>.
                     *
                     * @param evictionConfig {@link EvictionConfiguration} for the requested {@link EvictionStrategy} implementation
                     * @return the requested {@link EvictionStrategy} implementation
                     */
                    static boost::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S> > getEvictionStrategy(
                            const boost::shared_ptr<EvictionConfiguration<MAPKEY, MAPVALUE> > &evictionConfig) {
                        if (evictionConfig.get() == NULL) {
                            return boost::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S> >();
                        }
                        EvictionStrategyType::Type evictionStrategyType = evictionConfig->getEvictionStrategyType();

                        return EvictionStrategyProvider<MAPKEY, MAPVALUE, A, E, S>::EVICTION_STRATEGY_MAP[evictionStrategyType];

                        // TODO "evictionStrategyFactory" can be handled here from a single point
                        // for user defined custom implementations.
                        // So "EvictionStrategy" implementation can be taken from user defined factory.
                    }

                    /**
                     * Gets the default {@link EvictionStrategy} implementation.
                     *
                     * @return the default {@link EvictionStrategy} implementation
                     */
                    static boost::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S> > &getDefaultEvictionStrategy() {
                        return EvictionStrategyProvider<MAPKEY, MAPVALUE, A, E, S>::EVICTION_STRATEGY_MAP[EvictionStrategyType::DEFAULT_EVICTION_STRATEGY];
                    }

                    static std::map<EvictionStrategyType::Type, boost::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S> > > init() {
                        std::map<EvictionStrategyType::Type, boost::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S> > > map;
                        map[EvictionStrategyType::SAMPLING_BASED_EVICTION] = boost::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S> >(new impl::strategy::sampling::SamplingBasedEvictionStrategy<MAPKEY, MAPVALUE, A, E, S>());
                        return map;
                    }

                private:
                    static std::map<EvictionStrategyType::Type, boost::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S> > > EVICTION_STRATEGY_MAP;
                };

                template<typename MAPKEY, typename MAPVALUE, typename A, typename E, typename S>
                std::map<EvictionStrategyType::Type, boost::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S> > > EvictionStrategyProvider<MAPKEY, MAPVALUE, A, E, S>::EVICTION_STRATEGY_MAP = EvictionStrategyProvider<MAPKEY, MAPVALUE, A, E, S>::init();
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONSTRATEGYPROVIDER_H_ */
