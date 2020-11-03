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

                Statistics::Statistics(spi::ClientContext &clientContext) : clientContext(clientContext),
                                                                            clientProperties(
                                                                                    clientContext.getClientProperties()),
                                                                            logger_(clientContext.getLogger()),
                                                                            periodicStats(*this) {
                    this->enabled = clientProperties.getBoolean(clientProperties.getStatisticsEnabled());
                }

                void Statistics::start() {
                    if (!enabled) {
                        return;
                    }

                    int64_t periodSeconds = clientProperties.getLong(clientProperties.getStatisticsPeriodSeconds());
                    if (periodSeconds <= 0) {

                        int64_t defaultValue = util::IOUtil::to_value<int64_t>(
                                clientProperties.getStatisticsPeriodSeconds().getDefaultValue());
                        HZ_LOG(logger_, warning,
                            boost::str(boost::format(
                                "Provided client statistics %1% cannot be less than or equal to 0. "
                                "You provided %2% seconds as the configuration. "
                                "Client will use the default value of %3% instead.")
                                % clientProperties.getStatisticsPeriodSeconds().getName()
                                % periodSeconds % defaultValue)
                        );
                        periodSeconds = defaultValue;
                    }

                    schedulePeriodicStatisticsSendTask(periodSeconds);

                    HZ_LOG(logger_, info,
                        boost::str(boost::format("Client statistics is enabled with period %1% seconds.")
                                                 % periodSeconds)
                    );
                }

                void Statistics::shutdown() {
                    if (sendTaskTimer) {
                        boost::system::error_code ignored;
                        sendTaskTimer->cancel(ignored);
                    }
                }

                void Statistics::schedulePeriodicStatisticsSendTask(int64_t periodSeconds) {
                    sendTaskTimer = clientContext.getClientExecutionService().scheduleWithRepetition([=]() {
                        if (!clientContext.getLifecycleService().isRunning()) {
                            return;
                        }

                        auto collection_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch()).count();
                        std::shared_ptr<connection::Connection> connection = getConnection();
                        if (!connection) {
                            HZ_LOG(logger_, finest, "Cannot send client statistics to the server. No connection found.");
                            return;
                        }

                        std::ostringstream stats;

                        periodicStats.fillMetrics(stats, connection);

                        periodicStats.addNearCacheStats(stats);

                        sendStats(collection_timestamp, stats.str(), connection);
                    }, std::chrono::seconds(0), std::chrono::seconds(periodSeconds));
                }

                std::shared_ptr<connection::Connection> Statistics::getConnection() {
                    return clientContext.getConnectionManager().get_random_connection();
                }

                void Statistics::sendStats(int64_t timestamp, const std::string &newStats,
                                           const std::shared_ptr<connection::Connection> &connection) {
                    // TODO: implement metrics blob
                    auto request = protocol::codec::client_statistics_encode(timestamp, newStats, std::vector<byte>());
                    try {
                        spi::impl::ClientInvocation::create(clientContext, request, "", connection)->invoke().get();
                    } catch (exception::IException &e) {
                        // suppress exception, do not print too many messages
                        HZ_LOG(logger_, finest,
                            boost::str(boost::format("Could not send stats %1%") % e)
                        );
                    }
                }

                void Statistics::PeriodicStatistics::fillMetrics(std::ostringstream &stats,
                                                                 const std::shared_ptr<connection::Connection> &connection) {
                    stats << "lastStatisticsCollectionTime" << KEY_VALUE_SEPARATOR << util::currentTimeMillis();
                    addStat(stats, "enterprise", false);
                    addStat(stats, "clientType", protocol::ClientTypes::CPP);
                    addStat(stats, "clientVersion", HAZELCAST_VERSION);
                    addStat(stats, "clusterConnectionTimestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                            connection->getStartTime().time_since_epoch()).count());

                    auto localSocketAddress = connection->getLocalSocketAddress();
                    stats << STAT_SEPARATOR << "clientAddress" << KEY_VALUE_SEPARATOR;
                    if (localSocketAddress) {
                        stats << localSocketAddress->getHost() << ":" << localSocketAddress->getPort();
                    }

                    addStat(stats, "clientName", statistics.clientContext.getName());

                    auto credential = statistics.clientContext.getClientConfig().getCredentials();
                    if (credential) {
                        addStat(stats, "credentials.principal", credential->name());
                    }
                }

                void Statistics::PeriodicStatistics::addNearCacheStats(std::ostringstream &stats) {
                    for (const std::shared_ptr<internal::nearcache::BaseNearCache> &nearCache : statistics.clientContext.getNearCacheManager().listAllNearCaches()) {
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

                Statistics::PeriodicStatistics::PeriodicStatistics(Statistics &statistics) : statistics(statistics) {}

                std::string Statistics::escapeSpecialCharacters(const std::string &name) {
                    std::regex reComma(",");
                    std::string escapedName = std::regex_replace(name, reComma, std::string("\\,"));
                    std::regex reEqual("=");
                    escapedName = std::regex_replace(escapedName, reEqual, std::string("\\="));
                    std::regex reBackslash("\\\\");
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

                LocalMapStatsImpl::LocalMapStatsImpl(const std::shared_ptr<monitor::NearCacheStats> &s) : nearCacheStats(s) {}

                std::shared_ptr<monitor::NearCacheStats> LocalMapStatsImpl::getNearCacheStats() const {
                    return nearCacheStats;
                }

                NearCacheStatsImpl::NearCacheStatsImpl() : creationTime(util::currentTimeMillis()),
                                                           ownedEntryCount(0),
                                                           ownedEntryMemoryCost(0),
                                                           hits(0),
                                                           misses(0),
                                                           evictions(0),
                                                           expirations(0),
                                                           invalidations(0),
                                                           invalidationRequests(0),
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

                int64_t NearCacheStatsImpl::getInvalidations() {
                    return invalidations.load();
                }

                void NearCacheStatsImpl::incrementInvalidations() {
                    ++invalidations;
                }

                int64_t NearCacheStatsImpl::getInvalidationRequests() {
                    return invalidationRequests.load();
                }

                void NearCacheStatsImpl::incrementInvalidationRequests() {
                    ++invalidationRequests;
                }

                void NearCacheStatsImpl::resetInvalidationEvents() {
                    invalidationRequests = 0;
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
                        << ", invalidations=" << invalidations.load()
                        << ", invalidationRequests=" << invalidationRequests.load()
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
