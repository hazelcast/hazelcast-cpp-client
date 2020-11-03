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

#include <string>
#include <stdint.h>
#include <memory>
#include <ostream>
#include <cassert>

#include "hazelcast/client/internal/eviction/EvictionPolicyComparator.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/internal/eviction/EvictionConfiguration.h"
#include "hazelcast/client/config/EvictionPolicy.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            /**
             * Configuration for eviction.
             * You can set a limit for number of entries or total memory cost of entries.
             */
            class HAZELCAST_API EvictionConfig {
            public:
                ~EvictionConfig()  = default;

                /**
                 * Maximum Size Policy
                 */
                enum MaxSizePolicy {
                    /**
                     * Policy based on maximum number of entries stored per data structure (map, cache etc)
                     */
                            ENTRY_COUNT
                    /* TODO,
                    *
                     * Policy based on maximum used native memory in megabytes per data structure (map, cache etc)
                     * on each Hazelcast instance

                            USED_NATIVE_MEMORY_SIZE,
                    *
                     * Policy based on maximum used native memory percentage per data structure (map, cache etc)
                     * on each Hazelcast instance

                            USED_NATIVE_MEMORY_PERCENTAGE,
                    *
                     * Policy based on minimum free native memory in megabytes per Hazelcast instance

                            FREE_NATIVE_MEMORY_SIZE,
                    *
                     * Policy based on minimum free native memory percentage per Hazelcast instance

                            FREE_NATIVE_MEMORY_PERCENTAGE*/
                };

                /**
                 * Default maximum entry count.
                 */
                static constexpr int32_t DEFAULT_MAX_ENTRY_COUNT = INT32_MAX;

                /**
                 * Default Max-Size Policy.
                 */
                static constexpr MaxSizePolicy DEFAULT_MAX_SIZE_POLICY = MaxSizePolicy::ENTRY_COUNT;

                /**
                 * Default Eviction Policy.
                 */
                static constexpr EvictionPolicy DEFAULT_EVICTION_POLICY = EvictionPolicy::LRU;

                EvictionConfig();

                int32_t getSize() const;

                EvictionConfig &setSize(int32_t size);

                MaxSizePolicy getMaximumSizePolicy() const;

                EvictionConfig &setMaximumSizePolicy(const MaxSizePolicy &maxSizePolicy);

                EvictionPolicy getEvictionPolicy() const;

                EvictionConfig &setEvictionPolicy(EvictionPolicy policy);

                internal::eviction::EvictionStrategyType::Type getEvictionStrategyType() const;

                internal::eviction::EvictionPolicyType getEvictionPolicyType() const;

            protected:
                int32_t size;
                MaxSizePolicy maxSizePolicy;
                EvictionPolicy evictionPolicy;
            };

            std::ostream & HAZELCAST_API operator<<(std::ostream &out, const EvictionConfig &config);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


