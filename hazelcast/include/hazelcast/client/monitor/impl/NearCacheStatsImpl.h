/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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
#include <atomic>

#include "hazelcast/client/monitor/near_cache_stats.h"
#include "hazelcast/util/Sync.h"

#include "hazelcast/util/export.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace monitor {
namespace impl {
class HAZELCAST_API NearCacheStatsImpl : public near_cache_stats
{
public:
    NearCacheStatsImpl();

    int64_t get_creation_time() override;

    int64_t get_owned_entry_count() override;

    void set_owned_entry_count(int64_t owned_entry_count);

    void increment_owned_entry_count();
    void decrement_owned_entry_count();

    int64_t get_owned_entry_memory_cost() override;

    void set_owned_entry_memory_cost(int64_t owned_entry_memory_cost);

    void increment_owned_entry_memory_cost(int64_t owned_entry_memory_cost);

    void decrement_owned_entry_memory_cost(int64_t owned_entry_memory_cost);

    int64_t get_hits() override;

    // just for testing
    void set_hits(int64_t hits);

    void increment_hits();

    int64_t get_misses() override;

    // just for testing
    void set_misses(int64_t misses);

    void increment_misses();

    double get_ratio() override;

    int64_t get_evictions() override;

    void increment_evictions();

    int64_t get_expirations() override;

    void increment_expirations();

    int64_t get_invalidations() override;

    void increment_invalidations();

    int64_t get_invalidation_requests();

    void increment_invalidation_requests();

    void reset_invalidation_events();

    int64_t get_persistence_count() override;

    void add_persistence(int64_t duration,
                         int32_t written_bytes,
                         int32_t key_count);

    int64_t get_last_persistence_time() override;

    int64_t get_last_persistence_duration() override;

    int64_t get_last_persistence_written_bytes() override;

    int64_t get_last_persistence_key_count() override;

    std::string get_last_persistence_failure() override;

    std::string to_string() override;

private:
    std::atomic<int64_t> creation_time_;
    std::atomic<int64_t> owned_entry_count_;
    std::atomic<int64_t> owned_entry_memory_cost_;
    std::atomic<int64_t> hits_;
    std::atomic<int64_t> misses_;
    std::atomic<int64_t> evictions_;
    std::atomic<int64_t> expirations_;

    std::atomic<int64_t> invalidations_;
    std::atomic<int64_t> invalidation_requests_;

    std::atomic<int64_t> persistence_count_;
    std::atomic<int64_t> last_persistence_time_;
    std::atomic<int64_t> last_persistence_duration_;
    std::atomic<int64_t> last_persistence_written_bytes_;
    std::atomic<int64_t> last_persistence_key_count_;
    util::Sync<std::string> last_persistence_failure_;

    static const double PERCENTAGE;
};
} // namespace impl
} // namespace monitor
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
