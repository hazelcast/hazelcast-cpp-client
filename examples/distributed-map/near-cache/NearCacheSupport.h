/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by ihsan demir on 13 Jan 2017.
//

#ifndef HAZELCASTCLIENT_NEARCACHESUPPORT_H
#define HAZELCASTCLIENT_NEARCACHESUPPORT_H

#include <stdio.h>
#include <stdint.h>

#include <hazelcast/client/HazelcastAll.h>

class NearCacheSupport {
public:
    static const int CACHE_INVALIDATION_MESSAGE_BATCH_FREQUENCY_SECONDS = 10; // seconds

    static void printNearCacheStats(hazelcast::client::IMap<int, std::string> &map, const char *message) {
        hazelcast::client::monitor::NearCacheStats *stats = map.getLocalMapStats().getNearCacheStats();
        printf("%s (%ld entries, %ld hits, %ld misses, %ld evictions, %ld expirations)\n",
               message, (long) stats->getOwnedEntryCount(), (long) stats->getHits(), (long) stats->getMisses(),
               (long) stats->getEvictions(), (long) stats->getExpirations());
    }

    static void printNearCacheStats(hazelcast::client::IMap<int, std::string> &map) {
        hazelcast::client::monitor::NearCacheStats *stats = map.getLocalMapStats().getNearCacheStats();

        printf("The Near Cache contains %ld entries.\n", (long) stats->getOwnedEntryCount());
        printf("The first article instance was retrieved from the remote instance (Near Cache misses: %ld).\n",
               (long) stats->getMisses());
        printf("The second and third article instance were retrieved from the local Near Cache (Near Cache hits: %ld).\n",
                (long) stats->getHits());
    }

    static void waitForInvalidationEvents() {
        hazelcast::util::sleep(2 * CACHE_INVALIDATION_MESSAGE_BATCH_FREQUENCY_SECONDS);
    }

    static void waitForNearCacheEvictionCount(hazelcast::client::IMap<int, std::string> &map, int64_t expectedEvictionCount) {
        int64_t evictionCount;
        do {
            hazelcast::client::monitor::NearCacheStats *stats = map.getLocalMapStats().getNearCacheStats();
            evictionCount = stats->getEvictions();
            hazelcast::util::sleepmillis(500);
        } while (evictionCount > expectedEvictionCount);
    }
};

#endif //HAZELCASTCLIENT_NEARCACHESUPPORT_H
