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
#ifndef HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONCONFIGURATION_H_
#define HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONCONFIGURATION_H_

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/internal/eviction/EvictionStrategyType.h"

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
                class HAZELCAST_API EvictionConfiguration {
                public:
/*
                    */
/**
                     * Gets the type of eviction strategy.
                     *
                     * @return the type of eviction strategy
                     *//*

                    virtual const EvictionStrategyType &getEvictionStrategyType() const = 0;

                    */
/**
                     * Gets the type of eviction policy.
                     *
                     * @return the type of eviction policy
                     *//*

                    virtual const EvictionPolicyType &getEvictionPolicyType() const = 0;

                    */
/**
                     * Gets the class name of the configured {@link EvictionPolicyComparator} implementation.
                     *
                     * @return class name of the configured {@link EvictionPolicyComparator} implementation
                     *//*

*/
/*
                    virtual std::string &getComparatorClassName();
*//*


                    */
/**
                     * Gets instance of the configured {@link EvictionPolicyComparator} implementation.
                     *
                     * @return instance of the configured {@link EvictionPolicyComparator} implementation.
                     *//*

                    virtual const EvictionPolicyComparator &getComparator() const = 0;
*/
                };
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONCONFIGURATION_H_ */
