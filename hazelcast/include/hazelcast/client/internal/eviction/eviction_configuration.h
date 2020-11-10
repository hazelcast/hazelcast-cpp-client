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
#include <memory>

#include "hazelcast/client/config/eviction_strategy_type.h"
#include "hazelcast/client/config/eviction_policy.h"
#include "hazelcast/client/config/eviction_strategy_type.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                /**
                 * Interface for configuration information about eviction.
                 */
                template <typename K, typename V>
                class eviction_configuration {
                public:
                    virtual ~eviction_configuration() = default;

                    /**
                     * Gets the type of eviction strategy.
                     *
                     * @return the type of eviction strategy
                     */
                    virtual client::config::eviction_strategy_type get_eviction_strategy_type() const = 0;

                    /**
                     * Gets the type of eviction policy.
                     *
                     * @return the type of eviction policy
                     */
                    virtual ::hazelcast::client::config::eviction_policy get_eviction_policy() const = 0;

                    /**
                     * Gets the class name of the configured {@link EvictionPolicyComparator} implementation.
                     *
                     * @return class name of the configured {@link EvictionPolicyComparator} implementation
                     */
/*
                    virtual String getComparatorClassName();
*/

                    /**
                     * Gets instance of the configured {@link EvictionPolicyComparator} implementation.
                     *
                     * @return instance of the configured {@link EvictionPolicyComparator} implementation.
                     */
                    virtual const std::shared_ptr<EvictionPolicyComparator<K, V> > get_comparator() const {
                        assert(0);
                        return std::shared_ptr<EvictionPolicyComparator<K, V> >();
                    }
                };
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 


