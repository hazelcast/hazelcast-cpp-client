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

#include "hazelcast/util/HazelcastDll.h"

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
                class HAZELCAST_API EvictionStrategyType {
                public:
                    enum Type {
                        /**
                         * Sampling based eviction strategy type
                         */
                                SAMPLING_BASED_EVICTION
                    };
                    /**
                     * Default value of {@link com.hazelcast.internal.eviction.EvictionStrategyType}
                     */
                    static const Type DEFAULT_EVICTION_STRATEGY = SAMPLING_BASED_EVICTION;
                };
            }
        }
    }
};

namespace std {
    template<> struct hash<hazelcast::client::internal::eviction::EvictionStrategyType::Type> {
        std::size_t operator()(const hazelcast::client::internal::eviction::EvictionStrategyType::Type &object) const noexcept {
            return std::hash<int>{}(static_cast<int>(object));
        }
    };
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


