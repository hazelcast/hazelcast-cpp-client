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
#include <string>

#include "hazelcast/util/hazelcast_dll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace monitor {
            class HAZELCAST_API LocalInstanceStats {
            public:
                virtual ~LocalInstanceStats() = default;

                /**
                 * Fill a stat value with this if it is not available
                 */
                static const int64_t STAT_NOT_AVAILABLE;

                virtual int64_t get_creation_time() = 0;
            };

            class HAZELCAST_API NearCacheStats : public LocalInstanceStats {
            public:
                /**
                 * Returns the creation time of this Near Cache on this member.
                 *
                 * @return creation time of this Near Cache on this member.
                 */
                int64_t get_creation_time() = 0;

                /**
                 * Returns the number of Near Cache entries owned by this member.
                 *
                 * @return number of Near Cache entries owned by this member.
                 */
                virtual int64_t get_owned_entry_count() = 0;

                /**
                 * Returns memory cost (number of bytes) of Near Cache entries owned by this member.
                 *
                 * @return memory cost (number of bytes) of Near Cache entries owned by this member.
                 */
                virtual int64_t get_owned_entry_memory_cost() = 0;

                /**
                 * Returns the number of hits (reads) of Near Cache entries owned by this member.
                 *
                 * @return number of hits (reads) of Near Cache entries owned by this member.
                 */
                virtual int64_t get_hits() = 0;

                /**
                 * Returns the number of misses of Near Cache entries owned by this member.
                 *
                 * @return number of misses of Near Cache entries owned by this member.
                 */
                virtual int64_t get_misses() = 0;

                /**
                 * Returns the hit/miss ratio of Near Cache entries owned by this member.
                 *
                 * @return hit/miss ratio of Near Cache entries owned by this member.
                 */
                virtual double get_ratio() = 0;

                /**
                 * Returns the number of evictions of Near Cache entries owned by this member.
                 *
                 * @return number of evictions of Near Cache entries owned by this member.
                 */
                virtual int64_t get_evictions() = 0;

                /**
                 * Returns the number of TTL and max-idle expirations of Near Cache entries owned by this member.
                 *
                 * @return number of TTL and max-idle expirations of Near Cache entries owned by this member.
                 */
                virtual int64_t get_expirations() = 0;

                /**
                 * Returns the number of invalidations of Near Cache entries owned by this member.
                 *
                 * @return number of invalidations of Near Cache entries owned by this member
                 */
                virtual int64_t get_invalidations() = 0;

                /**
                 * Returns the number of Near Cache key persistences (when the pre-load feature is enabled).
                 *
                 * @return the number of Near Cache key persistences
                 */
                virtual int64_t get_persistence_count() = 0;

                /**
                 * Returns the timestamp of the last Near Cache key persistence (when the pre-load feature is enabled).
                 *
                 * @return the timestamp of the last Near Cache key persistence
                 */
                virtual int64_t get_last_persistence_time() = 0;

                /**
                 * Returns the duration in milliseconds of the last Near Cache key persistence (when the pre-load feature is enabled).
                 *
                 * @return the duration of the last Near Cache key persistence (in milliseconds)
                 */
                virtual int64_t get_last_persistence_duration() = 0;

                /**
                 * Returns the written bytes of the last Near Cache key persistence (when the pre-load feature is enabled).
                 *
                 * @return the written bytes of the last Near Cache key persistence
                 */
                virtual int64_t get_last_persistence_written_bytes() = 0;

                /**
                 * Returns the number of persisted keys of the last Near Cache key persistence (when the pre-load feature is enabled).
                 *
                 * @return the number of persisted keys of the last Near Cache key persistence
                 */
                virtual int64_t get_last_persistence_key_count() = 0;

                /**
                 * Returns the failure reason of the last Near Cache persistence (when the pre-load feature is enabled).
                 *
                 * @return the failure reason of the last Near Cache persistence
                 */
                virtual std::string get_last_persistence_failure() = 0;

                virtual std::string to_string() = 0;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



