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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <winsock2.h>
#endif

#include <algorithm>
#include <boost/asio.hpp>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <cctype>
#include <chrono>
#include <climits>
#include <clocale>
#include <condition_variable>
#include <ctime>
#include <functional>
#include <limits>
#include <random>
#include <regex>
#include <stdint.h>
#include <utility>

#ifdef HZ_BUILD_WITH_SSL

#include <openssl/ssl.h>

#endif // HZ_BUILD_WITH_SSL

#include "hazelcast/client/config/SSLConfig.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/impl/BuildInfo.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/impl/RoundRobinLB.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/impl/MemberAttributeChange.h"
#include "hazelcast/client/impl/ClientMessageDecoder.h"
#include "hazelcast/client/impl/ClientLockReferenceIdGenerator.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/crdt/pncounter/impl/PNCounterProxyFactory.h"
#include "hazelcast/client/proxy/ClientPNCounterProxy.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"
#include "hazelcast/client/spi/impl/SmartClientInvocationService.h"
#include "hazelcast/client/spi/impl/NonSmartClientInvocationService.h"
#include "hazelcast/client/spi/impl/listener/NonSmartClientListenerService.h"
#include "hazelcast/client/spi/impl/listener/SmartClientListenerService.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/spi/impl/sequence/CallIdFactory.h"
#include "hazelcast/client/spi/impl/AwsAddressProvider.h"
#include "hazelcast/client/spi/impl/DefaultAddressProvider.h"
#include "hazelcast/client/aws/impl/AwsAddressTranslator.h"
#include "hazelcast/client/spi/impl/DefaultAddressTranslator.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/mixedtype/impl/HazelcastClientImpl.h"
#include "hazelcast/client/flakeidgen/impl/FlakeIdGeneratorProxyFactory.h"
#include "hazelcast/client/idgen/impl/IdGeneratorProxyFactory.h"
#include "hazelcast/client/proxy/ClientFlakeIdGeneratorProxy.h"
#include "hazelcast/client/proxy/ClientIdGeneratorProxy.h"
#include "hazelcast/client/proxy/ClientAtomicLongProxy.h"
#include "hazelcast/client/atomiclong/impl/AtomicLongProxyFactory.h"
#include "hazelcast/client/impl/AbstractLoadBalancer.h"
#include "hazelcast/util/LockGuard.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/cluster/impl/VectorClock.h"
#include "hazelcast/client/cluster/memberselector/MemberSelectors.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/monitor/impl/LocalMapStatsImpl.h"
#include "hazelcast/client/monitor/NearCacheStats.h"
#include "hazelcast/client/monitor/impl/NearCacheStatsImpl.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/IAtomicLong.h"
#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/flakeidgen/impl/AutoBatcher.h"
#include "hazelcast/client/flakeidgen/impl/IdBatch.h"
#include "hazelcast/client/txn/ClientTransactionUtil.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/TransactionOptions.h"
#include "hazelcast/client/proxy/ReliableTopicImpl.h"
#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"
#include "hazelcast/client/proxy/IListImpl.h"
#include "hazelcast/client/spi/ClientListenerService.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/proxy/TransactionalMapImpl.h"
#include "hazelcast/client/proxy/TransactionalMultiMapImpl.h"
#include "hazelcast/client/proxy/TransactionalListImpl.h"
#include "hazelcast/client/proxy/TransactionalSetImpl.h"
#include "hazelcast/client/proxy/TransactionalObject.h"
#include "hazelcast/client/spi/ClientInvocationService.h"
#include "hazelcast/client/spi/impl/ClientInvocationFuture.h"
#include "hazelcast/client/config/ClientFlakeIdGeneratorConfig.h"
#include "hazelcast/client/proxy/IQueueImpl.h"
#include "hazelcast/client/TypedData.h"
#include "hazelcast/client/spi/ClientPartitionService.h"
#include "hazelcast/util/ExceptionUtil.h"
#include "hazelcast/client/proxy/PartitionSpecificClientProxy.h"
#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"
#include "hazelcast/client/spi/InternalCompletableFuture.h"
#include "hazelcast/client/proxy/MultiMapImpl.h"
#include "hazelcast/client/impl/EntryEventHandler.h"
#include "hazelcast/client/proxy/IMapImpl.h"
#include "hazelcast/client/EntryView.h"
#include "hazelcast/util/TimeUtil.h"
#include "hazelcast/client/proxy/TransactionalQueueImpl.h"
#include "hazelcast/client/proxy/ISetImpl.h"
#include "hazelcast/client/proxy/ITopicImpl.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/connection/DefaultClientConnectionStrategy.h"
#include "hazelcast/client/connection/AddressProvider.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/client/protocol/AuthenticationStatus.h"
#include "hazelcast/client/exception/AuthenticationException.h"
#include "hazelcast/client/connection/ConnectionListener.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/Executor.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/connection/AuthenticationFuture.h"
#include "hazelcast/client/config/ClientNetworkConfig.h"
#include "hazelcast/client/connection/HeartbeatManager.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/internal/socket/TcpSocket.h"
#include "hazelcast/client/spi/impl/listener/AbstractClientListenerService.h"
#include "hazelcast/client/connection/ClientConnectionStrategy.h"
#include "hazelcast/client/config/ClientConnectionStrategyConfig.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/MapEvent.h"
#include "hazelcast/client/Endpoint.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/config/LoggerConfig.h"
#include "hazelcast/client/config/matcher/MatchingPointConfigPatternMatcher.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/config/ReliableTopicConfig.h"
#include "hazelcast/client/config/SocketOptions.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/mixedtype/MultiMap.h"
#include "hazelcast/client/map/impl/MapMixedTypeProxyFactory.h"
#include "hazelcast/client/mixedtype/IQueue.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/mixedtype/IMap.h"
#include "hazelcast/client/mixedtype/Ringbuffer.h"
#include "hazelcast/client/mixedtype/ITopic.h"
#include "hazelcast/client/mixedtype/ISet.h"
#include "hazelcast/client/mixedtype/NearCachedClientMapProxy.h"
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/map/impl/nearcache/InvalidationAwareWrapper.h"
#include "hazelcast/client/map/impl/nearcache/KeyStateMarker.h"
#include "hazelcast/client/internal/nearcache/impl/KeyStateMarkerImpl.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/mixedtype/IList.h"
#include "hazelcast/client/mixedtype/ClientMapProxy.h"
#include "hazelcast/util/AddressUtil.h"
#include "hazelcast/client/cluster/impl/ClusterDataSerializerHook.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/MembershipEvent.h"
#include "hazelcast/client/spi/impl/ClientTransactionManagerServiceImpl.h"
#include "hazelcast/client/serialization/ClassDefinitionBuilder.h"
#include "hazelcast/client/serialization/FieldDefinition.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/client/HazelcastJsonValue.h"
#include "hazelcast/client/serialization/FieldType.h"
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/util/MurmurHash3.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableReader.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/client/serialization/pimpl/ConstantSerializers.h"
#include "hazelcast/client/serialization/pimpl/MorphingPortableReader.h"
#include "hazelcast/client/serialization/pimpl/PortableVersionHelper.h"
#include "hazelcast/client/serialization/VersionedPortable.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionContext.h"
#include "hazelcast/util/UTFUtil.h"
#include "hazelcast/client/serialization/pimpl/DataSerializer.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/serialization/pimpl/PortableReaderBase.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableWriter.h"
#include "hazelcast/client/serialization/pimpl/PortableSerializer.h"
#include "hazelcast/client/serialization/TypeIDS.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/LoadBalancer.h"
#include "hazelcast/client/internal/nearcache/impl/record/NearCacheDataRecord.h"
#include "hazelcast/util/HashUtil.h"
#include "hazelcast/client/internal/socket/SocketFactory.h"
#include "hazelcast/client/internal/eviction/EvictionChecker.h"
#include "hazelcast/client/Client.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicMessage.h"
#include "hazelcast/client/topic/impl/TopicDataSerializerHook.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicExecutor.h"
#include "hazelcast/client/proxy/ClientRingbufferProxy.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/protocol/ClientExceptionFactory.h"
#include "hazelcast/client/protocol/codec/StackTraceElement.h"
#include "hazelcast/client/protocol/codec/AddressCodec.h"
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/client/protocol/codec/MemberCodec.h"
#include "hazelcast/client/protocol/codec/UUIDCodec.h"
#include "hazelcast/client/protocol/codec/StackTraceElementCodec.h"
#include "hazelcast/client/protocol/codec/DataEntryViewCodec.h"
#include "hazelcast/client/protocol/Principal.h"
#include "hazelcast/client/protocol/ClientMessageBuilder.h"
#include "hazelcast/client/protocol/IMessageHandler.h"
#include "hazelcast/client/ItemEvent.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/spi/impl/AbstractClientInvocationService.h"
#include "hazelcast/util/UuidUtil.h"
#include "hazelcast/client/spi/impl/ListenerMessageCodec.h"
#include "hazelcast/client/spi/impl/listener/ClientEventRegistration.h"
#include "hazelcast/client/spi/impl/listener/ClientRegistrationKey.h"
#include "hazelcast/util/Callable.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/spi/impl/ClientMembershipListener.h"
#include "hazelcast/util/RuntimeAvailableProcessors.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequence.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequenceWithBackpressure.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequenceWithoutBackpressure.h"
#include "hazelcast/client/spi/impl/sequence/FailFastCallIdSequence.h"
#include "hazelcast/client/spi/impl/sequence/AbstractCallIdSequence.h"
#include "hazelcast/util/concurrent/BackoffIdleStrategy.h"
#include "hazelcast/util/Runnable.h"
#include "hazelcast/client/exception/IException.h"
#include "hazelcast/util/AddressHelper.h"
#include "hazelcast/client/spi/ProxyManager.h"
#include "hazelcast/client/spi/ClientProxy.h"
#include "hazelcast/client/spi/ClientProxyFactory.h"
#include "hazelcast/client/impl/statistics/Statistics.h"
#include "hazelcast/client/spi/DefaultObjectNamespace.h"
#include "hazelcast/client/FlakeIdGenerator.h"
#include "hazelcast/client/executor/impl/ExecutorServiceProxyFactory.h"
#include "hazelcast/client/IExecutorService.h"
#include "hazelcast/client/aws/impl/Filter.h"
#include "hazelcast/client/aws/impl/DescribeInstances.h"
#include "hazelcast/client/aws/impl/Constants.h"
#include "hazelcast/client/aws/utility/CloudUtility.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/util/SyncHttpClient.h"
#include "hazelcast/client/aws/utility/AwsURLEncoder.h"
#include "hazelcast/client/aws/security/EC2RequestSigner.h"
#include "hazelcast/client/aws/AWSClient.h"
#include "hazelcast/client/query/InstanceOfPredicate.h"
#include "hazelcast/client/query/impl/predicates/PredicateDataSerializerHook.h"
#include "hazelcast/client/query/ILikePredicate.h"
#include "hazelcast/client/query/TruePredicate.h"
#include "hazelcast/client/query/OrPredicate.h"
#include "hazelcast/client/query/NotPredicate.h"
#include "hazelcast/client/query/QueryConstants.h"
#include "hazelcast/client/query/SqlPredicate.h"
#include "hazelcast/client/query/FalsePredicate.h"
#include "hazelcast/client/query/RegexPredicate.h"
#include "hazelcast/client/query/AndPredicate.h"
#include "hazelcast/client/query/LikePredicate.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/internal/config/ConfigUtils.h"
#include "hazelcast/client/impl/statistics/Statistics.h"
#include "hazelcast/client/executor/impl/ExecutorServiceProxyFactory.h"
#include "hazelcast/client/flakeidgen/impl/IdBatch.h"
#include "hazelcast/client/txn/ClientTransactionUtil.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/impl/ClientInvocationFuture.h"
#include "hazelcast/client/spi/ClientProxy.h"
#include "hazelcast/client/IExecutorService.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4355) //for strerror
#pragma warning(disable: 4996) //for strerror
#endif

INITIALIZE_EASYLOGGINGPP

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
                        logger.warning("Provided client statistics ",
                                       clientProperties.getStatisticsPeriodSeconds().getName(),
                                       " cannot be less than or equal to 0. You provided ", periodSeconds,
                                       " seconds as the configuration. Client will use the default value of ",
                                       defaultValue, " instead.");
                        periodSeconds = defaultValue;
                    }

                    schedulePeriodicStatisticsSendTask(periodSeconds);

                    logger.info("Client statistics is enabled with period ", periodSeconds, " seconds.");

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
                                logger.finest("Client statistics cannot be sent to server ", *currentOwnerAddress,
                                              " since, connected owner server version is less than the minimum supported server version ",
                                              FEATURE_SUPPORTED_SINCE_VERSION_STRING);
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
                            logger.finest("Could not send stats ", e);
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
                        statistics.logger.finest("Cannot send client statistics to the server. No owner connection.");
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
    }
}

namespace hazelcast {
    namespace client {
        namespace impl {
            RoundRobinLB::RoundRobinLB() : index(0) {

            }

            void RoundRobinLB::init(Cluster &cluster) {
                AbstractLoadBalancer::init(cluster);
            }

            const Member RoundRobinLB::next() {
                std::vector<Member> members = getMembers();
                if (members.size() == 0) {
                    throw exception::IllegalStateException("const Member& RoundRobinLB::next()",
                                                           "No member in member list!!");
                }
                return members[++index % members.size()];
            }

            RoundRobinLB::RoundRobinLB(const RoundRobinLB &rhs) : index(const_cast<RoundRobinLB &>(rhs).index.load()) {
            }

            void RoundRobinLB::operator=(const RoundRobinLB &rhs) {
                index.store(const_cast<RoundRobinLB &>(rhs).index.load());
            }

        }
    }
}


namespace hazelcast {
    namespace client {
        namespace impl {

            MemberAttributeChange::MemberAttributeChange() {

            }

            MemberAttributeChange::MemberAttributeChange(std::unique_ptr<std::string> &uuid,
                                                         MemberAttributeEvent::MemberAttributeOperationType const &operationType,
                                                         std::unique_ptr<std::string> &key,
                                                         std::unique_ptr<std::string> &value)
                    : uuid(std::move(uuid)),
                      operationType(operationType),
                      key(std::move(key)),
                      value(std::move(value)) {
            }

            const std::string &MemberAttributeChange::getUuid() const {
                return *uuid;
            }

            MemberAttributeEvent::MemberAttributeOperationType MemberAttributeChange::getOperationType() const {
                return operationType;
            }

            const std::string &MemberAttributeChange::getKey() const {
                return *key;
            }

            const std::string &MemberAttributeChange::getValue() const {
                return *value;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace impl {
            std::shared_ptr<void>
            VoidMessageDecoder::decodeClientMessage(const std::shared_ptr<protocol::ClientMessage> &clientMessage,
                                                    serialization::pimpl::SerializationService &serializationService) {
                return std::shared_ptr<void>();
            }

            const std::shared_ptr<ClientMessageDecoder<void> > &VoidMessageDecoder::instance() {
                static std::shared_ptr<ClientMessageDecoder<void> > singleton(new VoidMessageDecoder);
                return singleton;
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace impl {
            ClientLockReferenceIdGenerator::ClientLockReferenceIdGenerator() : referenceIdCounter(0) {}

            int64_t ClientLockReferenceIdGenerator::getNextReferenceId() {
                return ++referenceIdCounter;
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace impl {
            BaseEventHandler::~BaseEventHandler() {
            }

            void BaseEventHandler::handle(const std::shared_ptr<protocol::ClientMessage> &event) {
                std::unique_ptr<protocol::ClientMessage> e(new protocol::ClientMessage(*event));
                handle(std::move(e));
            }

            BaseEventHandler::BaseEventHandler() : logger(NULL) {}

            void BaseEventHandler::setLogger(util::ILogger *logger) {
                BaseEventHandler::logger = logger;
            }

            util::ILogger *BaseEventHandler::getLogger() const {
                return logger;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace impl {
            int BuildInfo::calculateVersion(const std::string &version) {
                std::vector<std::string> versionTokens = util::StringUtil::tokenizeVersionString(version);
                if (!versionTokens.empty()) {
                    int calculatedVersion = MAJOR_VERSION_MULTIPLIER * util::IOUtil::to_value<int>(versionTokens[0])
                                            + MINOR_VERSION_MULTIPLIER *
                                              util::IOUtil::to_value<int>(versionTokens[1]);
                    if (versionTokens.size() > PATCH_TOKEN_INDEX) {
                        size_t snapshotStartPosition = versionTokens[PATCH_TOKEN_INDEX].find("-");
                        if (snapshotStartPosition == std::string::npos) {
                            calculatedVersion += util::IOUtil::to_value<int>(versionTokens[PATCH_TOKEN_INDEX]);
                        }

                        if (snapshotStartPosition > 0) {
                            calculatedVersion += util::IOUtil::to_value<int>(
                                    versionTokens[PATCH_TOKEN_INDEX].substr(0, snapshotStartPosition));
                        }
                    }
                    return calculatedVersion;
                }

                return UNKNOWN_HAZELCAST_VERSION;
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace impl {
            std::atomic<int32_t> HazelcastClientInstanceImpl::CLIENT_ID(0);

            HazelcastClientInstanceImpl::HazelcastClientInstanceImpl(const ClientConfig &config)
                    : clientConfig(config), clientProperties(const_cast<ClientConfig &>(config).getProperties()),
                      clientContext(*this),
                      serializationService(clientConfig.getSerializationConfig()), clusterService(clientContext),
                      transactionManager(clientContext, *clientConfig.getLoadBalancer()), cluster(clusterService),
                      lifecycleService(clientContext, clientConfig.getLifecycleListeners(),
                                       clientConfig.getLoadBalancer(), cluster), proxyManager(clientContext),
                      id(++CLIENT_ID), TOPIC_RB_PREFIX("_hz_rb_") {
                const std::shared_ptr<std::string> &name = clientConfig.getInstanceName();
                if (name.get() != NULL) {
                    instanceName = *name;
                } else {
                    std::ostringstream out;
                    out << "hz.client_" << id;
                    instanceName = out.str();
                }

                logger.reset(new util::ILogger(instanceName, clientConfig.getGroupConfig().getName(), HAZELCAST_VERSION,
                                               clientConfig.getLoggerConfig()));

                initalizeNearCacheManager();

                executionService = initExecutionService();

                int32_t maxAllowedConcurrentInvocations = clientProperties.getInteger(
                        clientProperties.getMaxConcurrentInvocations());
                int64_t backofftimeoutMs = clientProperties.getLong(
                        clientProperties.getBackpressureBackoffTimeoutMillis());
                bool isBackPressureEnabled = maxAllowedConcurrentInvocations != INT32_MAX;
                callIdSequence = spi::impl::sequence::CallIdFactory::newCallIdSequence(isBackPressureEnabled,
                                                                                       maxAllowedConcurrentInvocations,
                                                                                       backofftimeoutMs);

                std::vector<std::shared_ptr<connection::AddressProvider> > addressProviders = createAddressProviders();

                connectionManager = initConnectionManagerService(addressProviders);

                partitionService.reset(new spi::impl::ClientPartitionServiceImpl(clientContext, *executionService));

                invocationService = initInvocationService();
                listenerService = initListenerService();

                proxyManager.init();

                lockReferenceIdGenerator.reset(new impl::ClientLockReferenceIdGenerator());

                statistics.reset(new statistics::Statistics(clientContext));
            }

            HazelcastClientInstanceImpl::~HazelcastClientInstanceImpl() {
            }

            void HazelcastClientInstanceImpl::start() {
                startLogger();

                lifecycleService.fireLifecycleEvent(LifecycleEvent::STARTING);

                try {
                    if (!lifecycleService.start()) {
                        try {
                            lifecycleService.shutdown();
                        } catch (exception::IException &e) {
                            logger->info("Lifecycle service start failed. Exception during shutdown: ", e.what());
                            // ignore
                        }
                        throw exception::IllegalStateException("HazelcastClient",
                                                               "HazelcastClient could not be started!");
                    }
                } catch (exception::IException &) {
                    try {
                        lifecycleService.shutdown();
                    } catch (exception::IException &e) {
                        logger->info("Exception during shutdown: ", e.what());
                        // ignore
                    }
                    throw;
                }

                mixedTypeSupportAdaptor = std::make_unique<mixedtype::impl::HazelcastClientImpl>(*this);
            }

            void HazelcastClientInstanceImpl::startLogger() {
                try {
                    if (!logger->start()) {
                        throw (exception::ExceptionBuilder<exception::IllegalStateException>(
                                "HazelcastClientInstanceImpl::initLogger")
                                << "Could not start logger for instance " << instanceName).build();
                    }
                } catch (std::invalid_argument &ia) {
                    throw exception::IllegalStateException("HazelcastClientInstanceImpl::initLogger", ia.what());
                }
            }

            ClientConfig &HazelcastClientInstanceImpl::getClientConfig() {
                return clientConfig;
            }

            Cluster &HazelcastClientInstanceImpl::getCluster() {
                return cluster;
            }

            void HazelcastClientInstanceImpl::addLifecycleListener(LifecycleListener *lifecycleListener) {
                lifecycleService.addLifecycleListener(lifecycleListener);
            }

            bool HazelcastClientInstanceImpl::removeLifecycleListener(LifecycleListener *lifecycleListener) {
                return lifecycleService.removeLifecycleListener(lifecycleListener);
            }

            void HazelcastClientInstanceImpl::shutdown() {
                lifecycleService.shutdown();
            }

            IdGenerator HazelcastClientInstanceImpl::getIdGenerator(const std::string &name) {
                idgen::impl::IdGeneratorProxyFactory factory(&clientContext);
                std::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(proxy::ClientIdGeneratorProxy::SERVICE_NAME, name, factory);

                std::shared_ptr<impl::IdGeneratorInterface> impl = std::static_pointer_cast<proxy::ClientIdGeneratorProxy>(
                        proxy);

                return IdGenerator(impl);
            }

            IAtomicLong HazelcastClientInstanceImpl::getIAtomicLong(const std::string &name) {
                atomiclong::impl::AtomicLongProxyFactory factory(&clientContext);
                std::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(proxy::ClientAtomicLongProxy::SERVICE_NAME, name, factory);

                std::shared_ptr<proxy::ClientAtomicLongProxy> impl = std::static_pointer_cast<proxy::ClientAtomicLongProxy>(
                        proxy);

                return IAtomicLong(impl);
            }

            FlakeIdGenerator HazelcastClientInstanceImpl::getFlakeIdGenerator(const std::string &name) {
                flakeidgen::impl::FlakeIdGeneratorProxyFactory factory(&clientContext);
                std::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(proxy::ClientFlakeIdGeneratorProxy::SERVICE_NAME, name, factory);

                std::shared_ptr<impl::IdGeneratorInterface> impl = std::static_pointer_cast<proxy::ClientFlakeIdGeneratorProxy>(
                        proxy);

                return FlakeIdGenerator(impl);
            }

            ICountDownLatch HazelcastClientInstanceImpl::getICountDownLatch(const std::string &instanceName) {
                return getDistributedObject<ICountDownLatch>(instanceName);
            }

            ISemaphore HazelcastClientInstanceImpl::getISemaphore(const std::string &instanceName) {
                return getDistributedObject<ISemaphore>(instanceName);
            }

            ILock HazelcastClientInstanceImpl::getILock(const std::string &instanceName) {
                return getDistributedObject<ILock>(instanceName);
            }

            TransactionContext HazelcastClientInstanceImpl::newTransactionContext() {
                TransactionOptions defaultOptions;
                return newTransactionContext(defaultOptions);
            }

            TransactionContext HazelcastClientInstanceImpl::newTransactionContext(const TransactionOptions &options) {
                return TransactionContext(transactionManager, options);
            }

            internal::nearcache::NearCacheManager &HazelcastClientInstanceImpl::getNearCacheManager() {
                return *nearCacheManager;
            }

            serialization::pimpl::SerializationService &HazelcastClientInstanceImpl::getSerializationService() {
                return serializationService;
            }

            std::shared_ptr<spi::ClientProxy> HazelcastClientInstanceImpl::getDistributedObjectForService(
                    const std::string &serviceName,
                    const std::string &name,
                    spi::ClientProxyFactory &factory) {
                return proxyManager.getOrCreateProxy(serviceName, name, factory);
            }

            mixedtype::HazelcastClient &HazelcastClientInstanceImpl::toMixedType() const {
                return *mixedTypeSupportAdaptor;
            }

            const protocol::ClientExceptionFactory &HazelcastClientInstanceImpl::getExceptionFactory() const {
                return exceptionFactory;
            }

            std::shared_ptr<spi::ClientListenerService> HazelcastClientInstanceImpl::initListenerService() {
                int eventQueueCapacity = clientProperties.getInteger(clientProperties.getEventQueueCapacity());
                int eventThreadCount = clientProperties.getInteger(clientProperties.getEventThreadCount());
                config::ClientNetworkConfig &networkConfig = clientConfig.getNetworkConfig();
                if (networkConfig.isSmartRouting()) {
                    return std::shared_ptr<spi::ClientListenerService>(
                            new spi::impl::listener::SmartClientListenerService(clientContext, eventThreadCount,
                                                                                eventQueueCapacity));
                } else {
                    return std::shared_ptr<spi::ClientListenerService>(
                            new spi::impl::listener::NonSmartClientListenerService(clientContext, eventThreadCount,
                                                                                   eventQueueCapacity));
                }
            }

            std::unique_ptr<spi::ClientInvocationService> HazelcastClientInstanceImpl::initInvocationService() {
                if (clientConfig.getNetworkConfig().isSmartRouting()) {
                    return std::unique_ptr<spi::ClientInvocationService>(
                            new spi::impl::SmartClientInvocationService(clientContext));
                } else {
                    return std::unique_ptr<spi::ClientInvocationService>(
                            new spi::impl::NonSmartClientInvocationService(clientContext));
                }
            }

            std::shared_ptr<spi::impl::ClientExecutionServiceImpl> HazelcastClientInstanceImpl::initExecutionService() {
                return std::shared_ptr<spi::impl::ClientExecutionServiceImpl>(
                        new spi::impl::ClientExecutionServiceImpl(instanceName, clientProperties,
                                                                  clientConfig.getExecutorPoolSize(), *logger));
            }

            std::unique_ptr<connection::ClientConnectionManagerImpl>
            HazelcastClientInstanceImpl::initConnectionManagerService(
                    const std::vector<std::shared_ptr<connection::AddressProvider> > &addressProviders) {
                config::ClientAwsConfig &awsConfig = clientConfig.getNetworkConfig().getAwsConfig();
                std::shared_ptr<connection::AddressTranslator> addressTranslator;
                if (awsConfig.isEnabled()) {
                    try {
                        addressTranslator.reset(new aws::impl::AwsAddressTranslator(awsConfig, *logger));
                    } catch (exception::InvalidConfigurationException &e) {
                        logger->warning(std::string("Invalid aws configuration! ") + e.what());
                        throw;
                    }
                } else {
                    addressTranslator.reset(new spi::impl::DefaultAddressTranslator());
                }
                return std::unique_ptr<connection::ClientConnectionManagerImpl>(
                        new connection::ClientConnectionManagerImpl(
                                clientContext, addressTranslator, addressProviders));

            }

            void HazelcastClientInstanceImpl::onClusterConnect(
                    const std::shared_ptr<connection::Connection> &ownerConnection) {
                partitionService->listenPartitionTable(ownerConnection);
                clusterService.listenMembershipEvents(ownerConnection);
            }

            std::vector<std::shared_ptr<connection::AddressProvider> >
            HazelcastClientInstanceImpl::createAddressProviders() {
                config::ClientNetworkConfig &networkConfig = getClientConfig().getNetworkConfig();
                config::ClientAwsConfig &awsConfig = networkConfig.getAwsConfig();
                std::vector<std::shared_ptr<connection::AddressProvider> > addressProviders;

                if (awsConfig.isEnabled()) {
                    int awsMemberPort = clientProperties.getInteger(clientProperties.getAwsMemberPort());
                    if (awsMemberPort < 0 || awsMemberPort > 65535) {
                        throw (exception::ExceptionBuilder<exception::InvalidConfigurationException>(
                                "HazelcastClientInstanceImpl::createAddressProviders") << "Configured aws member port "
                                                                                       << awsMemberPort
                                                                                       << " is not a valid port number. It should be between 0-65535 inclusive.").build();
                    }
                    addressProviders.push_back(std::shared_ptr<connection::AddressProvider>(
                            new spi::impl::AwsAddressProvider(awsConfig, awsMemberPort, *logger)));
                }

                addressProviders.push_back(std::shared_ptr<connection::AddressProvider>(
                        new spi::impl::DefaultAddressProvider(networkConfig, addressProviders.empty())));

                return addressProviders;
            }

            const std::string &HazelcastClientInstanceImpl::getName() const {
                return instanceName;
            }

            spi::LifecycleService &HazelcastClientInstanceImpl::getLifecycleService() {
                return lifecycleService;
            }

            const std::shared_ptr<ClientLockReferenceIdGenerator> &
            HazelcastClientInstanceImpl::getLockReferenceIdGenerator() const {
                return lockReferenceIdGenerator;
            }

            std::shared_ptr<crdt::pncounter::PNCounter>
            HazelcastClientInstanceImpl::getPNCounter(const std::string &name) {
                crdt::pncounter::impl::PNCounterProxyFactory factory(&clientContext);
                std::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(proxy::ClientPNCounterProxy::SERVICE_NAME, name, factory);

                return std::static_pointer_cast<proxy::ClientPNCounterProxy>(proxy);
            }

            spi::ProxyManager &HazelcastClientInstanceImpl::getProxyManager() {
                return proxyManager;
            }

            void HazelcastClientInstanceImpl::initalizeNearCacheManager() {
                nearCacheManager.reset(new internal::nearcache::NearCacheManager(serializationService, *logger));
            }

            std::shared_ptr<IExecutorService>
            HazelcastClientInstanceImpl::getExecutorService(const std::string &name) {
                executor::impl::ExecutorServiceProxyFactory factory(&clientContext);
                std::shared_ptr<spi::ClientProxy> proxy =
                        getDistributedObjectForService(IExecutorService::SERVICE_NAME, name, factory);

                return std::static_pointer_cast<IExecutorService>(proxy);
            }

            Client HazelcastClientInstanceImpl::getLocalEndpoint() const {
                return clusterService.getLocalClient();
            }

            const std::shared_ptr<util::ILogger> &HazelcastClientInstanceImpl::getLogger() const {
                return logger;
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace impl {
            AbstractLoadBalancer::AbstractLoadBalancer(const AbstractLoadBalancer &rhs) {
                *this = rhs;
            }

            void AbstractLoadBalancer::operator=(const AbstractLoadBalancer &rhs) {
                util::LockGuard lg(const_cast<util::Mutex &>(rhs.membersLock));
                util::LockGuard lg2(membersLock);
                membersRef = rhs.membersRef;
                cluster = rhs.cluster;
            }

            void AbstractLoadBalancer::init(Cluster &cluster) {
                this->cluster = &cluster;
                setMembersRef();
                cluster.addMembershipListener(this);
            }

            void AbstractLoadBalancer::setMembersRef() {
                util::LockGuard lg(membersLock);
                membersRef = cluster->getMembers();
            }

            void AbstractLoadBalancer::memberAdded(const MembershipEvent &membershipEvent) {
                setMembersRef();
            }

            void AbstractLoadBalancer::memberRemoved(const MembershipEvent &membershipEvent) {
                setMembersRef();
            }

            void AbstractLoadBalancer::memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) {
            }

            std::vector<Member> AbstractLoadBalancer::getMembers() {
                util::LockGuard lg(membersLock);
                return membersRef;
            }

            AbstractLoadBalancer::~AbstractLoadBalancer() {
            }

            AbstractLoadBalancer::AbstractLoadBalancer() : cluster(NULL) {
            }

            void AbstractLoadBalancer::init(const InitialMembershipEvent &event) {
                setMembersRef();
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace cluster {
            namespace impl {

                VectorClock::VectorClock() {}

                VectorClock::VectorClock(const VectorClock::TimestampVector &replicaLogicalTimestamps)
                        : replicaTimestampEntries(replicaLogicalTimestamps) {
                    for (const VectorClock::TimestampVector::value_type &replicaTimestamp : replicaLogicalTimestamps) {
                        replicaTimestamps[replicaTimestamp.first] = replicaTimestamp.second;
                    }
                }

                VectorClock::TimestampVector VectorClock::entrySet() {
                    return replicaTimestampEntries;
                }

                bool VectorClock::isAfter(VectorClock &other) {
                    bool anyTimestampGreater = false;
                    for (const VectorClock::TimestampMap::value_type &otherEntry : other.replicaTimestamps) {
                        const std::string &replicaId = otherEntry.first;
                        int64_t otherReplicaTimestamp = otherEntry.second;
                        std::pair<bool, int64_t> localReplicaTimestamp = getTimestampForReplica(replicaId);

                        if (!localReplicaTimestamp.first ||
                            localReplicaTimestamp.second < otherReplicaTimestamp) {
                            return false;
                        } else if (localReplicaTimestamp.second > otherReplicaTimestamp) {
                            anyTimestampGreater = true;
                        }
                    }
                    // there is at least one local timestamp greater or local vector clock has additional timestamps
                    return anyTimestampGreater || other.replicaTimestamps.size() < replicaTimestamps.size();
                }

                std::pair<bool, int64_t> VectorClock::getTimestampForReplica(const std::string &replicaId) {
                    if (replicaTimestamps.count(replicaId) == 0) {
                        return std::make_pair(false, -1);
                    }
                    return std::make_pair(true, replicaTimestamps[replicaId]);
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace cluster {
            namespace memberselector {
                bool MemberSelectors::DataMemberSelector::select(const Member &member) const {
                    return !member.isLiteMember();
                }

                void MemberSelectors::DataMemberSelector::toString(std::ostream &os) const {
                    os << "Default DataMemberSelector";
                }

                const std::unique_ptr<MemberSelector> MemberSelectors::DATA_MEMBER_SELECTOR(
                        new MemberSelectors::DataMemberSelector());
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        Cluster::Cluster(spi::ClientClusterService &clusterService)
                : clusterService(clusterService) {
        }

        void Cluster::addMembershipListener(MembershipListener *listener) {
            clusterService.addMembershipListener(
                    std::shared_ptr<MembershipListener>(new MembershipListenerDelegator(listener)));
        }

        bool Cluster::removeMembershipListener(MembershipListener *listener) {
            return clusterService.removeMembershipListener(listener->getRegistrationId());
        }

        std::vector<Member> Cluster::getMembers() {
            return clusterService.getMemberList();
        }

        std::string Cluster::addMembershipListener(const std::shared_ptr<MembershipListener> &listener) {
            return clusterService.addMembershipListener(listener);
        }

        bool Cluster::removeMembershipListener(const std::string &registrationId) {
            return clusterService.removeMembershipListener(registrationId);
        }

        std::string Cluster::addMembershipListener(const std::shared_ptr<InitialMembershipListener> &listener) {
            return clusterService.addMembershipListener(listener);
        }

        std::string Cluster::addMembershipListener(InitialMembershipListener *listener) {
            return clusterService.addMembershipListener(
                    std::shared_ptr<MembershipListener>(new InitialMembershipListenerDelegator(listener)));

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace crdt {
            namespace pncounter {
                namespace impl {
                    PNCounterProxyFactory::PNCounterProxyFactory(spi::ClientContext *clientContext) : clientContext(
                            clientContext) {}

                    std::shared_ptr<spi::ClientProxy> PNCounterProxyFactory::create(const std::string &id) {
                        return std::shared_ptr<spi::ClientProxy>(
                                new proxy::ClientPNCounterProxy(proxy::ClientPNCounterProxy::SERVICE_NAME, id,
                                                                clientContext));
                    }
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        EntryEventType::EntryEventType() : value(UNDEFINED) {

        }

        EntryEventType::EntryEventType(Type value)
                : value(value) {

        }

        EntryEventType::operator int() const {
            return value;
        }

        void EntryEventType::operator=(int i) {
            value = (EntryEventType::Type) i;
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace monitor {
            namespace impl {
                LocalMapStatsImpl::LocalMapStatsImpl() : nearCacheStats(NULL) {}

                NearCacheStats *LocalMapStatsImpl::getNearCacheStats() {
                    return nearCacheStats;
                }

                void LocalMapStatsImpl::setNearCacheStats(NearCacheStats &stats) {
                    this->nearCacheStats = &stats;
                }
            }
        }
    }
}


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


namespace hazelcast {
    namespace client {
        namespace monitor {
            const int64_t LocalInstanceStats::STAT_NOT_AVAILABLE = -99L;
        }
    }
}




namespace hazelcast {
    namespace client {
        HazelcastClient::HazelcastClient() : clientImpl(new impl::HazelcastClientInstanceImpl(ClientConfig())) {
            clientImpl->start();
        }

        HazelcastClient::HazelcastClient(const ClientConfig &config) : clientImpl(
                new impl::HazelcastClientInstanceImpl(config)) {
            clientImpl->start();
        }

        const std::string &HazelcastClient::getName() const {
            return clientImpl->getName();
        }

        IdGenerator HazelcastClient::getIdGenerator(const std::string &name) {
            return clientImpl->getIdGenerator(name);
        }

        FlakeIdGenerator HazelcastClient::getFlakeIdGenerator(const std::string &name) {
            return clientImpl->getFlakeIdGenerator(name);
        }

        IAtomicLong HazelcastClient::getIAtomicLong(const std::string &name) {
            return clientImpl->getIAtomicLong(name);
        }

        std::shared_ptr<crdt::pncounter::PNCounter> HazelcastClient::getPNCounter(const std::string &name) {
            return clientImpl->getPNCounter(name);
        }

        ICountDownLatch HazelcastClient::getICountDownLatch(const std::string &name) {
            return clientImpl->getICountDownLatch(name);
        }

        ILock HazelcastClient::getILock(const std::string &name) {
            return clientImpl->getILock(name);
        }

        ISemaphore HazelcastClient::getISemaphore(const std::string &name) {
            return clientImpl->getISemaphore(name);
        }

        ClientConfig &HazelcastClient::getClientConfig() {
            return clientImpl->getClientConfig();
        }

        TransactionContext HazelcastClient::newTransactionContext() {
            return clientImpl->newTransactionContext();
        }

        TransactionContext HazelcastClient::newTransactionContext(const TransactionOptions &options) {
            return clientImpl->newTransactionContext(options);
        }

        Cluster &HazelcastClient::getCluster() {
            return clientImpl->getCluster();
        }

        void HazelcastClient::addLifecycleListener(LifecycleListener *lifecycleListener) {
            clientImpl->addLifecycleListener(lifecycleListener);
        }

        bool HazelcastClient::removeLifecycleListener(LifecycleListener *lifecycleListener) {
            return clientImpl->removeLifecycleListener(lifecycleListener);
        }

        void HazelcastClient::shutdown() {
            clientImpl->shutdown();
        }

        mixedtype::HazelcastClient &HazelcastClient::toMixedType() const {
            return clientImpl->toMixedType();
        }

        spi::LifecycleService &HazelcastClient::getLifecycleService() {
            return clientImpl->getLifecycleService();
        }

        std::shared_ptr<IExecutorService> HazelcastClient::getExecutorService(const std::string &name) {
            return clientImpl->getExecutorService(name);
        }

        Client HazelcastClient::getLocalEndpoint() const {
            return clientImpl->getLocalEndpoint();
        }

        HazelcastClient::~HazelcastClient() {
            clientImpl->shutdown();
        }
    }
}



namespace hazelcast {
    namespace client {
        namespace flakeidgen {
            namespace impl {
                FlakeIdGeneratorProxyFactory::FlakeIdGeneratorProxyFactory(spi::ClientContext *clientContext)
                        : clientContext(
                        clientContext) {}

                std::shared_ptr<spi::ClientProxy> FlakeIdGeneratorProxyFactory::create(const std::string &id) {
                    return std::shared_ptr<spi::ClientProxy>(
                            new proxy::ClientFlakeIdGeneratorProxy(id, clientContext));
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace flakeidgen {
            namespace impl {

                AutoBatcher::AutoBatcher(int32_t batchSize, int64_t validity,
                                         const std::shared_ptr<AutoBatcher::IdBatchSupplier> &batchIdSupplier)
                        : batchSize(batchSize), validity(validity), batchIdSupplier(batchIdSupplier),
                          block(std::shared_ptr<Block>(new Block(IdBatch(0, 0, 0), 0))) {}

                int64_t AutoBatcher::newId() {
                    for (;;) {
                        std::shared_ptr<Block> block = this->block;
                        int64_t res = block->next();
                        if (res != INT64_MIN) {
                            return res;
                        }

                        {
                            std::lock_guard<std::mutex> guard(lock);
                            if (block != this->block.get()) {
                                // new block was assigned in the meantime
                                continue;
                            }
                            this->block = std::shared_ptr<Block>(
                                    new Block(batchIdSupplier->newIdBatch(batchSize), validity));
                        }
                    }
                }

                AutoBatcher::Block::Block(const IdBatch &idBatch, int64_t validity) : idBatch(idBatch), numReturned(0) {
                    invalidSince = validity > 0 ? util::currentTimeMillis() + validity : INT64_MAX;
                }

                int64_t AutoBatcher::Block::next() {
                    if (invalidSince <= util::currentTimeMillis()) {
                        return INT64_MIN;
                    }
                    int32_t index;
                    do {
                        index = numReturned;
                        if (index == idBatch.getBatchSize()) {
                            return INT64_MIN;
                        }
                    } while (!numReturned.compare_exchange_strong(index, index + 1));

                    return idBatch.getBase() + index * idBatch.getIncrement();
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace flakeidgen {
            namespace impl {
                IdBatch::IdIterator IdBatch::endOfBatch;

                const int64_t IdBatch::getBase() const {
                    return base;
                }

                const int64_t IdBatch::getIncrement() const {
                    return increment;
                }

                const int32_t IdBatch::getBatchSize() const {
                    return batchSize;
                }

                IdBatch::IdBatch(const int64_t base, const int64_t increment, const int32_t batchSize)
                        : base(base), increment(increment), batchSize(batchSize) {}

                IdBatch::IdIterator &IdBatch::end() {
                    return endOfBatch;
                }

                IdBatch::IdIterator IdBatch::iterator() {
                    return IdBatch::IdIterator(base, increment, batchSize);
                }

                IdBatch::IdIterator::IdIterator(int64_t base2, const int64_t increment, int32_t remaining) : base2(
                        base2), increment(increment), remaining(remaining) {}

                bool IdBatch::IdIterator::operator==(const IdBatch::IdIterator &rhs) const {
                    return base2 == rhs.base2 && increment == rhs.increment && remaining == rhs.remaining;
                }

                bool IdBatch::IdIterator::operator!=(const IdBatch::IdIterator &rhs) const {
                    return !(rhs == *this);
                }

                IdBatch::IdIterator::IdIterator() : base2(-1), increment(-1), remaining(-1) {
                }

                IdBatch::IdIterator &IdBatch::IdIterator::operator++() {
                    if (remaining == 0) {
                        return IdBatch::end();
                    }

                    --remaining;

                    base2 += increment;

                    return *this;
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace txn {
            const std::shared_ptr<util::ExceptionUtil::RuntimeExceptionFactory> ClientTransactionUtil::exceptionFactory(
                    new TransactionExceptionFactory());

            std::shared_ptr<protocol::ClientMessage>
            ClientTransactionUtil::invoke(std::unique_ptr<protocol::ClientMessage> &request,
                                          const std::string &objectName,
                                          spi::ClientContext &client,
                                          const std::shared_ptr<connection::Connection> &connection) {
                try {
                    std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client, request, objectName, connection);
                    std::shared_ptr<spi::impl::ClientInvocationFuture> future = clientInvocation->invoke();
                    return future->get();
                } catch (exception::IException &e) {
                    TRANSACTION_EXCEPTION_FACTORY()->rethrow(e, "ClientTransactionUtil::invoke failed");
                }
                return std::shared_ptr<protocol::ClientMessage>();
            }

            const std::shared_ptr<util::ExceptionUtil::RuntimeExceptionFactory> &
            ClientTransactionUtil::TRANSACTION_EXCEPTION_FACTORY() {
                return exceptionFactory;
            }

            void
            ClientTransactionUtil::TransactionExceptionFactory::rethrow(const client::exception::IException &throwable,
                                                                        const std::string &message) {
                throw TransactionException("TransactionExceptionFactory::create", message,
                                           std::shared_ptr<IException>(throwable.clone()));
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace txn {
#define MILLISECOND_IN_A_SECOND 1000

            TransactionProxy::TransactionProxy(TransactionOptions &txnOptions, spi::ClientContext &clientContext,
                                               std::shared_ptr<connection::Connection> connection)
                    : options(txnOptions), clientContext(clientContext), connection(connection),
                      threadId(util::getCurrentThreadId()), state(TxnState::NO_TXN), startTime(0) {
            }


            TransactionProxy::TransactionProxy(const TransactionProxy &rhs) : options(rhs.options),
                                                                              clientContext(rhs.clientContext),
                                                                              connection(rhs.connection),
                                                                              threadId(rhs.threadId), txnId(rhs.txnId),
                                                                              state(rhs.state),
                                                                              startTime(rhs.startTime) {
                TransactionProxy &nonConstRhs = const_cast<TransactionProxy &>(rhs);

                TRANSACTION_EXISTS.store(nonConstRhs.TRANSACTION_EXISTS.load());
            }

            const std::string &TransactionProxy::getTxnId() const {
                return txnId;
            }

            TxnState TransactionProxy::getState() const {
                return state;
            }

            int TransactionProxy::getTimeoutSeconds() const {
                return options.getTimeout();
            }


            void TransactionProxy::begin() {
                try {
                    if (clientContext.getConnectionManager().getOwnerConnection().get() == NULL) {
                        throw exception::TransactionException("TransactionProxy::begin()",
                                                              "Owner connection needs to be present to begin a transaction");
                    }
                    if (state == TxnState::ACTIVE) {
                        throw exception::IllegalStateException("TransactionProxy::begin()",
                                                               "Transaction is already active");
                    }
                    checkThread();
                    if (TRANSACTION_EXISTS) {
                        throw exception::IllegalStateException("TransactionProxy::begin()",
                                                               "Nested transactions are not allowed!");
                    }
                    TRANSACTION_EXISTS.store(true);
                    startTime = util::currentTimeMillis();
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::TransactionCreateCodec::encodeRequest(
                            options.getTimeout() * MILLISECOND_IN_A_SECOND, options.getDurability(),
                            options.getTransactionType(), threadId);

                    std::shared_ptr<protocol::ClientMessage> response = invoke(request);

                    protocol::codec::TransactionCreateCodec::ResponseParameters result =
                            protocol::codec::TransactionCreateCodec::ResponseParameters::decode(*response);
                    txnId = result.response;
                    state = TxnState::ACTIVE;
                } catch (exception::IException &) {
                    TRANSACTION_EXISTS.store(false);
                    throw;
                }
            }

            void TransactionProxy::commit() {
                try {
                    if (state != TxnState::ACTIVE) {
                        throw exception::IllegalStateException("TransactionProxy::commit()",
                                                               "Transaction is not active");
                    }
                    state = TxnState::COMMITTING;
                    checkThread();
                    checkTimeout();

                    std::unique_ptr<protocol::ClientMessage> request =
                            protocol::codec::TransactionCommitCodec::encodeRequest(txnId, threadId);

                    invoke(request);

                    state = TxnState::COMMITTED;
                } catch (exception::IException &e) {
                    state = TxnState::COMMIT_FAILED;
                    TRANSACTION_EXISTS.store(false);
                    util::ExceptionUtil::rethrow(e);
                }
            }

            void TransactionProxy::rollback() {
                try {
                    if (state == TxnState::NO_TXN || state == TxnState::ROLLED_BACK) {
                        throw exception::IllegalStateException("TransactionProxy::rollback()",
                                                               "Transaction is not active");
                    }
                    state = TxnState::ROLLING_BACK;
                    checkThread();
                    try {
                        std::unique_ptr<protocol::ClientMessage> request =
                                protocol::codec::TransactionRollbackCodec::encodeRequest(txnId, threadId);

                        invoke(request);
                    } catch (exception::IException &exception) {
                        clientContext.getLogger().warning("Exception while rolling back the transaction. Exception:",
                                                          exception);
                    }
                    state = TxnState::ROLLED_BACK;
                    TRANSACTION_EXISTS.store(false);
                } catch (exception::IException &) {
                    TRANSACTION_EXISTS.store(false);
                    throw;
                }
            }

            serialization::pimpl::SerializationService &TransactionProxy::getSerializationService() {
                return clientContext.getSerializationService();
            }

            std::shared_ptr<connection::Connection> TransactionProxy::getConnection() {
                return connection;
            }

            void TransactionProxy::checkThread() {
                if (threadId != util::getCurrentThreadId()) {
                    throw exception::IllegalStateException("TransactionProxy::checkThread()",
                                                           "Transaction cannot span multiple threads!");
                }
            }

            void TransactionProxy::checkTimeout() {
                if (startTime + options.getTimeoutMillis() < util::currentTimeMillis()) {
                    throw exception::TransactionException("TransactionProxy::checkTimeout()",
                                                          "Transaction is timed-out!");
                }
            }

            TxnState::TxnState(State value)
                    : value(value) {
                values.resize(9);
                values[0] = NO_TXN;
                values[1] = ACTIVE;
                values[2] = PREPARING;
                values[3] = PREPARED;
                values[4] = COMMITTING;
                values[5] = COMMITTED;
                values[6] = COMMIT_FAILED;
                values[7] = ROLLING_BACK;
                values[8] = ROLLED_BACK;
            }

            TxnState::operator int() const {
                return value;
            }

            void TxnState::operator=(int i) {
                value = values[i];
            }

            std::shared_ptr<protocol::ClientMessage> TransactionProxy::invoke(
                    std::unique_ptr<protocol::ClientMessage> &request) {
                return ClientTransactionUtil::invoke(request, getTxnId(), clientContext, connection);
            }

            spi::ClientContext &TransactionProxy::getClientContext() const {
                return clientContext;
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace proxy {
            ReliableTopicImpl::ReliableTopicImpl(const std::string &instanceName, spi::ClientContext *context,
                                                 std::shared_ptr<Ringbuffer<topic::impl::reliable::ReliableTopicMessage> > rb)
                    : proxy::ProxyImpl("hz:impl:topicService", instanceName, context), ringbuffer(rb),
                      logger(context->getLogger()),
                      config(context->getClientConfig().getReliableTopicConfig(instanceName)) {
            }

            void ReliableTopicImpl::publish(const serialization::pimpl::Data &data) {
                std::unique_ptr<Address> nullAddress;
                topic::impl::reliable::ReliableTopicMessage message(data, nullAddress);
                ringbuffer->add(message);
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace proxy {
            const std::string ClientPNCounterProxy::SERVICE_NAME = "hz:impl:PNCounterService";
            const std::shared_ptr<std::set<Address> > ClientPNCounterProxy::EMPTY_ADDRESS_LIST(
                    new std::set<Address>());

            ClientPNCounterProxy::ClientPNCounterProxy(const std::string &serviceName, const std::string &objectName,
                                                       spi::ClientContext *context)
                    : ProxyImpl(serviceName, objectName, context), maxConfiguredReplicaCount(0),
                      observedClock(std::shared_ptr<cluster::impl::VectorClock>(new cluster::impl::VectorClock())),
                      logger(context->getLogger()) {
            }

            std::ostream &operator<<(std::ostream &os, const ClientPNCounterProxy &proxy) {
                os << "PNCounter{name='" << proxy.getName() << "\'}";
                return os;
            }

            int64_t ClientPNCounterProxy::get() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::get",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeGetInternal(EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);
                protocol::codec::PNCounterGetCodec::ResponseParameters resultParameters = protocol::codec::PNCounterGetCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndAdd(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndAdd",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(delta, true, EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::addAndGet(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::addAndGet",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(delta, false,
                                                                                      EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndSubtract(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndSubtract",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(-delta, true,
                                                                                      EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::subtractAndGet(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::subtractAndGet",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(-delta, false,
                                                                                      EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::decrementAndGet() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::decrementAndGet",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(-1, false, EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::incrementAndGet() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::incrementAndGet",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(1, false, EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndDecrement() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndDecrement",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(-1, true, EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndIncrement() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndIncrement",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(1, true, EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            void ClientPNCounterProxy::reset() {
                observedClock = std::shared_ptr<cluster::impl::VectorClock>(new cluster::impl::VectorClock());
            }

            std::shared_ptr<Address>
            ClientPNCounterProxy::getCRDTOperationTarget(const std::set<Address> &excludedAddresses) {
                if (currentTargetReplicaAddress.get().get() != NULL &&
                    excludedAddresses.find(*currentTargetReplicaAddress.get()) == excludedAddresses.end()) {
                    return currentTargetReplicaAddress;
                }

                {
                    util::LockGuard guard(targetSelectionMutex);
                    if (currentTargetReplicaAddress.get() == NULL ||
                        excludedAddresses.find(*currentTargetReplicaAddress.get()) != excludedAddresses.end()) {
                        currentTargetReplicaAddress = chooseTargetReplica(excludedAddresses);
                    }
                }
                return currentTargetReplicaAddress;
            }

            std::shared_ptr<Address>
            ClientPNCounterProxy::chooseTargetReplica(const std::set<Address> &excludedAddresses) {
                std::vector<Address> replicaAddresses = getReplicaAddresses(excludedAddresses);
                if (replicaAddresses.empty()) {
                    return std::shared_ptr<Address>();
                }
                // TODO: Use a random generator as used in Java (ThreadLocalRandomProvider) which is per thread
                int randomReplicaIndex = std::abs(rand()) % (int) replicaAddresses.size();
                return std::shared_ptr<Address>(new Address(replicaAddresses[randomReplicaIndex]));
            }

            std::vector<Address> ClientPNCounterProxy::getReplicaAddresses(const std::set<Address> &excludedAddresses) {
                std::vector<Member> dataMembers = getContext().getClientClusterService().getMembers(
                        *cluster::memberselector::MemberSelectors::DATA_MEMBER_SELECTOR);
                int32_t maxConfiguredReplicaCount = getMaxConfiguredReplicaCount();
                int currentReplicaCount = util::min<int>(maxConfiguredReplicaCount, (int) dataMembers.size());

                std::vector<Address> replicaAddresses;
                for (int i = 0; i < currentReplicaCount; i++) {
                    const Address &dataMemberAddress = dataMembers[i].getAddress();
                    if (excludedAddresses.find(dataMemberAddress) == excludedAddresses.end()) {
                        replicaAddresses.push_back(dataMemberAddress);
                    }
                }
                return replicaAddresses;
            }

            int32_t ClientPNCounterProxy::getMaxConfiguredReplicaCount() {
                if (maxConfiguredReplicaCount > 0) {
                    return maxConfiguredReplicaCount;
                } else {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::PNCounterGetConfiguredReplicaCountCodec::encodeRequest(
                            getName());
                    maxConfiguredReplicaCount = invokeAndGetResult<int32_t, protocol::codec::PNCounterGetConfiguredReplicaCountCodec::ResponseParameters>(
                            request);
                }
                return maxConfiguredReplicaCount;
            }

            std::shared_ptr<protocol::ClientMessage>
            ClientPNCounterProxy::invokeGetInternal(std::shared_ptr<std::set<Address> > excludedAddresses,
                                                    const std::unique_ptr<exception::IException> &lastException,
                                                    const std::shared_ptr<Address> &target) {
                if (target.get() == NULL) {
                    if (lastException.get()) {
                        throw *lastException;
                    } else {
                        throw (exception::ExceptionBuilder<exception::NoDataMemberInClusterException>(
                                "ClientPNCounterProxy::invokeGetInternal") <<
                                                                           "Cannot invoke operations on a CRDT because the cluster does not contain any data members").build();
                    }
                }
                try {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::PNCounterGetCodec::encodeRequest(
                            getName(), observedClock.get()->entrySet(), *target);
                    return invokeOnAddress(request, *target);
                } catch (exception::HazelcastException &e) {
                    logger.finest("Exception occurred while invoking operation on target ", *target,
                                  ", choosing different target. Cause: ", e);
                    if (excludedAddresses == EMPTY_ADDRESS_LIST) {
                        // TODO: Make sure that this only affects the local variable of the method
                        excludedAddresses = std::shared_ptr<std::set<Address> >(new std::set<Address>());
                    }
                    excludedAddresses->insert(*target);
                    std::shared_ptr<Address> newTarget = getCRDTOperationTarget(*excludedAddresses);
                    std::unique_ptr<exception::IException> exception = e.clone();
                    return invokeGetInternal(excludedAddresses, exception, newTarget);
                }
            }


            std::shared_ptr<protocol::ClientMessage>
            ClientPNCounterProxy::invokeAddInternal(int64_t delta, bool getBeforeUpdate,
                                                    std::shared_ptr<std::set<Address> > excludedAddresses,
                                                    const std::unique_ptr<exception::IException> &lastException,
                                                    const std::shared_ptr<Address> &target) {
                if (target.get() == NULL) {
                    if (lastException.get()) {
                        throw *lastException;
                    } else {
                        throw (exception::ExceptionBuilder<exception::NoDataMemberInClusterException>(
                                "ClientPNCounterProxy::invokeAddInternal") <<
                                                                           "Cannot invoke operations on a CRDT because the cluster does not contain any data members").build();
                    }
                }

                try {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::PNCounterAddCodec::encodeRequest(
                            getName(), delta, getBeforeUpdate, observedClock.get()->entrySet(), *target);
                    return invokeOnAddress(request, *target);
                } catch (exception::HazelcastException &e) {
                    logger.finest("Unable to provide session guarantees when sending operations to ", *target,
                                  ", choosing different target. Cause: ", e);
                    if (excludedAddresses == EMPTY_ADDRESS_LIST) {
                        // TODO: Make sure that this only affects the local variable of the method
                        excludedAddresses = std::shared_ptr<std::set<Address> >(new std::set<Address>());
                    }
                    excludedAddresses->insert(*target);
                    std::shared_ptr<Address> newTarget = getCRDTOperationTarget(*excludedAddresses);
                    std::unique_ptr<exception::IException> exception = e.clone();
                    return invokeAddInternal(delta, getBeforeUpdate, excludedAddresses, exception, newTarget);
                }
            }

            void ClientPNCounterProxy::updateObservedReplicaTimestamps(
                    const std::vector<std::pair<std::string, int64_t> > &receivedLogicalTimestamps) {
                std::shared_ptr<cluster::impl::VectorClock> received = toVectorClock(receivedLogicalTimestamps);
                for (;;) {
                    std::shared_ptr<cluster::impl::VectorClock> currentClock = this->observedClock;
                    if (currentClock->isAfter(*received)) {
                        break;
                    }
                    if (observedClock.compareAndSet(currentClock, received)) {
                        break;
                    }
                }
            }

            std::shared_ptr<cluster::impl::VectorClock> ClientPNCounterProxy::toVectorClock(
                    const std::vector<std::pair<std::string, int64_t> > &replicaLogicalTimestamps) {
                return std::shared_ptr<cluster::impl::VectorClock>(
                        new cluster::impl::VectorClock(replicaLogicalTimestamps));
            }

            std::shared_ptr<Address> ClientPNCounterProxy::getCurrentTargetReplicaAddress() {
                return currentTargetReplicaAddress.get();
            }
        }
    }
}






// Includes for parameters classes

namespace hazelcast {
    namespace client {
        namespace proxy {

            IListImpl::IListImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:listService", instanceName, context) {
                serialization::pimpl::Data keyData = getContext().getSerializationService().toData<std::string>(
                        &instanceName);
                partitionId = getPartitionId(keyData);
            }

            std::string IListImpl::addItemListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                return registerListener(createItemListenerCodec(includeValue), entryEventHandler);
            }

            bool IListImpl::removeItemListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            int IListImpl::size() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListSizeCodec::encodeRequest(getName());

                return invokeAndGetResult<int, protocol::codec::ListSizeCodec::ResponseParameters>(request,
                                                                                                   partitionId);
            }

            bool IListImpl::isEmpty() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListIsEmptyCodec::encodeRequest(getName());

                return invokeAndGetResult<bool, protocol::codec::ListIsEmptyCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            bool IListImpl::contains(const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListContainsCodec::encodeRequest(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::ListContainsCodec::ResponseParameters>(request,
                                                                                                        partitionId);
            }

            std::vector<serialization::pimpl::Data> IListImpl::toArrayData() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListGetAllCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::ListGetAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IListImpl::add(const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListAddCodec::encodeRequest(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::ListAddCodec::ResponseParameters>(request,
                                                                                                   partitionId);
            }

            bool IListImpl::remove(const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListRemoveCodec::encodeRequest(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::ListRemoveCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            bool IListImpl::containsAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListContainsAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::ListContainsAllCodec::ResponseParameters>(request,
                                                                                                           partitionId);
            }

            bool IListImpl::addAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListAddAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::ListAddAllCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            bool IListImpl::addAll(int index, const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListAddAllWithIndexCodec::encodeRequest(getName(), index,
                                                                                 elements);

                return invokeAndGetResult<bool, protocol::codec::ListAddAllWithIndexCodec::ResponseParameters>(request,
                                                                                                               partitionId);
            }

            bool IListImpl::removeAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListCompareAndRemoveAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::ListCompareAndRemoveAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IListImpl::retainAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListCompareAndRetainAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::ListCompareAndRetainAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            void IListImpl::clear() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListClearCodec::encodeRequest(getName());

                invokeOnPartition(request, partitionId);
            }

            std::unique_ptr<serialization::pimpl::Data> IListImpl::getData(int index) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListGetCodec::encodeRequest(getName(), index);

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::ListGetCodec::ResponseParameters>(
                        request, partitionId);
            }

            std::unique_ptr<serialization::pimpl::Data> IListImpl::setData(int index,
                                                                           const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListSetCodec::encodeRequest(getName(), index, element);

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::ListSetCodec::ResponseParameters>(
                        request, partitionId);
            }

            void IListImpl::add(int index, const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListAddWithIndexCodec::encodeRequest(getName(), index, element);

                invokeOnPartition(request, partitionId);
            }

            std::unique_ptr<serialization::pimpl::Data> IListImpl::removeData(int index) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListRemoveWithIndexCodec::encodeRequest(getName(), index);

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::ListRemoveWithIndexCodec::ResponseParameters>(
                        request, partitionId);
            }

            int IListImpl::indexOf(const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListIndexOfCodec::encodeRequest(getName(), element);

                return invokeAndGetResult<int, protocol::codec::ListIndexOfCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            int IListImpl::lastIndexOf(const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListLastIndexOfCodec::encodeRequest(getName(), element);

                return invokeAndGetResult<int, protocol::codec::ListLastIndexOfCodec::ResponseParameters>(request,
                                                                                                          partitionId);
            }

            std::vector<serialization::pimpl::Data> IListImpl::subListData(int fromIndex, int toIndex) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListSubCodec::encodeRequest(getName(), fromIndex, toIndex);

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::ListSubCodec::ResponseParameters>(
                        request, partitionId);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IListImpl::createItemListenerCodec(bool includeValue) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new ListListenerMessageCodec(getName(), includeValue));
            }

            IListImpl::ListListenerMessageCodec::ListListenerMessageCodec(const std::string &name,
                                                                          bool includeValue) : name(name),
                                                                                               includeValue(
                                                                                                       includeValue) {}

            std::unique_ptr<protocol::ClientMessage>
            IListImpl::ListListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::ListAddListenerCodec::encodeRequest(name, includeValue, localOnly);
            }

            std::string IListImpl::ListListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::ListAddListenerCodec::ResponseParameters::decode(responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            IListImpl::ListListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::ListRemoveListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool IListImpl::ListListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::ListRemoveListenerCodec::ResponseParameters::decode(clientMessage).response;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace proxy {
            const std::string ClientIdGeneratorProxy::SERVICE_NAME = "hz:impl:idGeneratorService";
            const std::string ClientIdGeneratorProxy::ATOMIC_LONG_NAME = "hz:atomic:idGenerator:";

            ClientIdGeneratorProxy::ClientIdGeneratorProxy(const std::string &instanceName, spi::ClientContext *context,
                                                           const IAtomicLong &atomicLong)
                    : proxy::ProxyImpl(ClientIdGeneratorProxy::SERVICE_NAME, instanceName, context),
                      atomicLong(atomicLong), local(new std::atomic<int64_t>(-1)),
                      residue(new std::atomic<int32_t>(BLOCK_SIZE)), localLock(new util::Mutex) {
                this->atomicLong.get();
            }

            bool ClientIdGeneratorProxy::init(int64_t id) {
                if (id < 0) {
                    return false;
                }
                int64_t step = (id / BLOCK_SIZE);

                util::LockGuard lg(*localLock);
                bool init = atomicLong.compareAndSet(0, step + 1);
                if (init) {
                    local->store(step);
                    residue->store((int32_t) (id % BLOCK_SIZE) + 1);
                }
                return init;
            }

            int64_t ClientIdGeneratorProxy::newId() {
                int64_t block = local->load();
                int32_t value = (*residue)++;

                if (local->load() != block) {
                    return newId();
                }

                if (value < BLOCK_SIZE) {
                    return block * BLOCK_SIZE + value;
                }

                {
                    util::LockGuard lg(*localLock);
                    value = *residue;
                    if (value >= BLOCK_SIZE) {
                        *local = atomicLong.getAndIncrement();
                        *residue = 0;
                    }
                }

                return newId();
            }

            void ClientIdGeneratorProxy::destroy() {
                util::LockGuard lg(*localLock);
                atomicLong.destroy();
                *local = -1;
                *residue = BLOCK_SIZE;
            }
        }
    }
}



// Includes for parameters classes

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalMapImpl::TransactionalMapImpl(const std::string &name, txn::TransactionProxy *transactionProxy)
                    : TransactionalObject("hz:impl:mapService", name, transactionProxy) {

            }

            bool TransactionalMapImpl::containsKey(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapContainsKeyCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return invokeAndGetResult<bool, protocol::codec::TransactionalMapContainsKeyCodec::ResponseParameters>(
                        request);
            }

            std::unique_ptr<serialization::pimpl::Data>
            TransactionalMapImpl::getData(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapGetCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::TransactionalMapGetCodec::ResponseParameters>(
                        request);
            }

            int TransactionalMapImpl::size() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapSizeCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetResult<int, protocol::codec::TransactionalMapSizeCodec::ResponseParameters>(request);
            }

            bool TransactionalMapImpl::isEmpty() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapIsEmptyCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetResult<bool, protocol::codec::TransactionalMapIsEmptyCodec::ResponseParameters>(
                        request);
            }

            std::unique_ptr<serialization::pimpl::Data> TransactionalMapImpl::putData(
                    const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {

                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapPutCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value,
                                getTimeoutInMilliseconds());

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::TransactionalMapPutCodec::ResponseParameters>(
                        request);

            }

            void
            TransactionalMapImpl::set(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapSetCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value);

                invoke(request);
            }

            std::unique_ptr<serialization::pimpl::Data>
            TransactionalMapImpl::putIfAbsentData(const serialization::pimpl::Data &key,
                                                  const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapPutIfAbsentCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value);

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::TransactionalMapPutIfAbsentCodec::ResponseParameters>(
                        request);
            }

            std::unique_ptr<serialization::pimpl::Data>
            TransactionalMapImpl::replaceData(const serialization::pimpl::Data &key,
                                              const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapReplaceCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value);

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::TransactionalMapReplaceCodec::ResponseParameters>(
                        request);
            }

            bool TransactionalMapImpl::replace(const serialization::pimpl::Data &key,
                                               const serialization::pimpl::Data &oldValue,
                                               const serialization::pimpl::Data &newValue) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapReplaceIfSameCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, oldValue, newValue);

                return invokeAndGetResult<bool, protocol::codec::TransactionalMapReplaceIfSameCodec::ResponseParameters>(
                        request);
            }

            std::unique_ptr<serialization::pimpl::Data>
            TransactionalMapImpl::removeData(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapRemoveCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::TransactionalMapRemoveCodec::ResponseParameters>(
                        request);
            }

            void TransactionalMapImpl::deleteEntry(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapDeleteCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                invoke(request);
            }

            bool TransactionalMapImpl::remove(const serialization::pimpl::Data &key,
                                              const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapRemoveIfSameCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value);

                return invokeAndGetResult<bool, protocol::codec::TransactionalMapRemoveIfSameCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> TransactionalMapImpl::keySetData() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapKeySetCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::TransactionalMapKeySetCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data>
            TransactionalMapImpl::keySetData(const serialization::IdentifiedDataSerializable *predicate) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapKeySetWithPredicateCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(),
                                toData<serialization::IdentifiedDataSerializable>(predicate));

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::TransactionalMapKeySetWithPredicateCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> TransactionalMapImpl::valuesData() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapValuesCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::TransactionalMapValuesCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data>
            TransactionalMapImpl::valuesData(const serialization::IdentifiedDataSerializable *predicate) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapValuesWithPredicateCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(),
                                toData<serialization::IdentifiedDataSerializable>(predicate));

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::TransactionalMapValuesWithPredicateCodec::ResponseParameters>(
                        request);
            }
        }
    }
}




// Includes for parameters classes

namespace hazelcast {
    namespace client {
        namespace proxy {

            TransactionalMultiMapImpl::TransactionalMultiMapImpl(const std::string &name,
                                                                 txn::TransactionProxy *transactionProxy)
                    : TransactionalObject("hz:impl:multiMapService", name, transactionProxy) {

            }

            bool TransactionalMultiMapImpl::put(const serialization::pimpl::Data &key,
                                                const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMultiMapPutCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value);

                return invokeAndGetResult<bool, protocol::codec::TransactionalMultiMapPutCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> TransactionalMultiMapImpl::getData(
                    const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMultiMapGetCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::TransactionalMultiMapGetCodec::ResponseParameters>(
                        request);

            }

            bool TransactionalMultiMapImpl::remove(const serialization::pimpl::Data &key,
                                                   const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMultiMapRemoveEntryCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value);

                return invokeAndGetResult<bool, protocol::codec::TransactionalMultiMapRemoveEntryCodec::ResponseParameters>(
                        request);

            }

            std::vector<serialization::pimpl::Data> TransactionalMultiMapImpl::removeData(
                    const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMultiMapRemoveCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::TransactionalMultiMapRemoveCodec::ResponseParameters>(
                        request);

            }

            int TransactionalMultiMapImpl::valueCount(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMultiMapValueCountCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return invokeAndGetResult<int, protocol::codec::TransactionalMultiMapValueCountCodec::ResponseParameters>(
                        request);
            }

            int TransactionalMultiMapImpl::size() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMultiMapSizeCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetResult<int, protocol::codec::TransactionalMultiMapSizeCodec::ResponseParameters>(
                        request);
            }

        }
    }
}



// Includes for parameters classes

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalListImpl::TransactionalListImpl(const std::string &objectName, txn::TransactionProxy *context)
                    : TransactionalObject("hz:impl:listService", objectName, context) {
            }

            bool TransactionalListImpl::add(const serialization::pimpl::Data &e) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalListAddCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e);

                return invokeAndGetResult<bool, protocol::codec::TransactionalListAddCodec::ResponseParameters>(
                        request);
            }

            bool TransactionalListImpl::remove(const serialization::pimpl::Data &e) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalListRemoveCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e);

                return invokeAndGetResult<bool, protocol::codec::TransactionalListRemoveCodec::ResponseParameters>(
                        request);
            }

            int TransactionalListImpl::size() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalListSizeCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetResult<int, protocol::codec::TransactionalListSizeCodec::ResponseParameters>(
                        request);
            }
        }
    }
}




// Includes for parameters classes

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalSetImpl::TransactionalSetImpl(const std::string &name, txn::TransactionProxy *transactionProxy)
                    : TransactionalObject("hz:impl:setService", name, transactionProxy) {

            }

            bool TransactionalSetImpl::add(const serialization::pimpl::Data &e) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalSetAddCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e);

                return invokeAndGetResult<bool, protocol::codec::TransactionalSetAddCodec::ResponseParameters>(request);
            }

            bool TransactionalSetImpl::remove(const serialization::pimpl::Data &e) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalSetRemoveCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e);

                return invokeAndGetResult<bool, protocol::codec::TransactionalSetRemoveCodec::ResponseParameters>(
                        request);
            }

            int TransactionalSetImpl::size() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalSetSizeCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetResult<int, protocol::codec::TransactionalSetSizeCodec::ResponseParameters>(request);
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace proxy {
#define MILLISECONDS_IN_A_SECOND 1000

            TransactionalObject::TransactionalObject(const std::string &serviceName, const std::string &objectName,
                                                     txn::TransactionProxy *context)
                    : serviceName(serviceName), name(objectName), context(context) {

            }

            TransactionalObject::~TransactionalObject() {

            }

            const std::string &TransactionalObject::getServiceName() {
                return serviceName;
            }

            const std::string &TransactionalObject::getName() {
                return name;
            }

            void TransactionalObject::destroy() {
                onDestroy();

                std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ClientDestroyProxyCodec::encodeRequest(
                        name, serviceName);

                std::shared_ptr<connection::Connection> connection = context->getConnection();
                std::shared_ptr<spi::impl::ClientInvocation> invocation = spi::impl::ClientInvocation::create(
                        context->getClientContext(), request, name, connection);
                invocation->invoke()->get();
            }

            void TransactionalObject::onDestroy() {

            }

            std::string TransactionalObject::getTransactionId() const {
                return context->getTxnId();
            }

            int TransactionalObject::getTimeoutInMilliseconds() const {
                return context->getTimeoutSeconds() * MILLISECONDS_IN_A_SECOND;
            }

            std::shared_ptr<protocol::ClientMessage> TransactionalObject::invoke(
                    std::unique_ptr<protocol::ClientMessage> &request) {
                std::shared_ptr<connection::Connection> connection = context->getConnection();
                std::shared_ptr<spi::impl::ClientInvocation> invocation = spi::impl::ClientInvocation::create(
                        context->getClientContext(), request, name, connection);
                return invocation->invoke()->get();
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace proxy {
            const std::string ClientFlakeIdGeneratorProxy::SERVICE_NAME = "hz:impl:flakeIdGeneratorService";

            ClientFlakeIdGeneratorProxy::ClientFlakeIdGeneratorProxy(const std::string &objectName,
                                                                     spi::ClientContext *context)
                    : ProxyImpl(SERVICE_NAME, objectName, context) {
                std::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = getContext().getClientConfig().findFlakeIdGeneratorConfig(
                        getName());
                batcher.reset(
                        new flakeidgen::impl::AutoBatcher(config->getPrefetchCount(),
                                                          config->getPrefetchValidityMillis(),
                                                          std::shared_ptr<flakeidgen::impl::AutoBatcher::IdBatchSupplier>(
                                                                  new FlakeIdBatchSupplier(*this))));
            }

            int64_t ClientFlakeIdGeneratorProxy::newId() {
                return batcher->newId();
            }

            flakeidgen::impl::IdBatch ClientFlakeIdGeneratorProxy::newIdBatch(int32_t batchSize) {
                std::unique_ptr<protocol::ClientMessage> requestMsg = protocol::codec::FlakeIdGeneratorNewIdBatchCodec::encodeRequest(
                        getName(), batchSize);
                std::shared_ptr<protocol::ClientMessage> responseMsg = spi::impl::ClientInvocation::create(
                        getContext(), requestMsg, getName())->invoke()->get();
                protocol::codec::FlakeIdGeneratorNewIdBatchCodec::ResponseParameters response =
                        protocol::codec::FlakeIdGeneratorNewIdBatchCodec::ResponseParameters::decode(*responseMsg);
                return flakeidgen::impl::IdBatch(response.base, response.increment, response.batchSize);
            }

            bool ClientFlakeIdGeneratorProxy::init(int64_t id) {
                // Add 1 hour worth of IDs as a reserve: due to long batch validity some clients might be still getting
                // older IDs. 1 hour is just a safe enough value, not a real guarantee: some clients might have longer
                // validity.
                // The init method should normally be called before any client generated IDs: in this case no reserve is
                // needed, so we don't want to increase the reserve excessively.
                int64_t reserve =
                        (int64_t) (3600 * 1000) /* 1 HOUR in milliseconds */ << (BITS_NODE_ID + BITS_SEQUENCE);
                return newId() >= id + reserve;
            }

            ClientFlakeIdGeneratorProxy::FlakeIdBatchSupplier::FlakeIdBatchSupplier(ClientFlakeIdGeneratorProxy &proxy)
                    : proxy(proxy) {}

            flakeidgen::impl::IdBatch ClientFlakeIdGeneratorProxy::FlakeIdBatchSupplier::newIdBatch(int32_t batchSize) {
                return proxy.newIdBatch(batchSize);
            }
        }
    }
}



// Includes for parameters classes


namespace hazelcast {
    namespace client {
        namespace proxy {
            IQueueImpl::IQueueImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:queueService", instanceName, context) {
                serialization::pimpl::Data data = getContext().getSerializationService().toData<std::string>(
                        &instanceName);
                partitionId = getPartitionId(data);
            }

            std::string IQueueImpl::addItemListener(impl::BaseEventHandler *itemEventHandler, bool includeValue) {
                return registerListener(createItemListenerCodec(includeValue), itemEventHandler);
            }

            bool IQueueImpl::removeItemListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            bool IQueueImpl::offer(const serialization::pimpl::Data &element, long timeoutInMillis) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueOfferCodec::encodeRequest(getName(), element,
                                                                        timeoutInMillis);

                return invokeAndGetResult<bool, protocol::codec::QueueOfferCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            void IQueueImpl::put(const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueuePutCodec::encodeRequest(getName(), element);

                invokeOnPartition(request, partitionId);
            }

            std::unique_ptr<serialization::pimpl::Data> IQueueImpl::pollData(long timeoutInMillis) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueuePollCodec::encodeRequest(getName(), timeoutInMillis);

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::QueuePollCodec::ResponseParameters>(
                        request, partitionId);
            }

            int IQueueImpl::remainingCapacity() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueRemainingCapacityCodec::encodeRequest(getName());

                return invokeAndGetResult<int, protocol::codec::QueueRemainingCapacityCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IQueueImpl::remove(const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueRemoveCodec::encodeRequest(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::QueueRemoveCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            bool IQueueImpl::contains(const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueContainsCodec::encodeRequest(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::QueueContainsCodec::ResponseParameters>(request,
                                                                                                         partitionId);
            }

            std::vector<serialization::pimpl::Data> IQueueImpl::drainToData(size_t maxElements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueDrainToMaxSizeCodec::encodeRequest(getName(), (int32_t) maxElements);

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::QueueDrainToMaxSizeCodec::ResponseParameters>(
                        request, partitionId);
            }

            std::vector<serialization::pimpl::Data> IQueueImpl::drainToData() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueDrainToCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::QueueDrainToMaxSizeCodec::ResponseParameters>(
                        request, partitionId);
            }

            std::unique_ptr<serialization::pimpl::Data> IQueueImpl::peekData() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueuePeekCodec::encodeRequest(getName());

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::QueuePeekCodec::ResponseParameters>(
                        request, partitionId);
            }

            int IQueueImpl::size() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueSizeCodec::encodeRequest(getName());

                return invokeAndGetResult<int, protocol::codec::QueueSizeCodec::ResponseParameters>(request,
                                                                                                    partitionId);
            }

            bool IQueueImpl::isEmpty() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueIsEmptyCodec::encodeRequest(getName());

                return invokeAndGetResult<bool, protocol::codec::QueueIsEmptyCodec::ResponseParameters>(request,
                                                                                                        partitionId);
            }

            std::vector<serialization::pimpl::Data> IQueueImpl::toArrayData() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueIteratorCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::QueueIteratorCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IQueueImpl::containsAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueContainsAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::QueueContainsAllCodec::ResponseParameters>(request,
                                                                                                            partitionId);
            }

            bool IQueueImpl::addAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueAddAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::QueueAddAllCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            bool IQueueImpl::removeAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueCompareAndRemoveAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::QueueCompareAndRemoveAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IQueueImpl::retainAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueCompareAndRetainAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::QueueCompareAndRetainAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            void IQueueImpl::clear() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueClearCodec::encodeRequest(getName());

                invokeOnPartition(request, partitionId);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IQueueImpl::createItemListenerCodec(bool includeValue) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new QueueListenerMessageCodec(getName(), includeValue));
            }

            IQueueImpl::QueueListenerMessageCodec::QueueListenerMessageCodec(const std::string &name,
                                                                             bool includeValue) : name(name),
                                                                                                  includeValue(
                                                                                                          includeValue) {}

            std::unique_ptr<protocol::ClientMessage>
            IQueueImpl::QueueListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::QueueAddListenerCodec::encodeRequest(name, includeValue, localOnly);
            }

            std::string IQueueImpl::QueueListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::QueueAddListenerCodec::ResponseParameters::decode(responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            IQueueImpl::QueueListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::QueueRemoveListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool IQueueImpl::QueueListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::QueueRemoveListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

        }
    }
}


namespace hazelcast {
    namespace client {
        namespace proxy {
            ProxyImpl::ProxyImpl(const std::string &serviceName, const std::string &objectName,
                                 spi::ClientContext *context)
                    : ClientProxy(objectName, serviceName, *context) {
            }

            ProxyImpl::~ProxyImpl() {
            }

            int ProxyImpl::getPartitionId(const serialization::pimpl::Data &key) {
                return getContext().getPartitionService().getPartitionId(key);
            }

            std::shared_ptr<protocol::ClientMessage> ProxyImpl::invokeOnPartition(
                    std::unique_ptr<protocol::ClientMessage> &request, int partitionId) {
                try {
                    std::shared_ptr<spi::impl::ClientInvocationFuture> future = invokeAndGetFuture(request,
                                                                                                   partitionId);
                    return future->get();
                } catch (exception::IException &e) {
                    util::ExceptionUtil::rethrow(e);
                }
                return std::shared_ptr<protocol::ClientMessage>();
            }

            std::shared_ptr<spi::impl::ClientInvocationFuture>
            ProxyImpl::invokeAndGetFuture(std::unique_ptr<protocol::ClientMessage> &request, int partitionId) {
                try {
                    std::shared_ptr<spi::impl::ClientInvocation> invocation = spi::impl::ClientInvocation::create(
                            getContext(), request, getName(), partitionId);
                    return invocation->invoke();
                } catch (exception::IException &e) {
                    util::ExceptionUtil::rethrow(e);
                }
                return std::shared_ptr<spi::impl::ClientInvocationFuture>();
            }

            std::shared_ptr<spi::impl::ClientInvocationFuture>
            ProxyImpl::invokeOnKeyOwner(std::unique_ptr<protocol::ClientMessage> &request,
                                        const serialization::pimpl::Data &keyData) {
                int partitionId = getPartitionId(keyData);
                std::shared_ptr<spi::impl::ClientInvocation> invocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), partitionId);
                return invocation->invoke();
            }

            std::shared_ptr<protocol::ClientMessage>
            ProxyImpl::invoke(std::unique_ptr<protocol::ClientMessage> &request) {
                try {
                    std::shared_ptr<spi::impl::ClientInvocation> invocation = spi::impl::ClientInvocation::create(
                            getContext(), request, getName());
                    return invocation->invoke()->get();
                } catch (exception::IException &e) {
                    util::ExceptionUtil::rethrow(e);
                }
                return std::shared_ptr<protocol::ClientMessage>();
            }

            std::shared_ptr<protocol::ClientMessage>
            ProxyImpl::invokeOnAddress(std::unique_ptr<protocol::ClientMessage> &request, const Address &address) {
                try {

                    std::shared_ptr<spi::impl::ClientInvocation> invocation = spi::impl::ClientInvocation::create(
                            getContext(), request, getName(), address);
                    return invocation->invoke()->get();
                } catch (exception::IException &e) {
                    util::ExceptionUtil::rethrow(e);
                }
                return std::shared_ptr<protocol::ClientMessage>();
            }

            std::vector<hazelcast::client::TypedData>
            ProxyImpl::toTypedDataCollection(const std::vector<serialization::pimpl::Data> &values) {
                std::vector<hazelcast::client::TypedData> result;
                typedef std::vector<serialization::pimpl::Data> VALUES;
                for (const VALUES::value_type &value : values) {
                    result.push_back(TypedData(
                            std::unique_ptr<serialization::pimpl::Data>(
                                    new serialization::pimpl::Data(value)),
                            getContext().getSerializationService()));
                }
                return result;
            }

            std::vector<std::pair<TypedData, TypedData> > ProxyImpl::toTypedDataEntrySet(
                    const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &dataEntrySet) {
                typedef std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > ENTRIES_DATA;
                std::vector<std::pair<TypedData, TypedData> > result;
                for (const ENTRIES_DATA::value_type &value : dataEntrySet) {
                    serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
                    TypedData keyData(std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(value.first)), serializationService);
                    TypedData valueData(std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(value.second)), serializationService);
                    result.push_back(std::make_pair(keyData, valueData));
                }
                return result;
            }

            std::shared_ptr<serialization::pimpl::Data> ProxyImpl::toShared(const serialization::pimpl::Data &data) {
                return std::shared_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(data));
            }

        }
    }
}


namespace hazelcast {
    namespace client {
        namespace proxy {

            PartitionSpecificClientProxy::PartitionSpecificClientProxy(const std::string &serviceName,
                                                                       const std::string &objectName,
                                                                       spi::ClientContext *context) : ProxyImpl(
                    serviceName, objectName, context) {}

            void PartitionSpecificClientProxy::onInitialize() {
                std::string partitionKey = internal::partition::strategy::StringPartitioningStrategy::getPartitionKey(
                        name);
                partitionId = getContext().getPartitionService().getPartitionId(toData<std::string>(partitionKey));
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace proxy {
            const std::string ClientAtomicLongProxy::SERVICE_NAME = "hz:impl:atomicLongService";

            ClientAtomicLongProxy::ClientAtomicLongProxy(const std::string &objectName, spi::ClientContext *context)
                    : PartitionSpecificClientProxy(SERVICE_NAME, objectName, context) {
            }

            int64_t ClientAtomicLongProxy::addAndGet(int64_t delta) {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(
                        addAndGetAsync(delta))->join());
            }

            bool ClientAtomicLongProxy::compareAndSet(int64_t expect, int64_t update) {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<bool> >(
                        compareAndSetAsync(expect, update))->join());
            }

            int64_t ClientAtomicLongProxy::decrementAndGet() {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(
                        decrementAndGetAsync())->join());
            }

            int64_t ClientAtomicLongProxy::get() {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(getAsync())->join());
            }

            int64_t ClientAtomicLongProxy::getAndAdd(int64_t delta) {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(
                        getAndAddAsync(delta))->join());
            }

            int64_t ClientAtomicLongProxy::getAndSet(int64_t newValue) {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(
                        getAndSetAsync(newValue))->join());
            }

            int64_t ClientAtomicLongProxy::incrementAndGet() {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(
                        incrementAndGetAsync())->join());
            }

            int64_t ClientAtomicLongProxy::getAndIncrement() {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(
                        getAndIncrementAsync())->join());
            }

            void ClientAtomicLongProxy::set(int64_t newValue) {
                std::static_pointer_cast<spi::InternalCompletableFuture<void> >(setAsync(newValue))->join();
            }

            std::shared_ptr<ICompletableFuture<int64_t> >
            ClientAtomicLongProxy::addAndGetAsync(int64_t delta) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongAddAndGetCodec::encodeRequest(name, delta);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongAddAndGetCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<bool> >
            ClientAtomicLongProxy::compareAndSetAsync(int64_t expect, int64_t update) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongCompareAndSetCodec::encodeRequest(name, expect, update);

                return invokeOnPartitionAsync<bool>(request,
                                                    impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongCompareAndSetCodec, bool>::instance());
            }

            std::shared_ptr<ICompletableFuture<int64_t> > ClientAtomicLongProxy::decrementAndGetAsync() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongDecrementAndGetCodec::encodeRequest(name);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongDecrementAndGetCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<int64_t> > ClientAtomicLongProxy::getAsync() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongGetCodec::encodeRequest(name);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongGetCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<int64_t> >
            ClientAtomicLongProxy::getAndAddAsync(int64_t delta) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongGetAndAddCodec::encodeRequest(name, delta);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongGetAndAddCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<int64_t> >
            ClientAtomicLongProxy::getAndSetAsync(int64_t newValue) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongGetAndSetCodec::encodeRequest(name, newValue);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongGetAndSetCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<int64_t> > ClientAtomicLongProxy::incrementAndGetAsync() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongIncrementAndGetCodec::encodeRequest(name);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongIncrementAndGetCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<int64_t> > ClientAtomicLongProxy::getAndIncrementAsync() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongGetAndIncrementCodec::encodeRequest(name);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongGetAndIncrementCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<void> > ClientAtomicLongProxy::setAsync(int64_t newValue) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongSetCodec::encodeRequest(name, newValue);

                return invokeOnPartitionAsync<void>(request, impl::VoidMessageDecoder::instance());
            }
        }
    }
}



// Includes for codec classes

namespace hazelcast {
    namespace client {
        namespace proxy {
            MultiMapImpl::MultiMapImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:multiMapService", instanceName, context) {
                // TODO: remove this line once the client instance getDistributedObject works as expected in Java for this proxy type
                lockReferenceIdGenerator = getContext().getLockReferenceIdGenerator();
            }

            bool MultiMapImpl::put(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapPutCodec::encodeRequest(getName(), key, value,
                                                                         util::getCurrentThreadId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapPutCodec::ResponseParameters>(request, key);
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::getData(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapGetCodec::encodeRequest(getName(), key,
                                                                         util::getCurrentThreadId());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapGetCodec::ResponseParameters>(
                        request, key);
            }

            bool MultiMapImpl::remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapRemoveEntryCodec::encodeRequest(getName(), key, value,
                                                                                 util::getCurrentThreadId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapRemoveEntryCodec::ResponseParameters>(request,
                                                                                                               key);
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::removeData(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapRemoveCodec::encodeRequest(getName(), key,
                                                                            util::getCurrentThreadId());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapRemoveCodec::ResponseParameters>(
                        request, key);
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::keySetData() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapKeySetCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapKeySetCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::valuesData() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapValuesCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapValuesCodec::ResponseParameters>(
                        request);
            }

            std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >
            MultiMapImpl::entrySetData() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapEntrySetCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >, protocol::codec::MultiMapEntrySetCodec::ResponseParameters>(
                        request);
            }

            bool MultiMapImpl::containsKey(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapContainsKeyCodec::encodeRequest(getName(), key,
                                                                                 util::getCurrentThreadId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapContainsKeyCodec::ResponseParameters>(request,
                                                                                                               key);
            }

            bool MultiMapImpl::containsValue(const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapContainsValueCodec::encodeRequest(getName(), value);

                return invokeAndGetResult<bool, protocol::codec::MultiMapContainsValueCodec::ResponseParameters>(
                        request);
            }

            bool MultiMapImpl::containsEntry(const serialization::pimpl::Data &key,
                                             const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapContainsEntryCodec::encodeRequest(getName(), key, value,
                                                                                   util::getCurrentThreadId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapContainsEntryCodec::ResponseParameters>(
                        request, key);
            }

            int MultiMapImpl::size() {
                std::unique_ptr<protocol::ClientMessage> request = protocol::codec::MultiMapSizeCodec::encodeRequest(
                        getName());

                return invokeAndGetResult<int, protocol::codec::MultiMapSizeCodec::ResponseParameters>(request);
            }

            void MultiMapImpl::clear() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapClearCodec::encodeRequest(getName());

                invoke(request);
            }

            int MultiMapImpl::valueCount(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapValueCountCodec::encodeRequest(getName(), key,
                                                                                util::getCurrentThreadId());

                return invokeAndGetResult<int, protocol::codec::MultiMapValueCountCodec::ResponseParameters>(request,
                                                                                                             key);
            }

            std::string MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                return registerListener(createMultiMapEntryListenerCodec(includeValue), entryEventHandler);
            }

            std::string MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler,
                                                       serialization::pimpl::Data &key, bool includeValue) {
                return registerListener(createMultiMapEntryListenerCodec(includeValue, key), entryEventHandler);
            }

            bool MultiMapImpl::removeEntryListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            void MultiMapImpl::lock(const serialization::pimpl::Data &key) {
                lock(key, -1);
            }

            void MultiMapImpl::lock(const serialization::pimpl::Data &key, long leaseTime) {
                int partitionId = getPartitionId(key);

                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapLockCodec::encodeRequest(getName(), key, util::getCurrentThreadId(),
                                                                          leaseTime,
                                                                          lockReferenceIdGenerator->getNextReferenceId());

                invokeOnPartition(request, partitionId);
            }


            bool MultiMapImpl::isLocked(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapIsLockedCodec::encodeRequest(getName(), key);

                return invokeAndGetResult<bool, protocol::codec::MultiMapIsLockedCodec::ResponseParameters>(request,
                                                                                                            key);
            }

            bool MultiMapImpl::tryLock(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapTryLockCodec::encodeRequest(getName(), key,
                                                                             util::getCurrentThreadId(), LONG_MAX,
                                                                             0,
                                                                             lockReferenceIdGenerator->getNextReferenceId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapTryLockCodec::ResponseParameters>(request,
                                                                                                           key);
            }

            bool MultiMapImpl::tryLock(const serialization::pimpl::Data &key, long timeInMillis) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapTryLockCodec::encodeRequest(getName(), key, util::getCurrentThreadId(),
                                                                             LONG_MAX, timeInMillis,
                                                                             lockReferenceIdGenerator->getNextReferenceId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapTryLockCodec::ResponseParameters>(request,
                                                                                                           key);
            }

            void MultiMapImpl::unlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapUnlockCodec::encodeRequest(getName(), key, util::getCurrentThreadId(),
                                                                            lockReferenceIdGenerator->getNextReferenceId());

                invokeOnPartition(request, partitionId);
            }

            void MultiMapImpl::forceUnlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapForceUnlockCodec::encodeRequest(getName(), key,
                                                                                 lockReferenceIdGenerator->getNextReferenceId());

                invokeOnPartition(request, partitionId);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            MultiMapImpl::createMultiMapEntryListenerCodec(bool includeValue) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MultiMapEntryListenerMessageCodec(getName(), includeValue));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            MultiMapImpl::createMultiMapEntryListenerCodec(bool includeValue, serialization::pimpl::Data &key) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MultiMapEntryListenerToKeyCodec(getName(), includeValue, key));
            }

            void MultiMapImpl::onInitialize() {
                ProxyImpl::onInitialize();

                lockReferenceIdGenerator = getContext().getLockReferenceIdGenerator();
            }

            MultiMapImpl::MultiMapEntryListenerMessageCodec::MultiMapEntryListenerMessageCodec(const std::string &name,
                                                                                               bool includeValue)
                    : name(name),
                      includeValue(
                              includeValue) {
            }

            std::unique_ptr<protocol::ClientMessage>
            MultiMapImpl::MultiMapEntryListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MultiMapAddEntryListenerCodec::encodeRequest(name, includeValue, localOnly);
            }

            std::string MultiMapImpl::MultiMapEntryListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MultiMapAddEntryListenerCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            MultiMapImpl::MultiMapEntryListenerMessageCodec::encodeRemoveRequest(
                    const std::string &realRegistrationId) const {
                return protocol::codec::MultiMapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool MultiMapImpl::MultiMapEntryListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MultiMapRemoveEntryListenerCodec::ResponseParameters::decode(
                        clientMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            MultiMapImpl::MultiMapEntryListenerToKeyCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MultiMapAddEntryListenerToKeyCodec::encodeRequest(name, key, includeValue,
                                                                                          localOnly);
            }

            std::string MultiMapImpl::MultiMapEntryListenerToKeyCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MultiMapAddEntryListenerToKeyCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            MultiMapImpl::MultiMapEntryListenerToKeyCodec::encodeRemoveRequest(
                    const std::string &realRegistrationId) const {
                return protocol::codec::MultiMapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool MultiMapImpl::MultiMapEntryListenerToKeyCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MultiMapRemoveEntryListenerCodec::ResponseParameters::decode(
                        clientMessage).response;
            }

            MultiMapImpl::MultiMapEntryListenerToKeyCodec::MultiMapEntryListenerToKeyCodec(const std::string &name,
                                                                                           bool includeValue,
                                                                                           serialization::pimpl::Data &key)
                    : name(name), includeValue(includeValue), key(key) {}


        }
    }
}




// Includes for parameters classes

namespace hazelcast {
    namespace client {
        namespace proxy {
            IMapImpl::IMapImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:mapService", instanceName, context) {
            }

            bool IMapImpl::containsKey(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapContainsKeyCodec::encodeRequest(getName(), key,
                                                                            util::getCurrentThreadId());

                return invokeAndGetResult<bool, protocol::codec::MapContainsKeyCodec::ResponseParameters>(request,
                                                                                                          key);
            }

            bool IMapImpl::containsValue(const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapContainsValueCodec::encodeRequest(getName(), value);

                return invokeAndGetResult<bool, protocol::codec::MapContainsValueCodec::ResponseParameters>(request);
            }

            std::unique_ptr<serialization::pimpl::Data> IMapImpl::getData(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapGetCodec::encodeRequest(getName(), key, util::getCurrentThreadId());

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::MapGetCodec::ResponseParameters>(
                        request, key);
            }

            std::unique_ptr<serialization::pimpl::Data> IMapImpl::removeData(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapRemoveCodec::encodeRequest(getName(), key, util::getCurrentThreadId());

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::MapRemoveCodec::ResponseParameters>(
                        request, key);
            }

            bool IMapImpl::remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapRemoveIfSameCodec::encodeRequest(getName(), key, value,
                                                                             util::getCurrentThreadId());

                return invokeAndGetResult<bool, protocol::codec::MapRemoveIfSameCodec::ResponseParameters>(request,
                                                                                                           key);
            }

            void IMapImpl::removeAll(const serialization::pimpl::Data &predicateData) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapRemoveAllCodec::encodeRequest(getName(), predicateData);

                invoke(request);
            }

            void IMapImpl::deleteEntry(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapDeleteCodec::encodeRequest(getName(), key, util::getCurrentThreadId());

                invokeOnPartition(request, partitionId);
            }

            void IMapImpl::flush() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapFlushCodec::encodeRequest(getName());

                invoke(request);
            }

            bool IMapImpl::tryRemove(const serialization::pimpl::Data &key, int64_t timeoutInMillis) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapTryRemoveCodec::encodeRequest(getName(), key,
                                                                          util::getCurrentThreadId(),
                                                                          timeoutInMillis);

                return invokeAndGetResult<bool, protocol::codec::MapTryRemoveCodec::ResponseParameters>(request, key);
            }

            bool IMapImpl::tryPut(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                  int64_t timeoutInMillis) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapTryPutCodec::encodeRequest(getName(), key, value,
                                                                       util::getCurrentThreadId(),
                                                                       timeoutInMillis);

                return invokeAndGetResult<bool, protocol::codec::MapTryPutCodec::ResponseParameters>(request, key);
            }

            std::unique_ptr<serialization::pimpl::Data> IMapImpl::putData(const serialization::pimpl::Data &key,
                                                                          const serialization::pimpl::Data &value,
                                                                          int64_t ttlInMillis) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapPutCodec::encodeRequest(getName(), key, value,
                                                                    util::getCurrentThreadId(),
                                                                    ttlInMillis);

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::MapPutCodec::ResponseParameters>(
                        request, key);
            }

            void IMapImpl::putTransient(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                        int64_t ttlInMillis) {
                int partitionId = getPartitionId(key);

                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapPutTransientCodec::encodeRequest(getName(), key, value,
                                                                             util::getCurrentThreadId(),
                                                                             ttlInMillis);

                invokeOnPartition(request, partitionId);
            }

            std::unique_ptr<serialization::pimpl::Data> IMapImpl::putIfAbsentData(const serialization::pimpl::Data &key,
                                                                                  const serialization::pimpl::Data &value,
                                                                                  int64_t ttlInMillis) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapPutIfAbsentCodec::encodeRequest(getName(), key, value,
                                                                            util::getCurrentThreadId(),
                                                                            ttlInMillis);

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::MapPutIfAbsentCodec::ResponseParameters>(
                        request, key);
            }

            bool IMapImpl::replace(const serialization::pimpl::Data &key, const serialization::pimpl::Data &oldValue,
                                   const serialization::pimpl::Data &newValue) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapReplaceIfSameCodec::encodeRequest(getName(), key, oldValue,
                                                                              newValue,
                                                                              util::getCurrentThreadId());

                return invokeAndGetResult<bool, protocol::codec::MapReplaceIfSameCodec::ResponseParameters>(request,
                                                                                                            key);
            }

            std::unique_ptr<serialization::pimpl::Data> IMapImpl::replaceData(const serialization::pimpl::Data &key,
                                                                              const serialization::pimpl::Data &value) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapReplaceCodec::encodeRequest(getName(), key, value,
                                                                        util::getCurrentThreadId());

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::MapReplaceCodec::ResponseParameters>(
                        request, key);
            }

            void IMapImpl::set(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                               int64_t ttl) {
                int partitionId = getPartitionId(key);

                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapSetCodec::encodeRequest(getName(), key, value,
                                                                    util::getCurrentThreadId(), ttl);

                invokeOnPartition(request, partitionId);
            }

            void IMapImpl::lock(const serialization::pimpl::Data &key) {
                lock(key, -1);
            }

            void IMapImpl::lock(const serialization::pimpl::Data &key, int64_t leaseTime) {
                int partitionId = getPartitionId(key);

                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapLockCodec::encodeRequest(getName(), key, util::getCurrentThreadId(),
                                                                     leaseTime,
                                                                     lockReferenceIdGenerator->getNextReferenceId());

                invokeOnPartition(request, partitionId);
            }

            bool IMapImpl::isLocked(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapIsLockedCodec::encodeRequest(getName(), key);

                return invokeAndGetResult<bool, protocol::codec::MapIsLockedCodec::ResponseParameters>(request, key);
            }

            bool IMapImpl::tryLock(const serialization::pimpl::Data &key, int64_t timeInMillis) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapTryLockCodec::encodeRequest(getName(), key, util::getCurrentThreadId(), -1,
                                                                        timeInMillis,
                                                                        lockReferenceIdGenerator->getNextReferenceId());

                return invokeAndGetResult<bool, protocol::codec::MapTryLockCodec::ResponseParameters>(request, key);
            }

            void IMapImpl::unlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapUnlockCodec::encodeRequest(getName(), key, util::getCurrentThreadId(),
                                                                       lockReferenceIdGenerator->getNextReferenceId());

                invokeOnPartition(request, partitionId);
            }

            void IMapImpl::forceUnlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapForceUnlockCodec::encodeRequest(getName(), key,
                                                                            lockReferenceIdGenerator->getNextReferenceId());

                invokeOnPartition(request, partitionId);
            }

            std::string IMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                // TODO: Use appropriate flags for the event type as implemented in Java instead of EntryEventType::ALL
                int32_t listenerFlags = EntryEventType::ALL;
                return registerListener(createMapEntryListenerCodec(includeValue, listenerFlags), entryEventHandler);
            }

            std::string
            IMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, const query::Predicate &predicate,
                                       bool includeValue) {
                // TODO: Use appropriate flags for the event type as implemented in Java instead of EntryEventType::ALL
                int32_t listenerFlags = EntryEventType::ALL;
                serialization::pimpl::Data predicateData = toData<serialization::IdentifiedDataSerializable>(predicate);
                return registerListener(createMapEntryListenerCodec(includeValue, predicateData, listenerFlags),
                                        entryEventHandler);
            }

            bool IMapImpl::removeEntryListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            std::string IMapImpl::addEntryListener(impl::BaseEventHandler *handler,
                                                   serialization::pimpl::Data &key, bool includeValue) {
                // TODO: Use appropriate flags for the event type as implemented in Java instead of EntryEventType::ALL
                int32_t listenerFlags = EntryEventType::ALL;
                return registerListener(createMapEntryListenerCodec(includeValue, listenerFlags, key), handler);

            }

            std::unique_ptr<map::DataEntryView> IMapImpl::getEntryViewData(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapGetEntryViewCodec::encodeRequest(getName(), key,
                                                                             util::getCurrentThreadId());

                return invokeAndGetResult<std::unique_ptr<map::DataEntryView>, protocol::codec::MapGetEntryViewCodec::ResponseParameters>(
                        request, key);
            }

            bool IMapImpl::evict(const serialization::pimpl::Data &key) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapEvictCodec::encodeRequest(getName(), key, util::getCurrentThreadId());

                return invokeAndGetResult<bool, protocol::codec::MapEvictCodec::ResponseParameters>(request,
                                                                                                    key);
            }

            void IMapImpl::evictAll() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapEvictAllCodec::encodeRequest(getName());

                invoke(request);
            }

            EntryVector
            IMapImpl::getAllData(const std::map<int, std::vector<serialization::pimpl::Data> > &partitionToKeyData) {
                std::vector<std::shared_ptr<spi::impl::ClientInvocationFuture> > futures;

                for (std::map<int, std::vector<serialization::pimpl::Data> >::const_iterator it = partitionToKeyData.begin();
                     it != partitionToKeyData.end(); ++it) {
                    std::unique_ptr<protocol::ClientMessage> request =
                            protocol::codec::MapGetAllCodec::encodeRequest(getName(), it->second);

                    futures.push_back(invokeAndGetFuture(request, it->first));
                }

                EntryVector result;
                // wait for all futures
                for (const std::shared_ptr<spi::impl::ClientInvocationFuture> &future : futures) {
                    std::shared_ptr<protocol::ClientMessage> responseForPartition = future->get();
                    protocol::codec::MapGetAllCodec::ResponseParameters resultForPartition =
                            protocol::codec::MapGetAllCodec::ResponseParameters::decode(
                                    *responseForPartition);
                    result.insert(result.end(), resultForPartition.response.begin(),
                                  resultForPartition.response.end());

                }

                return result;
            }

            std::vector<serialization::pimpl::Data> IMapImpl::keySetData() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapKeySetCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MapKeySetCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> IMapImpl::keySetData(
                    const serialization::IdentifiedDataSerializable &predicate) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapKeySetWithPredicateCodec::encodeRequest(getName(),
                                                                                    toData<serialization::IdentifiedDataSerializable>(
                                                                                            predicate));

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MapKeySetWithPredicateCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> IMapImpl::keySetForPagingPredicateData(
                    const serialization::IdentifiedDataSerializable &predicate) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapKeySetWithPagingPredicateCodec::encodeRequest(getName(),
                                                                                          toData<serialization::IdentifiedDataSerializable>(
                                                                                                  predicate));

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MapKeySetWithPagingPredicateCodec::ResponseParameters>(
                        request);
            }

            EntryVector IMapImpl::entrySetData() {

                std::unique_ptr<protocol::ClientMessage> request = protocol::codec::MapEntrySetCodec::encodeRequest(
                        getName());

                return invokeAndGetResult<EntryVector, protocol::codec::MapEntrySetCodec::ResponseParameters>(
                        request);
            }

            EntryVector IMapImpl::entrySetData(
                    const serialization::IdentifiedDataSerializable &predicate) {

                std::unique_ptr<protocol::ClientMessage> request = protocol::codec::MapEntriesWithPredicateCodec::encodeRequest(
                        getName(), toData(predicate));

                return invokeAndGetResult<EntryVector, protocol::codec::MapEntriesWithPredicateCodec::ResponseParameters>(
                        request);
            }

            EntryVector IMapImpl::entrySetForPagingPredicateData(
                    const serialization::IdentifiedDataSerializable &predicate) {

                std::unique_ptr<protocol::ClientMessage> request = protocol::codec::MapEntriesWithPagingPredicateCodec::encodeRequest(
                        getName(), toData(predicate));

                return invokeAndGetResult<EntryVector, protocol::codec::MapEntriesWithPagingPredicateCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> IMapImpl::valuesData() {

                std::unique_ptr<protocol::ClientMessage> request = protocol::codec::MapValuesCodec::encodeRequest(
                        getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MapValuesCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> IMapImpl::valuesData(
                    const serialization::IdentifiedDataSerializable &predicate) {

                std::unique_ptr<protocol::ClientMessage> request = protocol::codec::MapValuesWithPredicateCodec::encodeRequest(
                        getName(), toData<serialization::IdentifiedDataSerializable>(predicate));

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MapValuesWithPredicateCodec::ResponseParameters>(
                        request);
            }

            EntryVector
            IMapImpl::valuesForPagingPredicateData(const serialization::IdentifiedDataSerializable &predicate) {

                std::unique_ptr<protocol::ClientMessage> request = protocol::codec::MapValuesWithPagingPredicateCodec::encodeRequest(
                        getName(), toData<serialization::IdentifiedDataSerializable>(predicate));

                return invokeAndGetResult<EntryVector, protocol::codec::MapValuesWithPagingPredicateCodec::ResponseParameters>(
                        request);
            }

            void IMapImpl::addIndex(const std::string &attribute, bool ordered) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapAddIndexCodec::encodeRequest(getName(), attribute, ordered);

                invoke(request);
            }

            int IMapImpl::size() {
                std::unique_ptr<protocol::ClientMessage> request = protocol::codec::MapSizeCodec::encodeRequest(
                        getName());

                return invokeAndGetResult<int, protocol::codec::MapSizeCodec::ResponseParameters>(request);
            }

            bool IMapImpl::isEmpty() {
                std::unique_ptr<protocol::ClientMessage> request = protocol::codec::MapIsEmptyCodec::encodeRequest(
                        getName());

                return invokeAndGetResult<bool, protocol::codec::MapIsEmptyCodec::ResponseParameters>(request);
            }

            void IMapImpl::putAllData(const std::map<int, EntryVector> &partitionedEntries) {
                std::vector<std::shared_ptr<spi::impl::ClientInvocationFuture> > futures;

                for (std::map<int, EntryVector>::const_iterator it = partitionedEntries.begin();
                     it != partitionedEntries.end(); ++it) {
                    std::unique_ptr<protocol::ClientMessage> request =
                            protocol::codec::MapPutAllCodec::encodeRequest(getName(), it->second);

                    futures.push_back(invokeAndGetFuture(request, it->first));
                }

                // wait for all futures
                for (const std::shared_ptr<spi::impl::ClientInvocationFuture> &future : futures) {
                    future->get();
                }
            }

            void IMapImpl::clear() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapClearCodec::encodeRequest(getName());

                invoke(request);
            }

            std::unique_ptr<serialization::pimpl::Data>
            IMapImpl::executeOnKeyData(const serialization::pimpl::Data &key,
                                       const serialization::pimpl::Data &processor) {
                int partitionId = getPartitionId(key);

                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapExecuteOnKeyCodec::encodeRequest(getName(),
                                                                             processor,
                                                                             key,
                                                                             util::getCurrentThreadId());

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>,
                        protocol::codec::MapExecuteOnKeyCodec::ResponseParameters>(request, partitionId);
            }

            EntryVector IMapImpl::executeOnKeysData(const std::vector<serialization::pimpl::Data> &keys,
                                                    const serialization::pimpl::Data &processor) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapExecuteOnKeysCodec::encodeRequest(getName(), processor, keys);

                return invokeAndGetResult<EntryVector,
                        protocol::codec::MapExecuteOnKeysCodec::ResponseParameters>(request);
            }

            std::string IMapImpl::addInterceptor(serialization::Portable &interceptor) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapAddInterceptorCodec::encodeRequest(
                                getName(), toData<serialization::Portable>(interceptor));

                return invokeAndGetResult<std::string, protocol::codec::MapAddInterceptorCodec::ResponseParameters>(
                        request);
            }

            std::string IMapImpl::addInterceptor(serialization::IdentifiedDataSerializable &interceptor) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapAddInterceptorCodec::encodeRequest(
                                getName(), toData<serialization::IdentifiedDataSerializable>(interceptor));

                return invokeAndGetResult<std::string, protocol::codec::MapAddInterceptorCodec::ResponseParameters>(
                        request);
            }

            void IMapImpl::removeInterceptor(const std::string &id) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapRemoveInterceptorCodec::encodeRequest(getName(), id);

                invoke(request);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::createMapEntryListenerCodec(bool includeValue, serialization::pimpl::Data &predicate,
                                                  int32_t listenerFlags) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerWithPredicateMessageCodec(getName(), includeValue, listenerFlags,
                                                                      predicate));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::createMapEntryListenerCodec(bool includeValue, int32_t listenerFlags) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerMessageCodec(getName(), includeValue, listenerFlags));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::createMapEntryListenerCodec(bool includeValue, int32_t listenerFlags,
                                                  serialization::pimpl::Data &key) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerToKeyCodec(getName(), includeValue, listenerFlags, key));
            }

            void IMapImpl::onInitialize() {
                ProxyImpl::onInitialize();

                lockReferenceIdGenerator = getContext().getLockReferenceIdGenerator();
            }

            IMapImpl::MapEntryListenerMessageCodec::MapEntryListenerMessageCodec(const std::string &name,
                                                                                 bool includeValue,
                                                                                 int32_t listenerFlags) : name(name),
                                                                                                          includeValue(
                                                                                                                  includeValue),
                                                                                                          listenerFlags(
                                                                                                                  listenerFlags) {}

            std::unique_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MapAddEntryListenerCodec::encodeRequest(name, includeValue, listenerFlags,
                                                                                localOnly);
            }

            std::string IMapImpl::MapEntryListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MapAddEntryListenerCodec::ResponseParameters::decode(responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::MapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool IMapImpl::MapEntryListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MapRemoveEntryListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerToKeyCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MapAddEntryListenerToKeyCodec::encodeRequest(name, key, includeValue,
                                                                                     listenerFlags, localOnly);
            }

            std::string IMapImpl::MapEntryListenerToKeyCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MapAddEntryListenerToKeyCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerToKeyCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::MapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool IMapImpl::MapEntryListenerToKeyCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MapRemoveEntryListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

            IMapImpl::MapEntryListenerToKeyCodec::MapEntryListenerToKeyCodec(const std::string &name, bool includeValue,
                                                                             int32_t listenerFlags,
                                                                             const serialization::pimpl::Data &key)
                    : name(name), includeValue(includeValue), listenerFlags(listenerFlags), key(key) {}

            IMapImpl::MapEntryListenerWithPredicateMessageCodec::MapEntryListenerWithPredicateMessageCodec(
                    const std::string &name, bool includeValue, int32_t listenerFlags,
                    serialization::pimpl::Data &predicate) : name(name), includeValue(includeValue),
                                                             listenerFlags(listenerFlags), predicate(predicate) {}

            std::unique_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerWithPredicateMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MapAddEntryListenerWithPredicateCodec::encodeRequest(name, predicate,
                                                                                             includeValue,
                                                                                             listenerFlags, localOnly);
            }

            std::string IMapImpl::MapEntryListenerWithPredicateMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MapAddEntryListenerWithPredicateCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerWithPredicateMessageCodec::encodeRemoveRequest(
                    const std::string &realRegistrationId) const {
                return protocol::codec::MapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool IMapImpl::MapEntryListenerWithPredicateMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MapRemoveEntryListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

            std::shared_ptr<spi::impl::ClientInvocationFuture>
            IMapImpl::putAsyncInternalData(int64_t ttl, const util::concurrent::TimeUnit &ttlUnit,
                                           const int64_t *maxIdle, const util::concurrent::TimeUnit &maxIdleUnit,
                                           const serialization::pimpl::Data &keyData,
                                           const serialization::pimpl::Data &valueData) {
                int64_t ttlMillis = hazelcast::util::TimeUtil::timeInMsOrOneIfResultIsZero(ttl, ttlUnit);
                std::unique_ptr<protocol::ClientMessage> request;
                if (maxIdle != NULL) {
                    request = protocol::codec::MapPutWithMaxIdleCodec::encodeRequest(name, keyData, valueData,
                                                                                     getCurrentThreadId(),
                                                                                     ttlMillis,
                                                                                     TimeUtil::timeInMsOrOneIfResultIsZero(
                                                                                             *maxIdle,
                                                                                             maxIdleUnit));
                } else {
                    request = protocol::codec::MapPutCodec::encodeRequest(name, keyData, valueData,
                                                                          getCurrentThreadId(),
                                                                          ttlMillis);
                }

                return invokeOnKeyOwner(request, keyData);
            }

            std::shared_ptr<spi::impl::ClientInvocationFuture>
            IMapImpl::setAsyncInternalData(int64_t ttl, const util::concurrent::TimeUnit &ttlUnit,
                                           const int64_t *maxIdle, const util::concurrent::TimeUnit &maxIdleUnit,
                                           const serialization::pimpl::Data &keyData,
                                           const serialization::pimpl::Data &valueData) {
                int64_t ttlMillis = TimeUtil::timeInMsOrOneIfResultIsZero(ttl, ttlUnit);
                std::unique_ptr<protocol::ClientMessage> request;
                if (maxIdle != NULL) {
                    request = protocol::codec::MapSetWithMaxIdleCodec::encodeRequest(name, keyData, valueData,
                                                                                     getCurrentThreadId(),
                                                                                     ttlMillis,
                                                                                     TimeUtil::timeInMsOrOneIfResultIsZero(
                                                                                             *maxIdle,
                                                                                             maxIdleUnit));
                } else {
                    request = protocol::codec::MapSetCodec::encodeRequest(name, keyData, valueData,
                                                                          getCurrentThreadId(),
                                                                          ttlMillis);
                }

                return invokeOnKeyOwner(request, keyData);
            }

        }
    }
}




// Includes for parameters classes

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalQueueImpl::TransactionalQueueImpl(const std::string &name,
                                                           txn::TransactionProxy *transactionProxy)
                    : TransactionalObject("hz:impl:queueService", name, transactionProxy) {

            }

            bool TransactionalQueueImpl::offer(const serialization::pimpl::Data &e, long timeoutInMillis) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalQueueOfferCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e, timeoutInMillis);

                return invokeAndGetResult<bool, protocol::codec::TransactionalQueueOfferCodec::ResponseParameters>(
                        request);
            }

            std::unique_ptr<serialization::pimpl::Data> TransactionalQueueImpl::pollData(long timeoutInMillis) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalQueuePollCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), timeoutInMillis);

                return invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::TransactionalQueuePollCodec::ResponseParameters>(
                        request);
            }

            int TransactionalQueueImpl::size() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalQueueSizeCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetResult<int, protocol::codec::TransactionalQueueSizeCodec::ResponseParameters>(
                        request);
            }
        }
    }
}





// Includes for parameters classes

namespace hazelcast {
    namespace client {
        namespace proxy {
            ISetImpl::ISetImpl(const std::string &instanceName, spi::ClientContext *clientContext)
                    : ProxyImpl("hz:impl:setService", instanceName, clientContext) {
                serialization::pimpl::Data keyData = getContext().getSerializationService().toData<std::string>(
                        &instanceName);
                partitionId = getPartitionId(keyData);
            }

            std::string ISetImpl::addItemListener(impl::BaseEventHandler *itemEventHandler, bool includeValue) {
                return registerListener(createItemListenerCodec(includeValue), itemEventHandler);
            }

            bool ISetImpl::removeItemListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            int ISetImpl::size() {
                std::unique_ptr<protocol::ClientMessage> request = protocol::codec::SetSizeCodec::encodeRequest(
                        getName());

                return invokeAndGetResult<int, protocol::codec::SetSizeCodec::ResponseParameters>(request, partitionId);
            }

            bool ISetImpl::isEmpty() {
                std::unique_ptr<protocol::ClientMessage> request = protocol::codec::SetIsEmptyCodec::encodeRequest(
                        getName());

                return invokeAndGetResult<bool, protocol::codec::SetIsEmptyCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            bool ISetImpl::contains(const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetContainsCodec::encodeRequest(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::SetContainsCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            std::vector<serialization::pimpl::Data> ISetImpl::toArrayData() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetGetAllCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::SetGetAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool ISetImpl::add(const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetAddCodec::encodeRequest(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::SetAddCodec::ResponseParameters>(request, partitionId);
            }

            bool ISetImpl::remove(const serialization::pimpl::Data &element) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetRemoveCodec::encodeRequest(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::SetRemoveCodec::ResponseParameters>(request,
                                                                                                     partitionId);
            }

            bool ISetImpl::containsAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetContainsAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::SetContainsAllCodec::ResponseParameters>(request,
                                                                                                          partitionId);
            }

            bool ISetImpl::addAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetAddAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::SetAddAllCodec::ResponseParameters>(request,
                                                                                                     partitionId);
            }

            bool ISetImpl::removeAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetCompareAndRemoveAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::SetCompareAndRemoveAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool ISetImpl::retainAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetCompareAndRetainAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::SetCompareAndRetainAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            void ISetImpl::clear() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetClearCodec::encodeRequest(getName());

                invokeOnPartition(request, partitionId);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            ISetImpl::createItemListenerCodec(bool includeValue) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new SetListenerMessageCodec(getName(), includeValue));
            }

            ISetImpl::SetListenerMessageCodec::SetListenerMessageCodec(const std::string &name,
                                                                       bool includeValue) : name(name),
                                                                                            includeValue(
                                                                                                    includeValue) {}

            std::unique_ptr<protocol::ClientMessage>
            ISetImpl::SetListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::SetAddListenerCodec::encodeRequest(name, includeValue, localOnly);
            }

            std::string ISetImpl::SetListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::SetAddListenerCodec::ResponseParameters::decode(responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            ISetImpl::SetListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::SetRemoveListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool ISetImpl::SetListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::SetRemoveListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

        }
    }
}




// Includes for parameters classes

namespace hazelcast {
    namespace client {
        namespace proxy {
            ITopicImpl::ITopicImpl(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ProxyImpl("hz:impl:topicService", instanceName, context) {
                partitionId = getPartitionId(toData(instanceName));
            }

            void ITopicImpl::publish(const serialization::pimpl::Data &data) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::TopicPublishCodec::encodeRequest(getName(), data);

                invokeOnPartition(request, partitionId);
            }

            std::string ITopicImpl::addMessageListener(impl::BaseEventHandler *topicEventHandler) {
                return registerListener(createItemListenerCodec(), topicEventHandler);
            }

            bool ITopicImpl::removeMessageListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec> ITopicImpl::createItemListenerCodec() {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(new TopicListenerMessageCodec(getName()));
            }

            ITopicImpl::TopicListenerMessageCodec::TopicListenerMessageCodec(const std::string &name) : name(name) {}

            std::unique_ptr<protocol::ClientMessage>
            ITopicImpl::TopicListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::TopicAddMessageListenerCodec::encodeRequest(name, localOnly);
            }

            std::string ITopicImpl::TopicListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::TopicAddMessageListenerCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            ITopicImpl::TopicListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::TopicRemoveMessageListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool ITopicImpl::TopicListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::TopicRemoveMessageListenerCodec::ResponseParameters::decode(
                        clientMessage).response;
            }

        }
    }
}


namespace hazelcast {
    namespace client {
        TypedData::TypedData() : ss(NULL) {
        }

        TypedData::TypedData(std::unique_ptr<serialization::pimpl::Data> &data,
                             serialization::pimpl::SerializationService &serializationService) : data(std::move(data)),
                                                                                                 ss(&serializationService) {
        }

        TypedData::TypedData(const std::shared_ptr<serialization::pimpl::Data> &data,
                             serialization::pimpl::SerializationService &serializationService) : data(data),
                                                                                                 ss(&serializationService) {

        }

        TypedData::~TypedData() {}

        const serialization::pimpl::ObjectType TypedData::getType() const {
            return ss->getObjectType(data.get());
        }

        const std::shared_ptr<serialization::pimpl::Data> TypedData::getData() const {
            return data;
        }

        bool operator<(const TypedData &lhs, const TypedData &rhs) {
            const serialization::pimpl::Data *lhsData = lhs.getData().get();
            const serialization::pimpl::Data *rhsData = rhs.getData().get();
            if (lhsData == NULL) {
                return true;
            }

            if (rhsData == NULL) {
                return false;
            }

            return *lhsData < *rhsData;
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace idgen {
            namespace impl {
                IdGeneratorProxyFactory::IdGeneratorProxyFactory(spi::ClientContext *clientContext) : clientContext(
                        clientContext) {}

                std::shared_ptr<spi::ClientProxy> IdGeneratorProxyFactory::create(const std::string &id) {
                    IAtomicLong atomicLong = clientContext->getHazelcastClientImplementation()->getIAtomicLong(
                            proxy::ClientIdGeneratorProxy::ATOMIC_LONG_NAME + id);
                    return std::shared_ptr<spi::ClientProxy>(
                            new proxy::ClientIdGeneratorProxy(id, clientContext, atomicLong));
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

