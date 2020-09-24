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

#include <regex>
#include <iomanip>

#include "hazelcast/client/impl/statistics/Statistics.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/monitor/impl/NearCacheStatsImpl.h"
#include "hazelcast/client/monitor/impl/LocalMapStatsImpl.h"
#include "hazelcast/client/ClientConfig.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            namespace statistics {
                const std::string Statistics::NEAR_CACHE_CATEGORY_PREFIX("nc.");

                Statistics::Statistics(spi::ClientContext &clientContext) : client_context_(clientContext),
                                                                            client_properties_(
                                                                                    clientContext.getClientProperties()),
                                                                            logger_(clientContext.getLogger()),
                                                                            periodic_stats_(*this) {
                    this->enabled_ = client_properties_.getBoolean(client_properties_.getStatisticsEnabled());
                }

                void Statistics::start() {
                    if (!enabled_) {
                        return;
                    }

                    int64_t periodSeconds = client_properties_.getLong(client_properties_.getStatisticsPeriodSeconds());
                    if (periodSeconds <= 0) {

                        int64_t defaultValue = util::IOUtil::to_value<int64_t>(
                                client_properties_.getStatisticsPeriodSeconds().getDefaultValue());
                        logger_.warning("Provided client statistics ",
                                       client_properties_.getStatisticsPeriodSeconds().getName(),
                                       " cannot be less than or equal to 0. You provided ", periodSeconds,
                                       " seconds as the configuration. Client will use the default value of ",
                                       defaultValue, " instead.");
                        periodSeconds = defaultValue;
                    }

                    schedulePeriodicStatisticsSendTask(periodSeconds);

                    logger_.info("Client statistics is enabled with period ", periodSeconds, " seconds.");
                }

                void Statistics::shutdown() {
                    if (send_task_timer_) {
                        boost::system::error_code ignored;
                        send_task_timer_->cancel(ignored);
                    }
                }

                void Statistics::schedulePeriodicStatisticsSendTask(int64_t periodSeconds) {
                    send_task_timer_ = client_context_.getClientExecutionService().scheduleWithRepetition([=]() {
                        if (!client_context_.getLifecycleService().isRunning()) {
                            return;
                        }

                        auto collection_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch()).count();
                        std::shared_ptr<connection::Connection> connection = getConnection();
                        if (!connection) {
                            logger_.finest("annot send client statistics to the server. No connection found.");
                            return;
                        }

                        std::ostringstream stats;

                        periodic_stats_.fillMetrics(stats, connection);

                        periodic_stats_.addNearCacheStats(stats);

                        sendStats(collection_timestamp, stats.str(), connection);
                    }, std::chrono::seconds(0), std::chrono::seconds(periodSeconds));
                }

                std::shared_ptr<connection::Connection> Statistics::getConnection() {
                    return client_context_.getConnectionManager().get_random_connection();
                }

                void Statistics::sendStats(int64_t timestamp, const std::string &newStats,
                                           const std::shared_ptr<connection::Connection> &connection) {
                    // TODO: implement metrics blob
                    auto request = protocol::codec::client_statistics_encode(timestamp, newStats, std::vector<byte>());
                    try {
                        spi::impl::ClientInvocation::create(client_context_, request, "", connection)->invoke();
                    } catch (exception::IException &e) {
                        // suppress exception, do not print too many messages
                        if (logger_.isFinestEnabled()) {
                            logger_.finest("Could not send stats ", e);
                        }
                    }
                }

                void Statistics::PeriodicStatistics::fillMetrics(std::ostringstream &stats,
                                                                 const std::shared_ptr<connection::Connection> &connection) {
                    stats << "lastStatisticsCollectionTime" << KEY_VALUE_SEPARATOR << util::currentTimeMillis();
                    addStat(stats, "enterprise", false);
                    addStat(stats, "clientType", protocol::ClientTypes::CPP);
                    addStat(stats, "clientVersion", HAZELCAST_VERSION);
                    addStat(stats, "clusterConnectionTimestamp", connection->getStartTime().time_since_epoch().count());

                    auto localSocketAddress = connection->getLocalSocketAddress();
                    stats << STAT_SEPARATOR << "clientAddress" << KEY_VALUE_SEPARATOR;
                    if (localSocketAddress) {
                        stats << localSocketAddress->getHost() << ":" << localSocketAddress->getPort();
                    }

                    addStat(stats, "clientName", statistics_.client_context_.getName());

                    auto credential = statistics_.client_context_.getConnectionManager().getCurrentCredentials();
                    if (credential) {
                        addStat(stats, "credentials.principal", credential->get_name());
                    }
                }

                void Statistics::PeriodicStatistics::addNearCacheStats(std::ostringstream &stats) {
                    for (const std::shared_ptr<internal::nearcache::BaseNearCache> &nearCache : statistics_.client_context_.getNearCacheManager().listAllNearCaches()) {
                        std::string nearCacheName = nearCache->getName();
                        std::ostringstream nearCacheNameWithPrefix;
                        getNameWithPrefix(nearCacheName, nearCacheNameWithPrefix);

                        nearCacheNameWithPrefix << '.';

                        auto nearCacheStats = std::static_pointer_cast<monitor::impl::NearCacheStatsImpl>(nearCache->getNearCacheStats());

                        std::string prefix = nearCacheNameWithPrefix.str();

                        addStat(stats, prefix, "creationTime", nearCacheStats->getCreationTime());
                        addStat(stats, prefix, "evictions", nearCacheStats->getEvictions());
                        addStat(stats, prefix, "hits", nearCacheStats->getHits());
                        addStat(stats, prefix, "lastPersistenceDuration",
                                nearCacheStats->getLastPersistenceDuration());
                        addStat(stats, prefix, "lastPersistenceKeyCount",
                                nearCacheStats->getLastPersistenceKeyCount());
                        addStat(stats, prefix, "lastPersistenceTime",
                                nearCacheStats->getLastPersistenceTime());
                        addStat(stats, prefix, "lastPersistenceWrittenBytes",
                                nearCacheStats->getLastPersistenceWrittenBytes());
                        addStat(stats, prefix, "misses", nearCacheStats->getMisses());
                        addStat(stats, prefix, "ownedEntryCount", nearCacheStats->getOwnedEntryCount());
                        addStat(stats, prefix, "expirations", nearCacheStats->getExpirations());
                        addStat(stats, prefix, "invalidations", nearCacheStats->getInvalidations());
                        addStat(stats, prefix, "invalidationRequests",
                                nearCacheStats->getInvalidationRequests());
                        addStat(stats, prefix, "ownedEntryMemoryCost",
                                nearCacheStats->getOwnedEntryMemoryCost());
                        std::string persistenceFailure = nearCacheStats->getLastPersistenceFailure();
                        if (!persistenceFailure.empty()) {
                            addStat(stats, prefix, "lastPersistenceFailure", persistenceFailure);
                        }
                    }

                }

                Statistics::PeriodicStatistics::PeriodicStatistics(Statistics &statistics) : statistics_(statistics) {}

                std::string Statistics::escapeSpecialCharacters(const std::string &name) {
                    std::regex reComma(",");
                    std::string escapedName = std::regex_replace(name, reComma, std::string("\\,"));
                    std::regex reEqual("=");
                    escapedName = std::regex_replace(escapedName, reEqual, std::string("\\="));
                    std::regex reBackslash("\\");
                    escapedName = std::regex_replace(escapedName, reBackslash, std::string("\\\\"));

                    return name[0] == '/' ? escapedName.substr(1) : escapedName;
                }

                void
                Statistics::PeriodicStatistics::getNameWithPrefix(const std::string &name, std::ostringstream &out) {
                    out << NEAR_CACHE_CATEGORY_PREFIX << Statistics::escapeSpecialCharacters(name);
                }

                template<>
                void Statistics::PeriodicStatistics::addStat(std::ostringstream &stats, const std::string &name,
                                                             const bool &value) {
                    stats << STAT_SEPARATOR << name << KEY_VALUE_SEPARATOR << (value ? "true" : "false");
                }

            }
        }

        namespace monitor {
            const int64_t LocalInstanceStats::STAT_NOT_AVAILABLE = -99L;

            namespace impl {
                LocalMapStatsImpl::LocalMapStatsImpl() = default;

                LocalMapStatsImpl::LocalMapStatsImpl(const std::shared_ptr<monitor::NearCacheStats> &s) : near_cache_stats_(s) {}

                std::shared_ptr<monitor::NearCacheStats> LocalMapStatsImpl::getNearCacheStats() const {
                    return near_cache_stats_;
                }

                NearCacheStatsImpl::NearCacheStatsImpl() : creation_time_(util::currentTimeMillis()),
                                                           owned_entry_count_(0),
                                                           owned_entry_memory_cost_(0),
                                                           hits_(0),
                                                           misses_(0),
                                                           evictions_(0),
                                                           expirations_(0),
                                                           invalidations_(0),
                                                           invalidation_requests_(0),
                                                           persistence_count_(0),
                                                           last_persistence_time_(0),
                                                           last_persistence_duration_(0),
                                                           last_persistence_written_bytes_(0),
                                                           last_persistence_key_count_(0),
                                                           last_persistence_failure_("") {
                }

                int64_t NearCacheStatsImpl::getCreationTime() {
                    return creation_time_;
                }

                int64_t NearCacheStatsImpl::getOwnedEntryCount() {
                    return owned_entry_count_;
                }

                void NearCacheStatsImpl::setOwnedEntryCount(int64_t ownedEntryCount) {
                    this->owned_entry_count_ = ownedEntryCount;
                }

                void NearCacheStatsImpl::incrementOwnedEntryCount() {
                    ++owned_entry_count_;
                }

                void NearCacheStatsImpl::decrementOwnedEntryCount() {
                    --owned_entry_count_;
                }

                int64_t NearCacheStatsImpl::getOwnedEntryMemoryCost() {
                    return owned_entry_memory_cost_;
                }

                void NearCacheStatsImpl::setOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost) {
                    this->owned_entry_memory_cost_ = ownedEntryMemoryCost;
                }

                void NearCacheStatsImpl::incrementOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost) {
                    this->owned_entry_memory_cost_ += ownedEntryMemoryCost;
                }

                void NearCacheStatsImpl::decrementOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost) {
                    this->owned_entry_memory_cost_ -= ownedEntryMemoryCost;
                }

                int64_t NearCacheStatsImpl::getHits() {
                    return hits_;
                }

                // just for testing
                void NearCacheStatsImpl::setHits(int64_t hits) {
                    this->hits_ = hits;
                }

                void NearCacheStatsImpl::incrementHits() {
                    ++hits_;
                }

                int64_t NearCacheStatsImpl::getMisses() {
                    return misses_;
                }

                // just for testing
                void NearCacheStatsImpl::setMisses(int64_t misses) {
                    this->misses_ = misses;
                }

                void NearCacheStatsImpl::incrementMisses() {
                    ++misses_;
                }

                double NearCacheStatsImpl::getRatio() {
                    if (misses_ == (int64_t) 0) {
                        if (hits_ == (int64_t) 0) {
                            return std::numeric_limits<double>::signaling_NaN();
                        } else {
                            return std::numeric_limits<double>::infinity();
                        }
                    } else {
                        return ((double) hits_ / misses_) * PERCENTAGE;
                    }
                }

                int64_t NearCacheStatsImpl::getEvictions() {
                    return evictions_;
                }

                void NearCacheStatsImpl::incrementEvictions() {
                    ++evictions_;
                }

                int64_t NearCacheStatsImpl::getExpirations() {
                    return expirations_;
                }

                void NearCacheStatsImpl::incrementExpirations() {
                    ++expirations_;
                }

                int64_t NearCacheStatsImpl::getInvalidations() {
                    return invalidations_.load();
                }

                void NearCacheStatsImpl::incrementInvalidations() {
                    ++invalidations_;
                }

                int64_t NearCacheStatsImpl::getInvalidationRequests() {
                    return invalidation_requests_.load();
                }

                void NearCacheStatsImpl::incrementInvalidationRequests() {
                    ++invalidation_requests_;
                }

                void NearCacheStatsImpl::resetInvalidationEvents() {
                    invalidation_requests_ = 0;
                }

                int64_t NearCacheStatsImpl::getPersistenceCount() {
                    return persistence_count_;
                }

                void NearCacheStatsImpl::addPersistence(int64_t duration, int32_t writtenBytes, int32_t keyCount) {
                    ++persistence_count_;
                    last_persistence_time_ = util::currentTimeMillis();
                    last_persistence_duration_ = duration;
                    last_persistence_written_bytes_ = writtenBytes;
                    last_persistence_key_count_ = keyCount;
                    last_persistence_failure_ = "";
                }

                int64_t NearCacheStatsImpl::getLastPersistenceTime() {
                    return last_persistence_time_;
                }

                int64_t NearCacheStatsImpl::getLastPersistenceDuration() {
                    return last_persistence_duration_;
                }

                int64_t NearCacheStatsImpl::getLastPersistenceWrittenBytes() {
                    return last_persistence_written_bytes_;
                }

                int64_t NearCacheStatsImpl::getLastPersistenceKeyCount() {
                    return last_persistence_key_count_;
                }

                std::string NearCacheStatsImpl::getLastPersistenceFailure() {
                    return last_persistence_failure_;
                }

                std::string NearCacheStatsImpl::toString() {
                    std::ostringstream out;
                    std::string failureString = last_persistence_failure_;
                    out << "NearCacheStatsImpl{"
                        << "ownedEntryCount=" << owned_entry_count_
                        << ", ownedEntryMemoryCost=" << owned_entry_memory_cost_
                        << ", creationTime=" << creation_time_
                        << ", hits=" << hits_
                        << ", misses=" << misses_
                        << ", ratio=" << std::setprecision(1) << getRatio()
                        << ", evictions=" << evictions_
                        << ", expirations=" << expirations_
                        << ", invalidations=" << invalidations_.load()
                        << ", invalidationRequests=" << invalidation_requests_.load()
                        << ", lastPersistenceTime=" << last_persistence_time_
                        << ", persistenceCount=" << persistence_count_
                        << ", lastPersistenceDuration=" << last_persistence_duration_
                        << ", lastPersistenceWrittenBytes=" << last_persistence_written_bytes_
                        << ", lastPersistenceKeyCount=" << last_persistence_key_count_
                        << ", lastPersistenceFailure='" << failureString << "'"
                        << '}';

                    return out.str();
                }

                const double NearCacheStatsImpl::PERCENTAGE = 100.0;
            }
        }
    }
}
