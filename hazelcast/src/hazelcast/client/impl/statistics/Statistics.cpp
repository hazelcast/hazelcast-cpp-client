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

#include <regex>

#include <hazelcast/client/impl/statistics/Statistics.h>

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/impl/BuildInfo.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/protocol/codec/ClientStatisticsCodec.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/LifecycleService.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            namespace statistics {
                const std::string Statistics::NEAR_CACHE_CATEGORY_PREFIX("nc.");
                const std::string Statistics::FEATURE_SUPPORTED_SINCE_VERSION_STRING("3.9");
                const int Statistics::FEATURE_SUPPORTED_SINCE_VERSION = impl::BuildInfo::calculateVersion(
                        FEATURE_SUPPORTED_SINCE_VERSION_STRING);

                Statistics::Statistics(spi::ClientContext &clientContext) : clientContext(clientContext),
                                                                            clientProperties(
                                                                                    clientContext.getClientProperties()),
                                                                            logger(clientContext.getLogger()),
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
                        logger.warning() << "Provided client statistics "
                                         << clientProperties.getStatisticsPeriodSeconds().getName()
                                         << " cannot be less than or equal to 0. You provided " << periodSeconds
                                         << " seconds as the configuration. Client will use the default value of "
                                         << defaultValue << " instead.";
                        periodSeconds = defaultValue;
                    }

                    schedulePeriodicStatisticsSendTask(periodSeconds);

                    logger.info() << "Client statistics is enabled with period " << periodSeconds << " seconds.";

                }

                void Statistics::schedulePeriodicStatisticsSendTask(int64_t periodSeconds) {
                    clientContext.getClientExecutionService().scheduleWithRepetition(
                            std::shared_ptr<util::Runnable>(new CollectStatisticsTask(*this)), 0,
                            periodSeconds * MILLIS_IN_A_SECOND);
                }

                std::shared_ptr<connection::Connection> Statistics::getOwnerConnection() {
                    connection::ClientConnectionManagerImpl &connectionManager = clientContext.getConnectionManager();
                    std::shared_ptr<connection::Connection> connection = connectionManager.getOwnerConnection();
                    if (NULL == connection.get()) {
                        return std::shared_ptr<connection::Connection>();
                    }

                    std::shared_ptr<Address> currentOwnerAddress = connectionManager.getOwnerConnectionAddress();
                    int serverVersion = connection->getConnectedServerVersion();
                    if (serverVersion < FEATURE_SUPPORTED_SINCE_VERSION) {
                        // do not print too many logs if connected to an old version server
                        if (!isSameWithCachedOwnerAddress(currentOwnerAddress)) {
                            if (logger.isFinestEnabled()) {
                                logger.finest() << "Client statistics cannot be sent to server "
                                                << *currentOwnerAddress
                                                << " since, connected owner server version is less than the minimum supported server version "
                                                << FEATURE_SUPPORTED_SINCE_VERSION_STRING;
                            }
                        }

                        // cache the last connected server address for decreasing the log prints
                        cachedOwnerAddress = currentOwnerAddress;
                        return std::shared_ptr<connection::Connection>();
                    }

                    return connection;
                }

                bool Statistics::isSameWithCachedOwnerAddress(const std::shared_ptr<Address> &currentOwnerAddress) {
                    const std::shared_ptr<Address> cachedAddress = cachedOwnerAddress.get();
                    if (NULL == cachedAddress.get() && NULL == currentOwnerAddress.get()) {
                        return true;
                    }
                    return cachedAddress.get() && currentOwnerAddress.get() && *currentOwnerAddress == *cachedAddress;
                }

                void Statistics::sendStats(const std::string &newStats,
                                           const std::shared_ptr<connection::Connection> &ownerConnection) {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ClientStatisticsCodec::encodeRequest(
                            newStats);
                    try {
                        spi::impl::ClientInvocation::create(clientContext, request, "", ownerConnection)->invoke();
                    } catch (exception::IException &e) {
                        // suppress exception, do not print too many messages
                        if (logger.isFinestEnabled()) {
                            logger.finest() << "Could not send stats " << e;
                        }
                    }
                }

                const std::string Statistics::CollectStatisticsTask::getName() const {
                    return std::string();
                }

                void Statistics::CollectStatisticsTask::run() {
                    if (!statistics.clientContext.getLifecycleService().isRunning()) {
                        return;
                    }

                    std::shared_ptr<connection::Connection> ownerConnection = statistics.getOwnerConnection();
                    if (NULL == ownerConnection.get()) {
                        statistics.logger.finest()
                                << "Cannot send client statistics to the server. No owner connection.";
                        return;
                    }

                    std::ostringstream stats;

                    statistics.periodicStats.fillMetrics(stats, ownerConnection);

                    statistics.periodicStats.addNearCacheStats(stats);

                    statistics.sendStats(stats.str(), ownerConnection);
                }

                Statistics::CollectStatisticsTask::CollectStatisticsTask(Statistics &statistics) : statistics(
                        statistics) {}

                void Statistics::PeriodicStatistics::fillMetrics(std::ostringstream &stats,
                                                                 const std::shared_ptr<connection::Connection> &ownerConnection) {
                    stats << "lastStatisticsCollectionTime" << KEY_VALUE_SEPARATOR << util::currentTimeMillis();
                    addStat(stats, "enterprise", false);
                    addStat(stats, "clientType", protocol::ClientTypes::CPP);
                    addStat(stats, "clientVersion", HAZELCAST_VERSION);
                    addStat(stats, "clusterConnectionTimestamp", ownerConnection->getStartTimeInMillis());

                    std::unique_ptr<Address> localSocketAddress = ownerConnection->getLocalSocketAddress();
                    stats << STAT_SEPARATOR << "clientAddress" << KEY_VALUE_SEPARATOR;
                    if (localSocketAddress.get()) {
                        stats << localSocketAddress->getHost() << ":" << localSocketAddress->getPort();
                    }

                    addStat(stats, "clientName", statistics.clientContext.getName());

                    const Credentials *credentials = statistics.clientContext.getClientConfig().getCredentials();
                    if (credentials != NULL) {
                        addStat(stats, "credentials.principal", credentials->getPrincipal());
                    }

                }

                void Statistics::PeriodicStatistics::addNearCacheStats(std::ostringstream &stats) {
                    for (const std::shared_ptr<internal::nearcache::BaseNearCache> &nearCache : statistics.clientContext.getNearCacheManager().listAllNearCaches()) {
                                    std::string nearCacheName = nearCache->getName();
                                    std::ostringstream nearCacheNameWithPrefix;
                                    getNameWithPrefix(nearCacheName, nearCacheNameWithPrefix);

                                    nearCacheNameWithPrefix << '.';

                                    monitor::impl::NearCacheStatsImpl &nearCacheStats = static_cast<monitor::impl::NearCacheStatsImpl &>(nearCache->getNearCacheStats());

                                    std::string prefix = nearCacheNameWithPrefix.str();

                                    addStat(stats, prefix, "creationTime", nearCacheStats.getCreationTime());
                                    addStat(stats, prefix, "evictions", nearCacheStats.getEvictions());
                                    addStat(stats, prefix, "hits", nearCacheStats.getHits());
                                    addStat(stats, prefix, "lastPersistenceDuration",
                                            nearCacheStats.getLastPersistenceDuration());
                                    addStat(stats, prefix, "lastPersistenceKeyCount",
                                            nearCacheStats.getLastPersistenceKeyCount());
                                    addStat(stats, prefix, "lastPersistenceTime",
                                            nearCacheStats.getLastPersistenceTime());
                                    addStat(stats, prefix, "lastPersistenceWrittenBytes",
                                            nearCacheStats.getLastPersistenceWrittenBytes());
                                    addStat(stats, prefix, "misses", nearCacheStats.getMisses());
                                    addStat(stats, prefix, "ownedEntryCount", nearCacheStats.getOwnedEntryCount());
                                    addStat(stats, prefix, "expirations", nearCacheStats.getExpirations());
                                    addStat(stats, prefix, "invalidations", nearCacheStats.getInvalidations());
                                    addStat(stats, prefix, "invalidationRequests",
                                            nearCacheStats.getInvalidationRequests());
                                    addStat(stats, prefix, "ownedEntryMemoryCost",
                                            nearCacheStats.getOwnedEntryMemoryCost());
                                    std::string persistenceFailure = nearCacheStats.getLastPersistenceFailure();
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
                    std::regex reBackslash("\\");
                    escapedName = std::regex_replace(escapedName, reBackslash, std::string("\\\\"));

                    return name[0] == '/' ? escapedName.substr(1) : escapedName;
                }

                void Statistics::PeriodicStatistics::getNameWithPrefix(const std::string &name, std::ostringstream &out) {
                    out << NEAR_CACHE_CATEGORY_PREFIX << Statistics::escapeSpecialCharacters(name);
                }

                template<>
                void Statistics::PeriodicStatistics::addStat(std::ostringstream &stats, const std::string &name,
                                                             const bool &value) {
                    stats << STAT_SEPARATOR << name << KEY_VALUE_SEPARATOR << (value ? "true" : "false");
                }

            }
        }
    }
}
