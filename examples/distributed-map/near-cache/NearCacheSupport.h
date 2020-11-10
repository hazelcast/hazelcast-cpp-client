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

#include <hazelcast/client/hazelcast.h>

class NearCacheSupport {
public:
    static constexpr int CACHE_INVALIDATION_MESSAGE_BATCH_FREQUENCY_SECONDS = 10; // seconds

    static void print_near_cache_stats(std::shared_ptr<hazelcast::client::imap> &map, const char *message) {
        auto stats = map->get_local_map_stats().get_near_cache_stats();
        printf("%s (%ld entries, %ld hits, %ld misses, %ld evictions, %ld expirations)\n",
               message, (long) stats->get_owned_entry_count(), (long) stats->get_hits(), (long) stats->get_misses(),
               (long) stats->get_evictions(), (long) stats->get_expirations());
    }

    static void print_near_cache_stats(std::shared_ptr<hazelcast::client::imap> &map) {
        auto stats = map->get_local_map_stats().get_near_cache_stats();

        printf("The Near Cache contains %ld entries.\n", (long) stats->get_owned_entry_count());
        printf("The first article instance was retrieved from the remote instance (Near Cache misses: %ld).\n",
               (long) stats->get_misses());
        printf("The second and third article instance were retrieved from the local Near Cache (Near Cache hits: %ld).\n",
               (long) stats->get_hits());
    }

    static void wait_for_invalidation_events() {
        std::this_thread::sleep_for(std::chrono::seconds(2 * CACHE_INVALIDATION_MESSAGE_BATCH_FREQUENCY_SECONDS));
    }

    static void
    wait_for_near_cache_eviction_count(std::shared_ptr<hazelcast::client::imap> &map, int64_t expected_eviction_count) {
        int64_t evictionCount;
        do {
            auto stats = map->get_local_map_stats().get_near_cache_stats();
            evictionCount = stats->get_evictions();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } while (evictionCount > expected_eviction_count);
    }
};
