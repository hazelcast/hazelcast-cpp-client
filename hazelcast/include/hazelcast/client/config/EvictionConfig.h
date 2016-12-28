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
#ifndef HAZELCAST_CLIENT_CONFIG_EVICTIONCONFIG_H_
#define HAZELCAST_CLIENT_CONFIG_EVICTIONCONFIG_H_

#include <string>
#include <stdint.h>

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
            class HAZELCAST_API EvictionConfig : public internal::eviction::EvictionConfiguration {
            public:
                /**
                 * Maximum Size Policy
                 */
                enum MaxSizePolicy {
                    /**
                     * Policy based on maximum number of entries stored per data structure (map, cache etc)
                     */
                            ENTRY_COUNT,
                    /**
                     * Policy based on maximum used native memory in megabytes per data structure (map, cache etc)
                     * on each Hazelcast instance
                     */
                            USED_NATIVE_MEMORY_SIZE,
                    /**
                     * Policy based on maximum used native memory percentage per data structure (map, cache etc)
                     * on each Hazelcast instance
                     */
                            USED_NATIVE_MEMORY_PERCENTAGE,
                    /**
                     * Policy based on minimum free native memory in megabytes per Hazelcast instance
                     */
                            FREE_NATIVE_MEMORY_SIZE,
                    /**
                     * Policy based on minimum free native memory percentage per Hazelcast instance
                     */
                            FREE_NATIVE_MEMORY_PERCENTAGE
                };

                EvictionConfig();

                /**
                 * Default maximum entry count.
                 */
                static const int32_t DEFAULT_MAX_ENTRY_COUNT;

                /**
                 * Default maximum entry count for Map on-heap Near Caches.
                 */
                static const int32_t DEFAULT_MAX_ENTRY_COUNT_FOR_ON_HEAP_MAP;

                /**
                 * Default Max-Size Policy.
                 */
                static const MaxSizePolicy DEFAULT_MAX_SIZE_POLICY;

                /**
                 * Default Eviction Policy.
                 */
                static const EvictionPolicy DEFAULT_EVICTION_POLICY;

                int32_t getSize() const;

                EvictionConfig &setSize(int32_t size);

                EvictionPolicy getEvictionPolicy() const;

                EvictionConfig &setMaximumSizePolicy(const MaxSizePolicy &maxSizePolicy);

            protected:
                int32_t size;
                MaxSizePolicy maxSizePolicy;
                EvictionPolicy evictionPolicy;
                std::string comparatorClassName;
/*
           const EvictionPolicyComparator comparator;
*/
            private:
                /**
                 * Used by the {@link NearCacheConfigAccessor} to initialize the proper default value for on-heap maps.
                 */
                bool sizeConfigured;

            };

            std::ostream &operator<<(std::ostream &out, const EvictionConfig &config);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_CONFIG_EVICTIONCONFIG_H_ */
