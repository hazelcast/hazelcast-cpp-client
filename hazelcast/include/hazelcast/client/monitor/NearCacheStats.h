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
#ifndef HAZELCAST_CLIENT_MONITOR_NEARCACHESTATS_H_
#define HAZELCAST_CLIENT_MONITOR_NEARCACHESTATS_H_

#include <stdint.h>
#include <string>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace monitor {
            class HAZELCAST_API LocalInstanceStats {
            public:
                virtual ~LocalInstanceStats() {
                }

                /**
                 * Fill a stat value with this if it is not available
                 */
                static const int64_t STAT_NOT_AVAILABLE;

                virtual int64_t getCreationTime() = 0;
            };

            class HAZELCAST_API NearCacheStats : public LocalInstanceStats {
            public:
                /**
                 * Returns the creation time of this Near Cache on this member.
                 *
                 * @return creation time of this Near Cache on this member.
                 */
                //@Override
                virtual int64_t getCreationTime() = 0;

                /**
                 * Returns the number of Near Cache entries owned by this member.
                 *
                 * @return number of Near Cache entries owned by this member.
                 */
                virtual int64_t getOwnedEntryCount() = 0;

                /**
                 * Returns memory cost (number of bytes) of Near Cache entries owned by this member.
                 *
                 * @return memory cost (number of bytes) of Near Cache entries owned by this member.
                 */
                virtual int64_t getOwnedEntryMemoryCost() = 0;

                /**
                 * Returns the number of hits (reads) of Near Cache entries owned by this member.
                 *
                 * @return number of hits (reads) of Near Cache entries owned by this member.
                 */
                virtual int64_t getHits() = 0;

                /**
                 * Returns the number of misses of Near Cache entries owned by this member.
                 *
                 * @return number of misses of Near Cache entries owned by this member.
                 */
                virtual int64_t getMisses() = 0;

                /**
                 * Returns the hit/miss ratio of Near Cache entries owned by this member.
                 *
                 * @return hit/miss ratio of Near Cache entries owned by this member.
                 */
                virtual double getRatio() = 0;

                /**
                 * Returns the number of evictions of Near Cache entries owned by this member.
                 *
                 * @return number of evictions of Near Cache entries owned by this member.
                 */
                virtual int64_t getEvictions() = 0;

                /**
                 * Returns the number of TTL and max-idle expirations of Near Cache entries owned by this member.
                 *
                 * @return number of TTL and max-idle expirations of Near Cache entries owned by this member.
                 */
                virtual int64_t getExpirations() = 0;

                /**
                 * Returns the number of invalidations of Near Cache entries owned by this member.
                 *
                 * @return number of invalidations of Near Cache entries owned by this member
                 */
                virtual int64_t getInvalidations() = 0;

                /**
                 * Returns the number of Near Cache key persistences (when the pre-load feature is enabled).
                 *
                 * @return the number of Near Cache key persistences
                 */
                virtual int64_t getPersistenceCount() = 0;

                /**
                 * Returns the timestamp of the last Near Cache key persistence (when the pre-load feature is enabled).
                 *
                 * @return the timestamp of the last Near Cache key persistence
                 */
                virtual int64_t getLastPersistenceTime() = 0;

                /**
                 * Returns the duration in milliseconds of the last Near Cache key persistence (when the pre-load feature is enabled).
                 *
                 * @return the duration of the last Near Cache key persistence (in milliseconds)
                 */
                virtual int64_t getLastPersistenceDuration() = 0;

                /**
                 * Returns the written bytes of the last Near Cache key persistence (when the pre-load feature is enabled).
                 *
                 * @return the written bytes of the last Near Cache key persistence
                 */
                virtual int64_t getLastPersistenceWrittenBytes() = 0;

                /**
                 * Returns the number of persisted keys of the last Near Cache key persistence (when the pre-load feature is enabled).
                 *
                 * @return the number of persisted keys of the last Near Cache key persistence
                 */
                virtual int64_t getLastPersistenceKeyCount() = 0;

                /**
                 * Returns the failure reason of the last Near Cache persistence (when the pre-load feature is enabled).
                 *
                 * @return the failure reason of the last Near Cache persistence
                 */
                virtual std::string getLastPersistenceFailure() = 0;

                virtual std::string toString() = 0;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MONITOR_NEARCACHESTATS_H_ */

