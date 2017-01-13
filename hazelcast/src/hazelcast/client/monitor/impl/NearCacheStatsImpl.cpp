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
#include <iomanip>
#include <limits>
#include <sstream>

#include <hazelcast/client/monitor/impl/NearCacheStatsImpl.h>
#include <hazelcast/util/Util.h>

namespace hazelcast {
    namespace client {
        namespace monitor {
            namespace impl {
                NearCacheStatsImpl::NearCacheStatsImpl() : creationTime(util::currentTimeMillis()),
                                                           ownedEntryCount(0),
                                                           ownedEntryMemoryCost(0),
                                                           hits(0),
                                                           misses(0),
                                                           evictions(0),
                                                           expirations(0),
                                                           persistenceCount(0),
                                                           lastPersistenceTime(0),
                                                           lastPersistenceDuration(0),
                                                           lastPersistenceWrittenBytes(0),
                                                           lastPersistenceKeyCount(0),
                                                           lastPersistenceFailure("") {
                }

                int64_t NearCacheStatsImpl::getCreationTime() {
                    return creationTime;
                }

                int64_t NearCacheStatsImpl::getOwnedEntryCount() {
                    return ownedEntryCount;
                }

                void NearCacheStatsImpl::setOwnedEntryCount(int64_t ownedEntryCount) {
                    this->ownedEntryCount = ownedEntryCount;
                }

                void NearCacheStatsImpl::incrementOwnedEntryCount() {
                    ++ownedEntryCount;
                }

                void NearCacheStatsImpl::decrementOwnedEntryCount() {
                    --ownedEntryCount;
                }

                int64_t NearCacheStatsImpl::getOwnedEntryMemoryCost() {
                    return ownedEntryMemoryCost;
                }

                void NearCacheStatsImpl::setOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost) {
                    this->ownedEntryMemoryCost = ownedEntryMemoryCost;
                }

                void NearCacheStatsImpl::incrementOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost) {
                    this->ownedEntryMemoryCost += ownedEntryMemoryCost;
                }

                void NearCacheStatsImpl::decrementOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost) {
                    this->ownedEntryMemoryCost -= ownedEntryMemoryCost;
                }

                int64_t NearCacheStatsImpl::getHits() {
                    return hits;
                }

                // just for testing
                void NearCacheStatsImpl::setHits(int64_t hits) {
                    this->hits = hits;
                }

                void NearCacheStatsImpl::incrementHits() {
                    ++hits;
                }

                int64_t NearCacheStatsImpl::getMisses() {
                    return misses;
                }

                // just for testing
                void NearCacheStatsImpl::setMisses(int64_t misses) {
                    this->misses = misses;
                }

                void NearCacheStatsImpl::incrementMisses() {
                    ++misses;
                }

                double NearCacheStatsImpl::getRatio() {
                    if (misses == (int64_t) 0) {
                        if (hits == (int64_t) 0) {
                            return std::numeric_limits<double>::signaling_NaN();
                        } else {
                            return std::numeric_limits<double>::infinity();
                        }
                    } else {
                        return ((double) hits / misses) * PERCENTAGE;
                    }
                }

                int64_t NearCacheStatsImpl::getEvictions() {
                    return evictions;
                }

                void NearCacheStatsImpl::incrementEvictions() {
                    ++evictions;
                }

                int64_t NearCacheStatsImpl::getExpirations() {
                    return expirations;
                }

                void NearCacheStatsImpl::incrementExpirations() {
                    ++expirations;
                }

                int64_t NearCacheStatsImpl::getPersistenceCount() {
                    return persistenceCount;
                }

                void NearCacheStatsImpl::addPersistence(int64_t duration, int32_t writtenBytes, int32_t keyCount) {
                    ++persistenceCount;
                    lastPersistenceTime = util::currentTimeMillis();
                    lastPersistenceDuration = duration;
                    lastPersistenceWrittenBytes = writtenBytes;
                    lastPersistenceKeyCount = keyCount;
                    lastPersistenceFailure = "";
                }

                int64_t NearCacheStatsImpl::getLastPersistenceTime() {
                    return lastPersistenceTime;
                }

                int64_t NearCacheStatsImpl::getLastPersistenceDuration() {
                    return lastPersistenceDuration;
                }

                int64_t NearCacheStatsImpl::getLastPersistenceWrittenBytes() {
                    return lastPersistenceWrittenBytes;
                }

                int64_t NearCacheStatsImpl::getLastPersistenceKeyCount() {
                    return lastPersistenceKeyCount;
                }

                std::string NearCacheStatsImpl::getLastPersistenceFailure() {
                    return lastPersistenceFailure;
                }

                std::string NearCacheStatsImpl::toString() {
                    std::ostringstream out;
                    std::string failureString = lastPersistenceFailure;
                    out << "NearCacheStatsImpl{"
                    << "ownedEntryCount=" << ownedEntryCount
                    << ", ownedEntryMemoryCost=" << ownedEntryMemoryCost
                    << ", creationTime=" << creationTime
                    << ", hits=" << hits
                    << ", misses=" << misses
                    << ", ratio=" << std::setprecision(1) << getRatio()
                    << ", evictions=" << evictions
                    << ", expirations=" << expirations
                    << ", lastPersistenceTime=" << lastPersistenceTime
                    << ", persistenceCount=" << persistenceCount
                    << ", lastPersistenceDuration=" << lastPersistenceDuration
                    << ", lastPersistenceWrittenBytes=" << lastPersistenceWrittenBytes
                    << ", lastPersistenceKeyCount=" << lastPersistenceKeyCount
                    << ", lastPersistenceFailure='" << failureString << "'"
                    << '}';

                    return out.str();
                }

                const double NearCacheStatsImpl::PERCENTAGE = 100.0;
            }
        }
    }
}


