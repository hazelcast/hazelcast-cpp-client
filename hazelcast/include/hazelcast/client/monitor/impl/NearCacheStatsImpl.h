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
#include <atomic>

#include "hazelcast/client/monitor/NearCacheStats.h"
#include "hazelcast/util/Sync.h"

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace monitor {
            namespace impl {
                class HAZELCAST_API NearCacheStatsImpl : public NearCacheStats {
                public:
                    NearCacheStatsImpl();

                    int64_t getCreationTime() override;

                    int64_t getOwnedEntryCount() override;

                    void setOwnedEntryCount(int64_t ownedEntryCount);

                    void incrementOwnedEntryCount();
                    void decrementOwnedEntryCount();

                    int64_t getOwnedEntryMemoryCost() override;

                    void setOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost);

                    void incrementOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost);

                    void decrementOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost);

                    int64_t getHits() override;

                    // just for testing
                    void setHits(int64_t hits);

                    void incrementHits();

                    int64_t getMisses() override;

                    // just for testing
                    void setMisses(int64_t misses);

                    void incrementMisses();

                    double getRatio() override;

                    int64_t getEvictions() override;

                    void incrementEvictions();

                    int64_t getExpirations() override;

                    void incrementExpirations();

                    int64_t getInvalidations() override;

                    void incrementInvalidations();

                    int64_t getInvalidationRequests();

                    void incrementInvalidationRequests();

                    void resetInvalidationEvents();

                    int64_t getPersistenceCount() override;

                    void addPersistence(int64_t duration, int32_t writtenBytes, int32_t keyCount);

                    int64_t getLastPersistenceTime() override;

                    int64_t getLastPersistenceDuration() override;

                    int64_t getLastPersistenceWrittenBytes() override;

                    int64_t getLastPersistenceKeyCount() override;

                    std::string getLastPersistenceFailure() override;

                    std::string toString() override;

                private:
                    std::atomic<int64_t> creationTime_;
                    std::atomic<int64_t> ownedEntryCount_;
                    std::atomic<int64_t> ownedEntryMemoryCost_;
                    std::atomic<int64_t> hits_;
                    std::atomic<int64_t> misses_;
                    std::atomic<int64_t> evictions_;
                    std::atomic<int64_t> expirations_;

                    std::atomic<int64_t> invalidations_;
                    std::atomic<int64_t> invalidationRequests_;

                    std::atomic<int64_t> persistenceCount_;
                    std::atomic<int64_t> lastPersistenceTime_;
                    std::atomic<int64_t> lastPersistenceDuration_;
                    std::atomic<int64_t> lastPersistenceWrittenBytes_;
                    std::atomic<int64_t> lastPersistenceKeyCount_;
                    util::Sync<std::string> lastPersistenceFailure_;

                    static const double PERCENTAGE;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

