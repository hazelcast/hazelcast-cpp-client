/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include <unordered_map>

#include "hazelcast/client/config/eviction_strategy_type.h"
#include "hazelcast/client/internal/eviction/EvictionStrategy.h"
#include "hazelcast/client/internal/eviction/eviction_configuration.h"
#include "hazelcast/client/internal/eviction/impl/strategy/sampling/SamplingBasedEvictionStrategy.h"
#include "hazelcast/client/config/eviction_strategy_type.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace eviction {
/**
 * Provider to get any kind ({@link EvictionStrategyType}) of {@link
 * EvictionStrategy}.
 */
template<typename MAPKEY, typename MAPVALUE, typename A, typename E, typename S>
class EvictionStrategyProvider
{
public:
    /**
     * Gets the {@link EvictionStrategy} implementation specified with
     * <code>evictionStrategyType</code>.
     *
     * @param evictionConfig {@link eviction_configuration} for the requested
     * {@link EvictionStrategy} implementation
     * @return the requested {@link EvictionStrategy} implementation
     */
    static std::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S>>
    get_eviction_strategy(
      const client::config::eviction_config& eviction_config)
    {
        client::config::eviction_strategy_type evictionStrategyType =
          eviction_config.get_eviction_strategy_type();

        return EvictionStrategyProvider<MAPKEY, MAPVALUE, A, E, S>::
          EVICTION_STRATEGY_MAP[evictionStrategyType];

        // TODO "evictionStrategyFactory" can be handled here from a single
        // point for user defined custom implementations. So "EvictionStrategy"
        // implementation can be taken from user defined factory.
    }

    /**
     * Gets the default {@link EvictionStrategy} implementation.
     *
     * @return the default {@link EvictionStrategy} implementation
     */
    static std::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S>>&
    get_default_eviction_strategy()
    {
        return EvictionStrategyProvider<MAPKEY, MAPVALUE, A, E, S>::
          EVICTION_STRATEGY_MAP
            [client::config::eviction_strategy_type::DEFAULT_EVICTION_STRATEGY];
    }

    static std::unordered_map<
      client::config::eviction_strategy_type,
      std::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S>>>
    init()
    {
        std::unordered_map<
          client::config::eviction_strategy_type,
          std::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S>>>
          map;
        map[client::config::eviction_strategy_type::SAMPLING_BASED_EVICTION] =
          std::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S>>(
            new impl::strategy::sampling::
              SamplingBasedEvictionStrategy<MAPKEY, MAPVALUE, A, E, S>());
        return map;
    }

private:
    static std::unordered_map<
      client::config::eviction_strategy_type,
      std::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S>>>
      EVICTION_STRATEGY_MAP;
};

template<typename MAPKEY, typename MAPVALUE, typename A, typename E, typename S>
std::unordered_map<client::config::eviction_strategy_type,
                   std::shared_ptr<EvictionStrategy<MAPKEY, MAPVALUE, A, E, S>>>
  EvictionStrategyProvider<MAPKEY, MAPVALUE, A, E, S>::EVICTION_STRATEGY_MAP =
    EvictionStrategyProvider<MAPKEY, MAPVALUE, A, E, S>::init();
} // namespace eviction
} // namespace internal
} // namespace client
}; // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
