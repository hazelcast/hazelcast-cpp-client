/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
            template<typename K, typename V>
            class EvictionConfig : public internal::eviction::EvictionConfiguration<K, V> {
            public:
                virtual ~EvictionConfig() {
                }

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
                static const int32_t DEFAULT_MAX_ENTRY_COUNT;

                /**
                 * Default Max-Size Policy.
                 */
                static const MaxSizePolicy DEFAULT_MAX_SIZE_POLICY;

                /**
                 * Default Eviction Policy.
                 */
                static const EvictionPolicy DEFAULT_EVICTION_POLICY;

                EvictionConfig() : size(DEFAULT_MAX_ENTRY_COUNT), maxSizePolicy(DEFAULT_MAX_SIZE_POLICY),
                            evictionPolicy(DEFAULT_EVICTION_POLICY) {
                }


                EvictionConfig(int size, MaxSizePolicy maxSizePolicy,
                               const std::shared_ptr<internal::eviction::EvictionPolicyComparator<K, V> > &comparator) {
                    this->size = util::Preconditions::checkPositive(size, "Size must be positive number!");
                    this->maxSizePolicy = maxSizePolicy;
                    this->comparator = util::Preconditions::checkNotNull<internal::eviction::EvictionPolicyComparator>(
                            comparator, "Comparator cannot be null!");
                }

                int32_t getSize() const {
                    return size;
                }

                EvictionConfig &setSize(int32_t size) {
                    this->size = util::Preconditions::checkPositive(size, "Size must be positive number!");
                    return *this;
                }

                MaxSizePolicy getMaximumSizePolicy() const {
                    return maxSizePolicy;
                }

                EvictionConfig &setMaximumSizePolicy(const MaxSizePolicy &maxSizePolicy) {
                    this->maxSizePolicy = maxSizePolicy;
                    return *this;
                }

                EvictionPolicy getEvictionPolicy() const {
                    return evictionPolicy;
                }

                EvictionConfig<K, V> &setEvictionPolicy(EvictionPolicy policy) {
                    this->evictionPolicy = policy;
                    return *this;
                }

                const std::shared_ptr<internal::eviction::EvictionPolicyComparator<K, V> > getComparator() const {
                    return comparator;
                }

                EvictionConfig &setComparator(
                        const std::shared_ptr<internal::eviction::EvictionPolicyComparator<K, V> > &comparator) {
                    this->comparator = comparator;
                    return *this;
                }

                internal::eviction::EvictionStrategyType::Type getEvictionStrategyType() const {
                    // TODO: add support for other/custom eviction strategies
                    return internal::eviction::EvictionStrategyType::DEFAULT_EVICTION_STRATEGY;
                }

                internal::eviction::EvictionPolicyType getEvictionPolicyType() const {
                    if (evictionPolicy == LFU) {
                        return internal::eviction::LFU;
                    } else if (evictionPolicy == LRU) {
                        return internal::eviction::LRU;
                    } else if (evictionPolicy == RANDOM) {
                        return internal::eviction::RANDOM;
                    } else if (evictionPolicy == NONE) {
                        return internal::eviction::NONE;
                    } else {
                        assert(0);
                    }
		            return internal::eviction::NONE;
                }

            protected:
                int32_t size;
                MaxSizePolicy maxSizePolicy;
                EvictionPolicy evictionPolicy;
                std::shared_ptr<internal::eviction::EvictionPolicyComparator<K, V> > comparator;
            };

            template <typename K, typename V>
            std::ostream &operator<<(std::ostream &out, const EvictionConfig<K, V> &config) {
                out << "EvictionConfig{"
                    << "size=" << config.getSize()
                    << ", maxSizePolicy=" << config.getMaximumSizePolicy()
                    << ", evictionPolicy=" << config.getEvictionPolicy()
                    << ", comparator=" << config.getComparator()
                    << '}';

                return out;
            }

            /**
             * Default maximum entry count.
             */
            template <typename K, typename V>
            const int EvictionConfig<K, V>::DEFAULT_MAX_ENTRY_COUNT = INT32_MAX;

            /**
             * Default Max-Size Policy.
             */
            template <typename K, typename V>
            const typename EvictionConfig<K, V>::MaxSizePolicy EvictionConfig<K, V>::DEFAULT_MAX_SIZE_POLICY = ENTRY_COUNT;

            /**
             * Default Eviction Policy.
             */
            template <typename K, typename V>
            const EvictionPolicy EvictionConfig<K, V>::DEFAULT_EVICTION_POLICY = LRU;
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_CONFIG_EVICTIONCONFIG_H_ */
