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
#ifndef HAZELCAST_CLIENT_MONITOR_IMPL_NEARCACHESTATSIMPL_H_
#define HAZELCAST_CLIENT_MONITOR_IMPL_NEARCACHESTATSIMPL_H_

#include <stdint.h>
#include <string>

#include "hazelcast/client/monitor/NearCacheStats.h"
#include "hazelcast/util/Atomic.h"
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

                    virtual int64_t getCreationTime();

                    virtual int64_t getOwnedEntryCount();

                    void setOwnedEntryCount(int64_t ownedEntryCount);

                    void incrementOwnedEntryCount();
                    void decrementOwnedEntryCount();

                    virtual int64_t getOwnedEntryMemoryCost();

                    void setOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost);

                    void incrementOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost);

                    void decrementOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost);

                    virtual int64_t getHits();

                    // just for testing
                    void setHits(int64_t hits);

                    void incrementHits();

                    virtual int64_t getMisses();

                    // just for testing
                    void setMisses(int64_t misses);

                    void incrementMisses();

                    virtual double getRatio();

                    virtual int64_t getEvictions();

                    void incrementEvictions();

                    virtual int64_t getExpirations();

                    void incrementExpirations();

                    virtual int64_t getPersistenceCount();

                    void addPersistence(int64_t duration, int32_t writtenBytes, int32_t keyCount);

                    virtual int64_t getLastPersistenceTime();

                    virtual int64_t getLastPersistenceDuration();

                    virtual int64_t getLastPersistenceWrittenBytes();

                    virtual int64_t getLastPersistenceKeyCount();

                    virtual std::string getLastPersistenceFailure();

                    virtual std::string toString();
                private:
                    util::Atomic<int64_t> creationTime;
                    util::Atomic<int64_t> ownedEntryCount;
                    util::Atomic<int64_t> ownedEntryMemoryCost;
                    util::Atomic<int64_t> hits;
                    util::Atomic<int64_t> misses;
                    util::Atomic<int64_t> evictions;
                    util::Atomic<int64_t> expirations;

                    util::Atomic<int64_t> persistenceCount;
                    util::Atomic<int64_t> lastPersistenceTime;
                    util::Atomic<int64_t> lastPersistenceDuration;
                    util::Atomic<int64_t> lastPersistenceWrittenBytes;
                    util::Atomic<int64_t> lastPersistenceKeyCount;
                    util::Atomic<std::string> lastPersistenceFailure;

                    static const double PERCENTAGE;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MONITOR_IMPL_NEARCACHESTATSIMPL_H_ */

