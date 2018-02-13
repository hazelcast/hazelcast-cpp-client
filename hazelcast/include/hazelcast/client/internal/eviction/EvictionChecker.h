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
#ifndef HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONCHECKER_H
#define HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONCHECKER_H

#include <memory>

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
                 * Interface for checking about if eviction is required or not.
                 */
                class HAZELCAST_API EvictionChecker {
                public:
                    virtual ~EvictionChecker() { }

                    /**
                     * Empty {@link} EvictionChecker to allow eviction always.
                     */
                    static const std::auto_ptr <EvictionChecker> EVICT_ALWAYS;

                    /**
                     * Checks for if eviction is required or not.
                     *
                     * @return <code>true</code> if eviction is required, otherwise <code>false</code>
                     */
                    virtual bool isEvictionRequired() const = 0;
                };

                class HAZELCAST_API EvictAlways : public EvictionChecker {
                public:
                    //@override
                    bool isEvictionRequired() const;
                };
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_EVICTION_EVICTIONCHECKER_H */
