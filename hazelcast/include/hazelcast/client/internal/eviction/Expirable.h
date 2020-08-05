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

#include <stdint.h>

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
                 * Expiring Data model interface.
                 * <p>This interface provides a time variable to be compared against other time values
                 * to decide on "future" or "past".</p>
                 */
                class HAZELCAST_API Expirable {
                public:
                    virtual ~Expirable() = default;

                    /**
                     * Gets the expiration time in milliseconds.
                     * @return expiration time.
                     * @see System#currentTimeMillis()
                     */
                    virtual int64_t getExpirationTime() const = 0;

                    /**
                     * Sets the expiration time in milliseconds.
                     * @param expirationTime
                     * @see System#currentTimeMillis()
                     */
                    virtual void setExpirationTime(int64_t expirationTime) = 0;

                    /**
                     * Checks whether the expiration time is passed with respect to the provided time.
                     * <p>Returns <tt>true</tt> if and only if {@code now > getExpirationTime()}</p>.
                     * @param now time in milliseconds.
                     * @return true if expired.
                     */
                    virtual bool isExpiredAt(int64_t now) const = 0;
                };
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 


