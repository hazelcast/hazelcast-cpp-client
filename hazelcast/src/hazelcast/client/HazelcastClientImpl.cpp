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
#include <asio.hpp>
#include <boost/date_time.hpp>
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
#include "hazelcast/client/connection/InSelector.h"
#include "hazelcast/client/internal/socket/TcpSocket.h"
#include "hazelcast/client/connection/IOSelector.h"
#include "hazelcast/client/connection/ListenerTask.h"
#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/util/ServerSocket.h"
#include "hazelcast/client/spi/impl/listener/AbstractClientListenerService.h"
#include "hazelcast/client/connection/WriteHandler.h"
#include "hazelcast/client/connection/OutSelector.h"
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
                        } catch (exception::IException &) {
                            // ignore
                        }
                        throw exception::IllegalStateException("HazelcastClient",
                                                               "HazelcastClient could not be started!");
                    }
                } catch (exception::IException &) {
                    try {
                        lifecycleService.shutdown();
                    } catch (exception::IException &) {
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
        Socket::~Socket() {
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
                    util::ExceptionUtil::rethrow(e, TRANSACTION_EXCEPTION_FACTORY());
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




namespace hazelcast {
    namespace client {
        namespace connection {
            int ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC = 2;
            int ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC = 20;

            ClientConnectionManagerImpl::ClientConnectionManagerImpl(spi::ClientContext &client,
                                                                     const std::shared_ptr<AddressTranslator> &addressTranslator,
                                                                     const std::vector<std::shared_ptr<AddressProvider> > &addressProviders)
                    : logger(client.getLogger()), client(client),
                      socketInterceptor(client.getClientConfig().getSocketInterceptor()),
                      inSelector(*this, client.getClientConfig().getNetworkConfig().getSocketOptions()),
                      outSelector(*this, client.getClientConfig().getNetworkConfig().getSocketOptions()),
                      inSelectorThread(std::shared_ptr<util::Runnable>(new util::RunnableDelegator(inSelector)),
                                       logger),
                      outSelectorThread(std::shared_ptr<util::Runnable>(new util::RunnableDelegator(outSelector)),
                                        logger),
                      executionService(client.getClientExecutionService()),
                      translator(addressTranslator), connectionIdGen(0), socketFactory(client) {
                config::ClientNetworkConfig &networkConfig = client.getClientConfig().getNetworkConfig();

                int64_t connTimeout = networkConfig.getConnectionTimeout();
                connectionTimeoutMillis = connTimeout == 0 ? INT64_MAX : connTimeout;

                credentials = client.getClientConfig().getCredentials();

                connectionStrategy = initializeStrategy(client);

                clusterConnectionExecutor.reset(
                        new util::impl::SimpleExecutorService(logger, client.getName() + ".cluster-", 1));

                ClientProperties &clientProperties = client.getClientProperties();
                shuffleMemberList = clientProperties.getBoolean(clientProperties.getShuffleMemberList());

                ClientConnectionManagerImpl::addressProviders = addressProviders;

                connectionAttemptPeriod = networkConfig.getConnectionAttemptPeriod();

                int connAttemptLimit = networkConfig.getConnectionAttemptLimit();
                bool isAsync = client.getClientConfig().getConnectionStrategyConfig().isAsyncStart();

                if (connAttemptLimit < 0) {
                    this->connectionAttemptLimit = isAsync ? DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC
                                                           : DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC;
                } else {
                    this->connectionAttemptLimit = connAttemptLimit == 0 ? INT32_MAX : connAttemptLimit;
                }

            }

            bool ClientConnectionManagerImpl::start() {
                util::LockGuard guard(lock);
                if (alive) {
                    return true;
                }
                alive.store(true);

                clusterConnectionExecutor->start();

                if (!socketFactory.start()) {
                    return false;
                }

                socketInterceptor = client.getClientConfig().getSocketInterceptor();

                if (!inSelector.start()) {
                    return false;
                }
                if (!outSelector.start()) {
                    return false;
                }

                startEventLoopGroup();
                heartbeat.reset(new HeartbeatManager(client));
                heartbeat->start();
                connectionStrategy->start();

                return true;
            }

            void ClientConnectionManagerImpl::shutdown() {
                util::LockGuard guard(lock);
                if (!alive) {
                    return;
                }
                alive.store(false);

                stopEventLoopGroup();
                heartbeat->shutdown();

                connectionStrategy->shutdown();

                // let the waiting authentication futures not block anymore
                for (auto &authFuture : connectionsInProgress.values()) {
                    authFuture->onFailure(
                            std::make_shared<exception::IllegalStateException>("ClientConnectionManagerImpl::shutdown",
                                                                               "Client is shutting down"));
                }

                // close connections
                for (auto &connection : activeConnections.values()) {
                    // prevent any exceptions
                    util::IOUtil::closeResource(connection.get(), "Hazelcast client is shutting down");
                }

                for (auto &connection : pendingSocketIdToConnection.values()) {
                    // prevent any exceptions
                    util::IOUtil::closeResource(connection.get(), "Hazelcast client is shutting down");
                }

                spi::impl::ClientExecutionServiceImpl::shutdownExecutor("cluster", *clusterConnectionExecutor, logger);

                connectionListeners.clear();
                activeConnectionsFileDescriptors.clear();
                activeConnections.clear();
                socketConnections.clear();
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::getOrConnect(const Address &address) {
                return getOrConnect(address, false);
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::getOwnerConnection() {
                std::shared_ptr<Address> address = ownerConnectionAddress;
                if (address.get() == NULL) {
                    return std::shared_ptr<Connection>();
                }
                std::shared_ptr<Connection> connection = getActiveConnection(*address);
                return connection;

            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::connectAsOwner(const Address &address) {
                std::shared_ptr<Connection> connection;
                try {
                    logger.info("Trying to connect to ", address, " as owner member");
                    connection = getOrConnect(address, true);
                    client.onClusterConnect(connection);
                    fireConnectionEvent(LifecycleEvent::CLIENT_CONNECTED);
                    connectionStrategy->onConnectToCluster();
                } catch (exception::IException &e) {
                    logger.warning("Exception during initial connection to ", address, ", exception ", e);
                    if (NULL != connection.get()) {
                        std::ostringstream reason;
                        reason << "Could not connect to " << address << " as owner";
                        connection->close(reason.str().c_str(), std::shared_ptr<exception::IException>(e.clone()));
                    }
                    return std::shared_ptr<Connection>();
                }
                return connection;
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::createSocketConnection(const Address &address) {
                std::shared_ptr<Connection> conn(
                        new Connection(address, client, ++connectionIdGen, inSelector, outSelector, socketFactory));

                conn->connect(client.getClientConfig().getConnectionTimeout());
                if (socketInterceptor != NULL) {
                    socketInterceptor->onConnect(conn->getSocket());
                }

                return conn;
            }


            std::vector<std::shared_ptr<Connection> > ClientConnectionManagerImpl::getActiveConnections() {
                return activeConnections.values();
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::getOrTriggerConnect(const Address &target) {
                std::shared_ptr<Connection> connection = getConnection(target, false);
                if (connection.get() != NULL) {
                    return connection;
                }
                triggerConnect(target, false);
                return std::shared_ptr<Connection>();
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::getConnection(const Address &target, bool asOwner) {
                if (!asOwner) {
                    connectionStrategy->beforeGetConnection(target);
                }
                if (!asOwner && getOwnerConnection().get() == NULL) {
                    throw exception::IOException("ConnectionManager::getConnection",
                                                 "Owner connection is not available!");
                }

                std::shared_ptr<Connection> connection = activeConnections.get(target);

                if (connection.get() != NULL) {
                    if (!asOwner) {
                        return connection;
                    }
                    if (connection->isAuthenticatedAsOwner()) {
                        return connection;
                    }
                }
                return std::shared_ptr<Connection>();
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::getActiveConnection(const Address &target) {
                return activeConnections.get(target);
            }

            std::shared_ptr<Address> ClientConnectionManagerImpl::getOwnerConnectionAddress() {
                return ownerConnectionAddress;
            }

            std::shared_ptr<AuthenticationFuture>
            ClientConnectionManagerImpl::triggerConnect(const Address &target, bool asOwner) {
                if (!asOwner) {
                    connectionStrategy->beforeOpenConnection(target);
                }
                if (!alive) {
                    throw exception::HazelcastException("ConnectionManager::triggerConnect",
                                                        "ConnectionManager is not active!");
                }

                std::shared_ptr<AuthenticationFuture> future(new AuthenticationFuture());
                std::shared_ptr<AuthenticationFuture> oldFuture = connectionsInProgress.putIfAbsent(target, future);
                if (oldFuture.get() == NULL) {
                    executionService.execute(
                            std::shared_ptr<util::Runnable>(new InitConnectionTask(target, asOwner, future, *this)));
                    return future;
                }
                return oldFuture;
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::getOrConnect(const Address &address, bool asOwner) {
                while (true) {
                    std::shared_ptr<Connection> connection = getConnection(address, asOwner);
                    if (connection.get() != NULL) {
                        return connection;
                    }
                    std::shared_ptr<AuthenticationFuture> firstCallback = triggerConnect(address, asOwner);
                    connection = firstCallback->get();

                    if (!asOwner) {
                        return connection;
                    }
                    if (connection->isAuthenticatedAsOwner()) {
                        return connection;
                    }
                }
            }

            void
            ClientConnectionManagerImpl::authenticate(const Address &target, std::shared_ptr<Connection> &connection,
                                                      bool asOwner, std::shared_ptr<AuthenticationFuture> &future) {
                std::shared_ptr<protocol::Principal> principal = getPrincipal();
                std::unique_ptr<protocol::ClientMessage> clientMessage = encodeAuthenticationRequest(asOwner,
                                                                                                     client.getSerializationService(),
                                                                                                     principal.get());
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        client, clientMessage, "", connection);
                std::shared_ptr<spi::impl::ClientInvocationFuture> invocationFuture = clientInvocation->invokeUrgent();

                auto authCallback = std::make_shared<AuthCallback>(invocationFuture, connection, asOwner, target,
                                                                   future, *this);
                invocationFuture->andThen(authCallback);
            }

            const std::shared_ptr<protocol::Principal> ClientConnectionManagerImpl::getPrincipal() {
                return principal;
            }

            std::unique_ptr<protocol::ClientMessage>
            ClientConnectionManagerImpl::encodeAuthenticationRequest(bool asOwner,
                                                                     serialization::pimpl::SerializationService &ss,
                                                                     const protocol::Principal *principal) {
                byte serializationVersion = ss.getVersion();
                const std::string *uuid = NULL;
                const std::string *ownerUuid = NULL;
                if (principal != NULL) {
                    uuid = principal->getUuid();
                    ownerUuid = principal->getOwnerUuid();
                }
                std::unique_ptr<protocol::ClientMessage> clientMessage;
                if (credentials == NULL) {
                    // TODO: Change UsernamePasswordCredentials to implement Credentials interface so that we can just
                    // upcast the credentials as done at Java
                    GroupConfig &groupConfig = client.getClientConfig().getGroupConfig();
                    const protocol::UsernamePasswordCredentials cr(groupConfig.getName(), groupConfig.getPassword());
                    clientMessage = protocol::codec::ClientAuthenticationCodec::encodeRequest(
                            cr.getPrincipal(), cr.getPassword(), uuid, ownerUuid, asOwner, protocol::ClientTypes::CPP,
                            serializationVersion, HAZELCAST_VERSION);
                } else {
                    serialization::pimpl::Data data = ss.toData<Credentials>(credentials);
                    clientMessage = protocol::codec::ClientAuthenticationCustomCodec::encodeRequest(data,
                                                                                                    uuid,
                                                                                                    ownerUuid,
                                                                                                    asOwner,
                                                                                                    protocol::ClientTypes::CPP,
                                                                                                    serializationVersion,
                                                                                                    HAZELCAST_VERSION);
                }
                return clientMessage;
            }

            void ClientConnectionManagerImpl::setPrincipal(const std::shared_ptr<protocol::Principal> &principal) {
                ClientConnectionManagerImpl::principal = principal;
            }

            void ClientConnectionManagerImpl::onAuthenticated(const Address &target,
                                                              const std::shared_ptr<Connection> &connection) {
                std::shared_ptr<Connection> oldConnection = activeConnections.put(*connection->getRemoteEndpoint(),
                                                                                  connection);
                int socketId = connection->getSocket().getSocketId();
                activeConnectionsFileDescriptors.put(socketId, connection);
                pendingSocketIdToConnection.remove(socketId);

                if (oldConnection.get() == NULL) {
                    if (logger.isFinestEnabled()) {
                        logger.finest("Authentication succeeded for ", *connection,
                                      " and there was no old connection to this end-point");
                    }
                    fireConnectionAddedEvent(connection);
                } else {
                    if (logger.isFinestEnabled()) {
                        logger.finest("Re-authentication succeeded for ", *connection);
                    }
                    assert(*connection == *oldConnection);
                }

                connectionsInProgress.remove(target);
                std::ostringstream out;
                if (connection->getRemoteEndpoint().get()) {
                    out << *connection->getRemoteEndpoint();
                } else {
                    out << "null";
                }
                logger.info("Authenticated with server ", out.str(), ", server version:",
                            connection->getConnectedServerVersionString(), " Local address: ",
                            (connection->getLocalSocketAddress().get() != NULL
                             ? connection->getLocalSocketAddress()->toString() : "null"));

                /* check if connection is closed by remote before authentication complete, if that is the case
                we need to remove it back from active connections.
                Race description from https://github.com/hazelcast/hazelcast/pull/8832.(A little bit changed)
                - open a connection client -> member
                - send auth message
                - receive auth reply -> reply processing is offloaded to an executor. Did not start to run yet.
                - member closes the connection -> the connection is trying to removed from map
                                                                     but it was not there to begin with
                - the executor start processing the auth reply -> it put the connection to the connection map.
                - we end up with a closed connection in activeConnections map */
                if (!connection->isAlive()) {
                    removeFromActiveConnections(connection);
                }
            }

            void
            ClientConnectionManagerImpl::fireConnectionAddedEvent(const std::shared_ptr<Connection> &connection) {
                for (const std::shared_ptr<ConnectionListener> &connectionListener : connectionListeners.toArray()) {
                    connectionListener->connectionAdded(connection);
                }
                connectionStrategy->onConnect(connection);
            }

            void
            ClientConnectionManagerImpl::removeFromActiveConnections(const std::shared_ptr<Connection> &connection) {
                std::shared_ptr<Address> endpoint = connection->getRemoteEndpoint();

                if (endpoint.get() == NULL) {
                    if (logger.isFinestEnabled()) {
                        logger.finest("Destroying ", *connection, ", but it has end-point set to null ",
                                      "-> not removing it from a connection map");
                    }
                    return;
                }

                if (activeConnections.remove(*endpoint, connection)) {
                    logger.info("Removed connection to endpoint: ", *endpoint, ", connection: ", *connection);
                    activeConnectionsFileDescriptors.remove(connection->getSocket().getSocketId());
                    fireConnectionRemovedEvent(connection);
                } else {
                    if (logger.isFinestEnabled()) {
                        logger.finest("Destroying a connection, but there is no mapping ", endpoint, " -> ",
                                      *connection, " in the connection map.");
                    }
                }
            }

            void
            ClientConnectionManagerImpl::fireConnectionRemovedEvent(const std::shared_ptr<Connection> &connection) {
                if (connection->isAuthenticatedAsOwner()) {
                    disconnectFromCluster(connection);
                }

                for (const std::shared_ptr<ConnectionListener> &listener : connectionListeners.toArray()) {
                    listener->connectionRemoved(connection);
                }
                connectionStrategy->onDisconnect(connection);
            }

            void ClientConnectionManagerImpl::disconnectFromCluster(const std::shared_ptr<Connection> &connection) {
                clusterConnectionExecutor->execute(
                        std::shared_ptr<util::Runnable>(
                                new DisconnecFromClusterTask(connection, *this, *connectionStrategy)));
            }

            void
            ClientConnectionManagerImpl::setOwnerConnectionAddress(
                    const std::shared_ptr<Address> &ownerConnectionAddress) {
                previousOwnerConnectionAddress = this->ownerConnectionAddress.get();
                ClientConnectionManagerImpl::ownerConnectionAddress = ownerConnectionAddress;
            }

            void
            ClientConnectionManagerImpl::fireConnectionEvent(
                    const hazelcast::client::LifecycleEvent::LifeCycleState &state) {
                spi::LifecycleService &lifecycleService = client.getLifecycleService();
                lifecycleService.fireLifecycleEvent(state);
            }

            std::shared_ptr<util::Future<bool> > ClientConnectionManagerImpl::connectToClusterAsync() {
                std::shared_ptr<util::Callable<bool> > task(new ConnectToClusterTask(client));
                return clusterConnectionExecutor->submit<bool>(task);
            }

            void ClientConnectionManagerImpl::connectToClusterInternal() {
                int attempt = 0;
                std::set<Address> triedAddresses;

                while (attempt < connectionAttemptLimit) {
                    attempt++;
                    int64_t nextTry = util::currentTimeMillis() + connectionAttemptPeriod;

                    std::set<Address> addresses = getPossibleMemberAddresses();
                    for (const Address &address : addresses) {
                        if (!client.getLifecycleService().isRunning()) {
                            throw exception::IllegalStateException(
                                    "ConnectionManager::connectToClusterInternal",
                                    "Giving up on retrying to connect to cluster since client is shutdown.");
                        }
                        triedAddresses.insert(address);
                        if (connectAsOwner(address).get() != NULL) {
                            return;
                        }
                    }

                    // If the address providers load no addresses (which seems to be possible), then the above loop is not entered
                    // and the lifecycle check is missing, hence we need to repeat the same check at this point.
                    if (!client.getLifecycleService().isRunning()) {
                        throw exception::IllegalStateException("Client is being shutdown.");
                    }

                    if (attempt < connectionAttemptLimit) {
                        const int64_t remainingTime = nextTry - util::currentTimeMillis();
                        logger.warning("Unable to get alive cluster connection, try in ",
                                       (remainingTime > 0 ? remainingTime : 0), " ms later, attempt ", attempt,
                                       " of ", connectionAttemptLimit, ".");

                        if (remainingTime > 0) {
                            util::Thread::sleep(remainingTime);
                        }
                    } else {
                        logger.warning("Unable to get alive cluster connection, attempt ", attempt, " of ",
                                       connectionAttemptLimit, ".");
                    }
                }
                std::ostringstream out;
                out << "Unable to connect to any address! The following addresses were tried: { ";
                for (const std::set<Address>::value_type &address : triedAddresses) {
                    out << address << " , ";
                }
                out << "}";
                throw exception::IllegalStateException("ConnectionManager::connectToClusterInternal", out.str());
            }

            std::set<Address> ClientConnectionManagerImpl::getPossibleMemberAddresses() {
                std::set<Address> addresses;

                std::vector<Member> memberList = client.getClientClusterService().getMemberList();
                std::vector<Address> memberAddresses;
                for (const Member &member : memberList) {
                    memberAddresses.push_back(member.getAddress());
                }

                if (shuffleMemberList) {
                    shuffle(memberAddresses);
                }

                addresses.insert(memberAddresses.begin(), memberAddresses.end());

                std::set<Address> providerAddressesSet;
                for (std::shared_ptr<AddressProvider> &addressProvider : addressProviders) {
                    std::vector<Address> addrList = addressProvider->loadAddresses();
                    providerAddressesSet.insert(addrList.begin(), addrList.end());
                }

                std::vector<Address> providerAddresses(providerAddressesSet.begin(), providerAddressesSet.end());

                if (shuffleMemberList) {
                    shuffle(memberAddresses);
                }

                addresses.insert(providerAddresses.begin(), providerAddresses.end());

                std::shared_ptr<Address> previousAddress = previousOwnerConnectionAddress.get();
                if (previousAddress.get() != NULL) {
                    /*
                     * Previous owner address is moved to last item in set so that client will not try to connect to same one immediately.
                     * It could be the case that address is removed because it is healthy(it not responding to heartbeat/pings)
                     * In that case, trying other addresses first to upgrade make more sense.
                     */
                    addresses.erase(*previousAddress);
                    addresses.insert(*previousAddress);
                }
                return addresses;
            }

            void ClientConnectionManagerImpl::shuffle(
                    std::vector<Address> &memberAddresses) const {// obtain a time-based seed:
                unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
                std::shuffle(memberAddresses.begin(), memberAddresses.end(), std::default_random_engine(seed));
            }

            std::unique_ptr<ClientConnectionStrategy>
            ClientConnectionManagerImpl::initializeStrategy(spi::ClientContext &client) {
                // TODO: Add a way so that this strategy can be configurable as in Java
                return std::unique_ptr<ClientConnectionStrategy>(new DefaultClientConnectionStrategy(client, logger,
                                                                                                     client.getClientConfig().getConnectionStrategyConfig()));
            }

            void ClientConnectionManagerImpl::connectToCluster() {
                connectToClusterAsync()->get();
            }

            void ClientConnectionManagerImpl::startEventLoopGroup() {
                inSelectorThread.start();
                outSelectorThread.start();
            }

            bool ClientConnectionManagerImpl::isAlive() {
                return alive;
            }

            void ClientConnectionManagerImpl::stopEventLoopGroup() {
                inSelector.shutdown();
                inSelectorThread.join();
                outSelector.shutdown();
                outSelectorThread.join();
            }

            void ClientConnectionManagerImpl::onClose(Connection &connection) {
                removeFromActiveConnections(connection.shared_from_this());
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::getActiveConnection(int fileDescriptor) {
                std::shared_ptr<Connection> connection = activeConnectionsFileDescriptors.get(fileDescriptor);
                if (connection.get()) {
                    return connection;
                }

                return pendingSocketIdToConnection.get(fileDescriptor);
            }

            void
            ClientConnectionManagerImpl::addConnectionListener(
                    const std::shared_ptr<ConnectionListener> &connectionListener) {
                connectionListeners.add(connectionListener);
            }

            ClientConnectionManagerImpl::~ClientConnectionManagerImpl() {
                shutdown();
            }

            util::ILogger &ClientConnectionManagerImpl::getLogger() {
                return client.getLogger();
            }

            ClientConnectionManagerImpl::InitConnectionTask::InitConnectionTask(const Address &target,
                                                                                const bool asOwner,
                                                                                const std::shared_ptr<AuthenticationFuture> &future,
                                                                                ClientConnectionManagerImpl &connectionManager)
                    : target(
                    target), asOwner(asOwner), future(future), connectionManager(connectionManager),
                      logger(connectionManager.getLogger()) {}

            void ClientConnectionManagerImpl::InitConnectionTask::run() {
                std::shared_ptr<Connection> connection;
                try {
                    connection = getConnection(target);
                } catch (exception::IException &e) {
                    logger.finest(e);
                    future->onFailure(std::shared_ptr<exception::IException>(e.clone()));
                    connectionManager.connectionsInProgress.remove(target);
                    return;
                }

                try {
                    connectionManager.pendingSocketIdToConnection.put(connection->getSocket().getSocketId(),
                                                                      connection);

                    connection->getReadHandler().registerSocket();

                    connectionManager.authenticate(target, connection, asOwner, future);
                } catch (exception::IException &e) {
                    const std::shared_ptr<exception::IException> throwable(e.clone());
                    future->onFailure(throwable);
                    connection->close("Failed to authenticate connection", throwable);
                    connectionManager.connectionsInProgress.remove(target);
                }
            }

            const std::string ClientConnectionManagerImpl::InitConnectionTask::getName() const {
                return "ConnectionManager::InitConnectionTask";
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::InitConnectionTask::getConnection(const Address &target) {
                std::shared_ptr<Connection> connection = connectionManager.activeConnections.get(target);
                if (connection.get() != NULL) {
                    return connection;
                }
                Address address = connectionManager.translator->translate(target);
                return connectionManager.createSocketConnection(address);
            }

            ClientConnectionManagerImpl::AuthCallback::AuthCallback(
                    std::shared_ptr<spi::impl::ClientInvocationFuture> invocationFuture,
                    const std::shared_ptr<Connection> &connection,
                    bool asOwner,
                    const Address &target,
                    std::shared_ptr<AuthenticationFuture> &future,
                    ClientConnectionManagerImpl &connectionManager) : invocationFuture(invocationFuture),
                                                                      connection(connection), asOwner(asOwner),
                                                                      target(target), future(future),
                                                                      connectionManager(connectionManager),
                                                                      cancelled(false) {
                scheduleTimeoutTask();
            }

            void ClientConnectionManagerImpl::AuthCallback::cancelTimeoutTask() {
                {
                    std::lock_guard<std::mutex> g(timeoutMutex);
                    cancelled = true;
                }
                timeoutCondition.notify_one();
                timeoutTaskFuture.get();
            }

            void ClientConnectionManagerImpl::AuthCallback::scheduleTimeoutTask() {
                timeoutTaskFuture = std::async([&] {
                    std::unique_lock<std::mutex> uniqueLock(timeoutMutex);
                    if (cancelled) {
                        return;
                    }

                    if (timeoutCondition.wait_for(uniqueLock,
                                                  std::chrono::milliseconds(connectionManager.connectionTimeoutMillis),
                                                  [&] { return cancelled; })) {
                        return;
                    }

                    invocationFuture->complete((exception::ExceptionBuilder<exception::TimeoutException>(
                            "ClientConnectionManagerImpl::authenticate")
                            << "Authentication response did not come back in "
                            << connectionManager.connectionTimeoutMillis
                            << " millis").buildShared());
                });
            }

            void ClientConnectionManagerImpl::AuthCallback::onResponse(
                    const std::shared_ptr<protocol::ClientMessage> &response) {
                cancelTimeoutTask();

                std::unique_ptr<protocol::codec::ClientAuthenticationCodec::ResponseParameters> result;
                try {
                    result.reset(new protocol::codec::ClientAuthenticationCodec::ResponseParameters(
                            protocol::codec::ClientAuthenticationCodec::ResponseParameters::decode(*response)));
                } catch (exception::IException &e) {
                    handleAuthenticationException(std::shared_ptr<exception::IException>(e.clone()));
                    return;
                }
                protocol::AuthenticationStatus authenticationStatus = (protocol::AuthenticationStatus) result->status;
                switch (authenticationStatus) {
                    case protocol::AUTHENTICATED: {
                        connection->setConnectedServerVersion(result->serverHazelcastVersion);
                        connection->setRemoteEndpoint(std::shared_ptr<Address>(std::move(result->address)));
                        if (asOwner) {
                            connection->setIsAuthenticatedAsOwner();
                            std::shared_ptr<protocol::Principal> principal(
                                    new protocol::Principal(result->uuid, result->ownerUuid));
                            connectionManager.setPrincipal(principal);
                            //setting owner connection is moved to here(before onAuthenticated/before connected event)
                            //so that invocations that requires owner connection on this connection go through
                            connectionManager.setOwnerConnectionAddress(connection->getRemoteEndpoint());
                            connectionManager.logger.info("Setting ", *connection, " as owner with principal ",
                                                          *principal);
                        }
                        connectionManager.onAuthenticated(target, connection);
                        future->onSuccess(connection);
                        break;
                    }
                    case protocol::CREDENTIALS_FAILED: {
                        std::shared_ptr<protocol::Principal> p = connectionManager.principal;
                        std::shared_ptr<exception::AuthenticationException> exception;
                        if (p.get()) {
                            exception = (exception::ExceptionBuilder<exception::AuthenticationException>(
                                    "ConnectionManager::AuthCallback::onResponse") << "Invalid credentials! Principal: "
                                                                                   << *p).buildShared();
                        } else {
                            exception.reset(new exception::AuthenticationException(
                                    "ConnectionManager::AuthCallback::onResponse",
                                    "Invalid credentials! No principal."));
                        }
                        handleAuthenticationException(exception);
                        break;
                    }
                    default: {
                        handleAuthenticationException((exception::ExceptionBuilder<exception::AuthenticationException>(
                                "ConnectionManager::AuthCallback::onResponse")
                                << "Authentication status code not supported. status: "
                                << authenticationStatus).buildShared());
                    }
                }
            }

            void
            ClientConnectionManagerImpl::AuthCallback::onFailure(const std::shared_ptr<exception::IException> &e) {
                cancelTimeoutTask();

                handleAuthenticationException(e);
            }

            void ClientConnectionManagerImpl::AuthCallback::handleAuthenticationException(
                    const std::shared_ptr<exception::IException> &e) {
                this->onAuthenticationFailed(this->target, this->connection, e);
                this->future->onFailure(e);
            }

            void ClientConnectionManagerImpl::AuthCallback::onAuthenticationFailed(const Address &target,
                                                                                   const std::shared_ptr<Connection> &connection,
                                                                                   const std::shared_ptr<exception::IException> &cause) {
                if (connectionManager.logger.isFinestEnabled()) {
                    connectionManager.logger.finest("Authentication of ", connection, " failed.", cause);
                }
                connection->close("", cause);
                connectionManager.pendingSocketIdToConnection.remove(connection->getSocket().getSocketId());
                connectionManager.connectionsInProgress.remove(target);
            }

            ClientConnectionManagerImpl::DisconnecFromClusterTask::DisconnecFromClusterTask(
                    const std::shared_ptr<Connection> &connection, ClientConnectionManagerImpl &connectionManager,
                    ClientConnectionStrategy &connectionStrategy)
                    : connection(
                    connection), connectionManager(connectionManager), connectionStrategy(connectionStrategy) {
            }

            void ClientConnectionManagerImpl::DisconnecFromClusterTask::run() {
                std::shared_ptr<Address> endpoint = connection->getRemoteEndpoint();
                // it may be possible that while waiting on executor queue, the client got connected (another connection),
                // then we do not need to do anything for cluster disconnect.
                std::shared_ptr<Address> ownerAddress = connectionManager.ownerConnectionAddress;
                if (ownerAddress.get() && (endpoint.get() && *endpoint != *ownerAddress)) {
                    return;
                }

                connectionManager.setOwnerConnectionAddress(std::shared_ptr<Address>());
                connectionStrategy.onDisconnectFromCluster();

                if (connectionManager.client.getLifecycleService().isRunning()) {
                    connectionManager.fireConnectionEvent(LifecycleEvent::CLIENT_DISCONNECTED);
                }
            }

            const std::string ClientConnectionManagerImpl::DisconnecFromClusterTask::getName() const {
                return "DisconnecFromClusterTask";
            }

            ClientConnectionManagerImpl::ConnectToClusterTask::ConnectToClusterTask(
                    const spi::ClientContext &clientContext) : clientContext(clientContext) {
            }

            std::shared_ptr<bool> ClientConnectionManagerImpl::ConnectToClusterTask::call() {
                ClientConnectionManagerImpl &connectionManager = clientContext.getConnectionManager();
                try {
                    connectionManager.connectToClusterInternal();
                    return std::shared_ptr<bool>(new bool(true));
                } catch (exception::IException &e) {
                    connectionManager.getLogger().warning("Could not connect to cluster, shutting down the client. ",
                                                          e.getMessage());

                    static_cast<DefaultClientConnectionStrategy &>(*connectionManager.connectionStrategy).shutdownWithExternalThread(
                            clientContext.getHazelcastClientImplementation());
                    throw;
                } catch (...) {
                    throw;
                }
            }

            const std::string ClientConnectionManagerImpl::ConnectToClusterTask::getName() const {
                return "ClientConnectionManagerImpl::ConnectToClusterTask";
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace connection {
            AuthenticationFuture::AuthenticationFuture() : countDownLatch(new util::CountDownLatch(1)) {
            }

            void AuthenticationFuture::onSuccess(const std::shared_ptr<Connection> &connection) {
                this->connection = connection;
                countDownLatch->countDown();
            }

            void AuthenticationFuture::onFailure(const std::shared_ptr<exception::IException> &throwable) {
                this->throwable = throwable;
                countDownLatch->countDown();
            }

            std::shared_ptr<Connection> AuthenticationFuture::get() {
                countDownLatch->await();
                if (connection.get() != NULL) {
                    return connection;
                }
                assert(throwable.get() != NULL);
                throw exception::ExecutionException("AuthenticationFuture::get", "Could not be authenticated.",
                                                    throwable);
            }

        }
    }
}




//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {
            ReadHandler::ReadHandler(Connection &connection, InSelector &iListener, size_t bufferSize,
                                     spi::ClientContext &clientContext)
                    : IOHandler(connection, iListener), buffer(new char[bufferSize]), byteBuffer(buffer, bufferSize),
                      builder(connection) {
                lastReadTimeMillis = util::currentTimeMillis();
            }

            ReadHandler::~ReadHandler() {
                delete[] buffer;
            }

            void ReadHandler::run() {
                registerHandler();
            }

            void ReadHandler::handle() {
                lastReadTimeMillis = util::currentTimeMillis();
                try {
                    byteBuffer.readFrom(connection.getSocket());
                } catch (exception::IOException &e) {
                    handleSocketException(e.what());
                    return;
                }

                if (byteBuffer.position() == 0)
                    return;
                byteBuffer.flip();

                // it is important to check the onData return value since there may be left data less than a message
                // header size, and this may cause an infinite loop.
                while (byteBuffer.hasRemaining() && builder.onData(byteBuffer)) {
                }

                if (byteBuffer.hasRemaining()) {
                    byteBuffer.compact();
                } else {
                    byteBuffer.clear();
                }
            }

            int64_t ReadHandler::getLastReadTimeMillis() {
                return lastReadTimeMillis;
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace connection {
            InSelector::InSelector(ClientConnectionManagerImpl &connectionManager,
                                   const config::SocketOptions &socketOptions)
                    : IOSelector(connectionManager, socketOptions) {
            }

            bool InSelector::start() {
                return initListenSocket(socketSet);
            }

            void InSelector::listenInternal() {
                fd_set read_fds;
                util::SocketSet::FdRange socketRange = socketSet.fillFdSet(read_fds);
#if  defined(__GNUC__) || defined(__llvm__)
                errno = 0;
#endif
                t.tv_sec = 5;
                t.tv_usec = 0;
                int numSelected = select(socketRange.max + 1, &read_fds, NULL, NULL, &t);
                if (numSelected == 0) {
                    return;
                }

                if (checkError("Exception InSelector::listen => ", numSelected)) {
                    return;
                }

                for (int fd = socketRange.min; numSelected > 0 && fd <= socketRange.max; ++fd) {
                    if (FD_ISSET(fd, &read_fds)) {
                        --numSelected;
                        if (wakeUpListenerSocketId == fd) {
                            int wakeUpSignal;
                            sleepingSocket->receive(&wakeUpSignal, sizeof(int));
                        } else {
                            std::shared_ptr<Connection> conn = connectionManager.getActiveConnection(fd);
                            if (conn.get() != NULL) {
                                conn->getReadHandler().handle();
                            }
                        }
                    }
                }
            }

            const std::string InSelector::getName() const {
                return "InSelector";
            }
        }
    }
}






namespace hazelcast {
    namespace client {
        namespace connection {

            IOSelector::IOSelector(ClientConnectionManagerImpl &connectionManager,
                                   const config::SocketOptions &socketOptions)
                    : socketSet(connectionManager.getLogger()), connectionManager(connectionManager),
                      logger(connectionManager.getLogger()), socketOptions(socketOptions) {
                t.tv_sec = 5;
                t.tv_usec = 0;
            }

            IOSelector::~IOSelector() {
                shutdown();
            }

            void IOSelector::wakeUp() {
                if (!wakeUpSocket.get()) {
                    return;
                }

                int wakeUpSignal = 9;
                try {
                    wakeUpSocket->send(&wakeUpSignal, sizeof(int), MSG_WAITALL);
                } catch (exception::IOException &e) {
                    logger.warning(std::string("Exception at IOSelector::wakeUp ") + e.what());
                    throw;
                }
            }

            void IOSelector::run() {
                while (isAlive) {
                    try {
                        processListenerQueue();
                        listenInternal();
                    } catch (exception::IException &e) {
                        if (isAlive) {
                            logger.warning(std::string("Exception at IOSelector::run() ") + e.what());
                        } else {
                            if (logger.isFinestEnabled()) {
                                logger.finest(std::string("Exception at IOSelector::run() ") + e.what());
                            }
                        }
                    }
                }
            }

            bool IOSelector::initListenSocket(util::SocketSet &wakeUpSocketSet) {
                serverSocket = std::make_unique<util::ServerSocket>(0);
                int p = serverSocket->getPort();
                std::string localAddress;
                if (serverSocket->isIpv4())
                    localAddress = "127.0.0.1";
                else
                    localAddress = "::1";

                wakeUpSocket.reset(new internal::socket::TcpSocket(Address(localAddress, p), &socketOptions));
                int error = wakeUpSocket->connect(5000);
                if (error == 0) {
                    sleepingSocket.reset(serverSocket->accept());
                    sleepingSocket->setBlocking(false);
                    wakeUpSocketSet.insertSocket(sleepingSocket.get());
                    wakeUpListenerSocketId = sleepingSocket->getSocketId();
                    isAlive.store(true);
                    return true;
                } else {
                    logger.severe("IOSelector::initListenSocket " + std::string(strerror(errno)));
                    return false;
                }
            }

            void IOSelector::shutdown() {
                bool expected = true;
                if (!isAlive.compare_exchange_strong(expected, false)) {
                    return;
                }
                try {
                    wakeUp();
                } catch (exception::IOException &) {
                    // suppress io exception
                }

                sleepingSocket->close();
                wakeUpSocket->close();
                serverSocket->close();
            }

            void IOSelector::addTask(ListenerTask *listenerTask) {
                listenerTasks.offer(listenerTask);
            }

            void IOSelector::cancelTask(ListenerTask *listenerTask) {
                listenerTasks.removeAll(listenerTask);
            }

            void IOSelector::addSocket(const Socket &socket) {
                socketSet.insertSocket(&socket);
            }

            void IOSelector::removeSocket(const Socket &socket) {
                socketSet.removeSocket(&socket);
            }

            void IOSelector::processListenerQueue() {
                while (ListenerTask * task = listenerTasks.poll()) {
                    task->run();
                }
            }

            bool IOSelector::checkError(const char *messagePrefix, int numSelected) const {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                if (numSelected == SOCKET_ERROR) {
                    int error = WSAGetLastError();
                    if (WSAENOTSOCK == error) {
                        if (logger.isEnabled(FINEST)) {
                            char errorMsg[200];
                            util::strerror_s(error, errorMsg, 200, messagePrefix);
                            logger.finest(errorMsg);
                        }
                    } else {
                        char errorMsg[200];
                        util::strerror_s(error, errorMsg, 200, messagePrefix);
                        logger.severe(errorMsg);
                    }
                    return true;
                }
#else
                if (numSelected == -1) {
                    int error = errno;
                    if (EINTR == error ||
                        EBADF == error /* This case may happen if socket closed by cluster listener thread */) {
                        if (logger.isEnabled(FINEST)) {
                            char errorMsg[200];
                            util::strerror_s(error, errorMsg, 200, messagePrefix);
                            logger.finest(errorMsg);
                        }
                    } else {
                        char errorMsg[200];
                        util::strerror_s(error, errorMsg, 200, messagePrefix);
                        logger.severe(errorMsg);
                    }
                    return true;
                }
#endif

                return false;
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace connection {
            ListenerTask::~ListenerTask() {
            }
        }
    }
}


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996)
#pragma warning(disable: 4355)
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address &address, spi::ClientContext &clientContext, int connectionId,
                                   InSelector &iListener, OutSelector &oListener,
                                   internal::socket::SocketFactory &socketFactory)
                    : startTimeInMillis(util::currentTimeMillis()), closedTimeMillis(0),
                      clientContext(clientContext),
                      invocationService(clientContext.getInvocationService()),
                      readHandler(*this, iListener, 16 << 10, clientContext),
                      writeHandler(*this, oListener, 16 << 10),
                      connectionId(connectionId),
                      connectedServerVersion(impl::BuildInfo::UNKNOWN_HAZELCAST_VERSION),
                      logger(clientContext.getLogger()) {
                socket = socketFactory.create(address);
            }

            Connection::~Connection() {
            }

            void Connection::connect(int timeoutInMillis) {
                if (!isAlive()) {
                    std::ostringstream out;
                    out << "Connection " << (*this) << " is already closed!";
                    throw exception::IOException("Connection::connect", out.str());
                }

                int error = socket->connect(timeoutInMillis);
                if (error) {
                    char errorMsg[200];
                    util::strerror_s(error, errorMsg, 200);
                    throw exception::IOException("Connection::connect", errorMsg);
                }

                socket->send("CB2", 3, MSG_WAITALL);
            }

            void Connection::close(const char *reason) {
                close(reason, std::shared_ptr<exception::IException>());
            }

            void Connection::close(const char *reason, const std::shared_ptr<exception::IException> &cause) {
                int64_t expected = 0;
                if (!closedTimeMillis.compare_exchange_strong(expected, util::currentTimeMillis())) {
                    return;
                }

                closeCause = cause;
                if (reason) {
                    closeReason = reason;
                }

                logClose();

                writeHandler.deRegisterSocket();
                readHandler.deRegisterSocket();

                try {
                    innerClose();
                } catch (exception::IException &e) {
                    clientContext.getLogger().warning("Exception while closing connection", e.getMessage());
                }

                clientContext.getConnectionManager().onClose(*this);
            }

            bool Connection::write(const std::shared_ptr<protocol::ClientMessage> &message) {
                if (writeHandler.enqueueData(message)) {
                    return true;
                }

                if (logger.isFinestEnabled()) {
                    logger.finest("Connection is closed, dropping frame -> ", message);
                }
                return false;
            }

            Socket &Connection::getSocket() {
                return *socket;
            }

            const std::shared_ptr<Address> &Connection::getRemoteEndpoint() const {
                return remoteEndpoint;
            }

            void Connection::setRemoteEndpoint(const std::shared_ptr<Address> &remoteEndpoint) {
                this->remoteEndpoint = remoteEndpoint;
            }

            ReadHandler &Connection::getReadHandler() {
                return readHandler;
            }

            WriteHandler &Connection::getWriteHandler() {
                return writeHandler;
            }

            void Connection::handleClientMessage(const std::shared_ptr<protocol::ClientMessage> &message) {
                if (message->isFlagSet(protocol::ClientMessage::LISTENER_EVENT_FLAG)) {
                    spi::impl::listener::AbstractClientListenerService &listenerService =
                            (spi::impl::listener::AbstractClientListenerService &) clientContext.getClientListenerService();
                    listenerService.handleClientMessage(message, shared_from_this());
                } else {
                    invocationService.handleClientMessage(shared_from_this(), message);
                }
            }

            int Connection::getConnectionId() const {
                return connectionId;
            }

            bool Connection::isAlive() {
                return closedTimeMillis == 0;
            }

            const std::string &Connection::getCloseReason() const {
                return closeReason;
            }

            void Connection::logClose() {
                std::ostringstream message;
                message << *this << " closed. Reason: ";
                if (!closeReason.empty()) {
                    message << closeReason;
                } else if (closeCause.get() != NULL) {
                    message << closeCause->getSource() << "[" + closeCause->getMessage() << "]";
                } else {
                    message << "Socket explicitly closed";
                }

                util::ILogger &logger = clientContext.getLogger();
                if (clientContext.getLifecycleService().isRunning()) {
                    if (!closeCause.get()) {
                        logger.info(message.str());
                    } else {
                        logger.warning(message.str(), *closeCause);
                    }
                } else {
                    if (closeCause.get() == NULL) {
                        logger.finest(message.str());
                    } else {
                        logger.finest(message.str(), *closeCause);
                    }
                }
            }

            bool Connection::isAuthenticatedAsOwner() {
                return authenticatedAsOwner;
            }

            void Connection::setIsAuthenticatedAsOwner() {
                authenticatedAsOwner.store(true);
            }

            bool Connection::operator==(const Connection &rhs) const {
                return connectionId == rhs.connectionId;
            }

            bool Connection::operator!=(const Connection &rhs) const {
                return !(rhs == *this);
            }

            const std::string &Connection::getConnectedServerVersionString() const {
                return connectedServerVersionString;
            }

            void Connection::setConnectedServerVersion(const std::string &connectedServerVersionString) {
                Connection::connectedServerVersionString = connectedServerVersionString;
                connectedServerVersion = impl::BuildInfo::calculateVersion(connectedServerVersionString);
            }

            int Connection::getConnectedServerVersion() const {
                return connectedServerVersion;
            }

            std::unique_ptr<Address> Connection::getLocalSocketAddress() const {
                return socket->localSocketAddress();
            }

            int64_t Connection::lastReadTimeMillis() {
                return readHandler.getLastReadTimeMillis();
            }

            void Connection::innerClose() {
                if (!socket.get()) {
                    return;;
                }

                socket->close();
            }

            std::ostream &operator<<(std::ostream &os, const Connection &connection) {
                Connection &conn = const_cast<Connection &>(connection);
                int64_t lastRead = conn.lastReadTimeMillis();
                int64_t closedTime = conn.closedTimeMillis;
                os << "ClientConnection{"
                   << "alive=" << conn.isAlive()
                   << ", connectionId=" << connection.getConnectionId()
                   << ", remoteEndpoint=";
                if (connection.getRemoteEndpoint().get()) {
                    os << *connection.getRemoteEndpoint();
                } else {
                    os << "null";
                }
                os << ", lastReadTime=" << util::StringUtil::timeToStringFriendly(lastRead)
                   << ", closedTime=" << util::StringUtil::timeToStringFriendly(closedTime)
                   << ", connected server version=" << conn.connectedServerVersionString
                   << '}';

                return os;
            }

            bool Connection::operator<(const Connection &rhs) const {
                return connectionId < rhs.connectionId;
            }

            int64_t Connection::getStartTimeInMillis() const {
                return startTimeInMillis;
            }
        }
    }
}





//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {
            WriteHandler::WriteHandler(Connection &connection, OutSelector &oListener, size_t bufferSize)
                    : IOHandler(connection, oListener), ready(false), informSelector(true) {
            }


            WriteHandler::~WriteHandler() {
                // no need to delete the messages since they are owned by their associated future objects
            }

            void WriteHandler::run() {
                if (this->connection.isAlive()) {
                    informSelector.store(true);
                    if (ready) {
                        handle();
                    } else {
                        registerHandler();
                    }
                    ready = false;
                }
            }

            // TODO: Add a fragmentation layer here before putting the message into the write queue
            bool WriteHandler::enqueueData(const std::shared_ptr<protocol::ClientMessage> &message) {
                if (!this->connection.isAlive()) {
                    return false;
                }
                writeQueue.offer(message);
                bool expected = true;
                if (informSelector.compare_exchange_strong(expected, false)) {
                    ioSelector.addTask(this);
                    ioSelector.wakeUp();
                }
                return true;
            }

            void WriteHandler::handle() {
                if (lastMessage.get() == NULL) {
                    if (!(lastMessage = writeQueue.poll()).get()) {
                        ready = true;
                        return;
                    }

                    if (NULL != lastMessage.get()) {
                        numBytesWrittenToSocketForMessage = 0;
                        lastMessageFrameLen = lastMessage->getFrameLength();
                    }
                }

                while (NULL != lastMessage.get()) {
                    try {
                        numBytesWrittenToSocketForMessage += lastMessage->writeTo(connection.getSocket(),
                                                                                  numBytesWrittenToSocketForMessage,
                                                                                  lastMessageFrameLen);

                        if (numBytesWrittenToSocketForMessage >= lastMessageFrameLen) {
                            // Not deleting message since its memory management is at the future object
                            if ((lastMessage = writeQueue.poll()).get()) {
                                numBytesWrittenToSocketForMessage = 0;
                                lastMessageFrameLen = lastMessage->getFrameLength();
                            }
                        } else {
                            // Message could not be sent completely, just continue with another connection
                            break;
                        }
                    } catch (exception::IOException &e) {
                        handleSocketException(e.what());
                        return;
                    }
                }

                ready = false;
                registerHandler();
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace connection {

            ClientConnectionStrategy::ClientConnectionStrategy(spi::ClientContext &clientContext, util::ILogger &logger,
                                                               const config::ClientConnectionStrategyConfig &clientConnectionStrategyConfig)
                    : clientContext(clientContext), logger(logger),
                      clientConnectionStrategyConfig(clientConnectionStrategyConfig) {
            }

            ClientConnectionStrategy::~ClientConnectionStrategy() {
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace connection {

            IOHandler::IOHandler(Connection &connection, IOSelector &ioSelector)
                    : ioSelector(ioSelector), connection(connection) {
            }

            void IOHandler::registerSocket() {
                ioSelector.addTask(this);
                ioSelector.wakeUp();
            }

            void IOHandler::registerHandler() {
                if (!connection.isAlive())
                    return;
                Socket const &socket = connection.getSocket();
                ioSelector.addSocket(socket);
            }

            void IOHandler::deRegisterSocket() {
                ioSelector.cancelTask(this);
                ioSelector.removeSocket(connection.getSocket());
            }

            IOHandler::~IOHandler() {
            }

            void IOHandler::handleSocketException(const std::string &message) {
                // TODO: This call shall resend pending requests and reregister events, hence it can be off-loaded
                // to another thread in order not to block the critical IO thread
                connection.close(message.c_str());
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace connection {

            OutSelector::OutSelector(ClientConnectionManagerImpl &connectionManager,
                                     const config::SocketOptions &socketOptions)
                    : IOSelector(connectionManager, socketOptions), wakeUpSocketSet(connectionManager.getLogger()) {
            }

            bool OutSelector::start() {
                return initListenSocket(wakeUpSocketSet);
            }

            void OutSelector::listenInternal() {
                fd_set write_fds;
                util::SocketSet::FdRange socketRange = socketSet.fillFdSet(write_fds);

                fd_set wakeUp_fds;
                util::SocketSet::FdRange wakeupSocketRange = wakeUpSocketSet.fillFdSet(wakeUp_fds);

                int maxFd = (socketRange.max > wakeupSocketRange.max ? socketRange.max : wakeupSocketRange.max);

#if  defined(__GNUC__) || defined(__llvm__)
                errno = 0;
#endif
                t.tv_sec = 5;
                t.tv_usec = 0;

                int numSelected = select(maxFd + 1, &wakeUp_fds, &write_fds, NULL, &t);
                if (numSelected == 0) {
                    return;
                }

                if (checkError("Exception OutSelector::listen => ", numSelected)) {
                    return;
                }

                if (FD_ISSET(wakeUpListenerSocketId, &wakeUp_fds)) {
                    int wakeUpSignal;
                    sleepingSocket->receive(&wakeUpSignal, sizeof(int));
                    --numSelected;
                }

                for (int fd = socketRange.min; numSelected > 0 && fd <= socketRange.max; ++fd) {
                    if (FD_ISSET(fd, &write_fds)) {
                        --numSelected;
                        std::shared_ptr<Connection> conn = connectionManager.getActiveConnection(fd);

                        if (conn.get() != NULL) {
                            socketSet.removeSocket(&conn->getSocket());
                            conn->getWriteHandler().handle();
                        }
                    }
                }
            }

            const std::string OutSelector::getName() const {
                return "OutSelector";
            }

        }
    }
}




namespace hazelcast {
    namespace client {
        namespace connection {

            HeartbeatManager::HeartbeatManager(spi::ClientContext &client) : client(client), clientConnectionManager(
                    client.getConnectionManager()), logger(client.getLogger()) {
                ClientProperties &clientProperties = client.getClientProperties();
                int timeoutSeconds = clientProperties.getInteger(clientProperties.getHeartbeatTimeout());
                heartbeatTimeout = timeoutSeconds > 0 ? timeoutSeconds * 1000 : util::IOUtil::to_value<int>(
                        (std::string) ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT) * 1000;

                int intervalSeconds = clientProperties.getInteger(clientProperties.getHeartbeatInterval());
                heartbeatInterval = intervalSeconds > 0 ? intervalSeconds * 1000 : util::IOUtil::to_value<int>(
                        (std::string) ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT) * 1000;
            }

            void HeartbeatManager::start() {
                spi::impl::ClientExecutionServiceImpl &clientExecutionService = client.getClientExecutionService();

                clientExecutionService.scheduleWithRepetition(
                        std::shared_ptr<util::Runnable>(new util::RunnableDelegator(*this)), heartbeatInterval,
                        heartbeatInterval);
            }

            void HeartbeatManager::run() {
                if (!clientConnectionManager.isAlive()) {
                    return;
                }

                int64_t now = util::currentTimeMillis();
                for (std::shared_ptr<Connection> connection : clientConnectionManager.getActiveConnections()) {
                    checkConnection(now, connection);
                }
            }

            const std::string HeartbeatManager::getName() const {
                return "HeartbeatManager";
            }

            void HeartbeatManager::checkConnection(int64_t now, std::shared_ptr<Connection> &connection) {
                if (!connection->isAlive()) {
                    return;
                }

                if (now - connection->lastReadTimeMillis() > heartbeatTimeout) {
                    if (connection->isAlive()) {
                        logger.warning("Heartbeat failed over the connection: ", *connection);
                        onHeartbeatStopped(connection, "Heartbeat timed out");
                    }
                }

                if (now - connection->lastReadTimeMillis() > heartbeatInterval) {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ClientPingCodec::encodeRequest();
                    std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client, request, "", connection);
                    clientInvocation->invokeUrgent();
                }
            }

            void
            HeartbeatManager::onHeartbeatStopped(std::shared_ptr<Connection> &connection, const std::string &reason) {
                connection->close(reason.c_str(), (exception::ExceptionBuilder<exception::TargetDisconnectedException>(
                        "HeartbeatManager::onHeartbeatStopped") << "Heartbeat timed out to connection "
                                                                << *connection).buildShared());
            }

            void HeartbeatManager::shutdown() {
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        Member::Member() : liteMember(false) {
        }

        Member::Member(const Address &address, const std::string &uuid, bool lite,
                       const std::map<std::string, std::string> &attr) :
                address(address), uuid(uuid), liteMember(lite), attributes(attr) {
        }

        Member::Member(const Address &memberAddress) : address(memberAddress), liteMember(false) {
        }

        Member::Member(const std::string &uuid) : uuid(uuid), liteMember(false) {
        }

        bool Member::operator==(const Member &rhs) const {
            return address == rhs.address;
        }

        const Address &Member::getAddress() const {
            return address;
        }

        const std::string &Member::getUuid() const {
            return uuid;
        }

        bool Member::isLiteMember() const {
            return liteMember;
        }

        const std::map<std::string, std::string> &Member::getAttributes() const {
            return attributes;
        }

        std::ostream &operator<<(std::ostream &out, const Member &member) {
            const Address &address = member.getAddress();
            out << "Member[";
            out << address.getHost();
            out << "]";
            out << ":";
            out << address.getPort();
            out << " - " << member.getUuid();
            return out;
        }

        const std::string *Member::getAttribute(const std::string &key) const {
            std::map<std::string, std::string>::const_iterator it = attributes.find(key);
            if (attributes.end() != it) {
                return &(it->second);
            } else {
                return NULL;
            }
        }

        bool Member::lookupAttribute(const std::string &key) const {
            return attributes.find(key) != attributes.end();
        }

        bool Member::operator<(const Member &rhs) const {
            return uuid < rhs.uuid;
        }

        void Member::updateAttribute(Member::MemberAttributeOperationType operationType, const std::string &key,
                                     std::unique_ptr<std::string> &value) {
            switch (operationType) {
                case PUT:
                    attributes[key] = *value;
                    break;
                case REMOVE:
                    attributes.erase(key);
                    break;
                default:
                    throw (exception::ExceptionBuilder<exception::IllegalArgumentException>("Member::updateAttribute")
                            << "Not a known OperationType: " << operationType).build();
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace connection {

            DefaultClientConnectionStrategy::DefaultClientConnectionStrategy(spi::ClientContext &clientContext,
                                                                             util::ILogger &logger,
                                                                             const config::ClientConnectionStrategyConfig &clientConnectionStrategyConfig)
                    : ClientConnectionStrategy(clientContext, logger, clientConnectionStrategyConfig),
                      isShutdown(false) {
            }

            void DefaultClientConnectionStrategy::start() {
                clientStartAsync = clientConnectionStrategyConfig.isAsyncStart();
                reconnectMode = clientConnectionStrategyConfig.getReconnectMode();
                if (clientStartAsync) {
                    clientContext.getConnectionManager().connectToClusterAsync();
                } else {
                    clientContext.getConnectionManager().connectToCluster();
                }
            }

            void DefaultClientConnectionStrategy::beforeGetConnection(const Address &target) {
                checkShutdown("DefaultClientConnectionStrategy::beforeGetConnection");

                if (isClusterAvailable()) {
                    return;
                }
                if (clientStartAsync && !disconnectedFromCluster) {
                    throw exception::HazelcastClientOfflineException(
                            "DefaultClientConnectionStrategy::beforeGetConnection", "Client is connecting to cluster.");
                }
                if (reconnectMode == config::ClientConnectionStrategyConfig::ASYNC && disconnectedFromCluster) {
                    throw exception::HazelcastClientOfflineException(
                            "DefaultClientConnectionStrategy::beforeGetConnection", "Client is offline.");
                }
            }

            void DefaultClientConnectionStrategy::beforeOpenConnection(const Address &target) {
                checkShutdown("DefaultClientConnectionStrategy::beforeOpenConnection");

                if (isClusterAvailable()) {
                    return;
                }
                if (reconnectMode == config::ClientConnectionStrategyConfig::ASYNC && disconnectedFromCluster) {
                    throw exception::HazelcastClientOfflineException(
                            "DefaultClientConnectionStrategy::beforeGetConnection", "Client is offline");
                }
            }

            void DefaultClientConnectionStrategy::onConnectToCluster() {
                checkShutdown("DefaultClientConnectionStrategy::onConnectToCluster");

                disconnectedFromCluster.store(false);
            }

            void DefaultClientConnectionStrategy::onDisconnectFromCluster() {
                checkShutdown("DefaultClientConnectionStrategy::onDisconnectFromCluster");

                disconnectedFromCluster.store(true);
                if (reconnectMode == config::ClientConnectionStrategyConfig::OFF) {
                    shutdownWithExternalThread(clientContext.getHazelcastClientImplementation());
                    return;
                }
                if (clientContext.getLifecycleService().isRunning()) {
                    try {
                        clientContext.getConnectionManager().connectToClusterAsync();
                    } catch (exception::RejectedExecutionException &) {
                        shutdownWithExternalThread(clientContext.getHazelcastClientImplementation());
                    }
                }
            }

            void DefaultClientConnectionStrategy::onConnect(const std::shared_ptr<Connection> &connection) {
                checkShutdown("DefaultClientConnectionStrategy::onConnect");
            }

            void DefaultClientConnectionStrategy::onDisconnect(const std::shared_ptr<Connection> &connection) {
                checkShutdown("DefaultClientConnectionStrategy::onDisconnect");
            }

            void DefaultClientConnectionStrategy::shutdown() {
                isShutdown = true;
            }

            bool DefaultClientConnectionStrategy::isClusterAvailable() const {
                return clientContext.getConnectionManager().getOwnerConnectionAddress().get() != NULL;
            }

            void
            DefaultClientConnectionStrategy::shutdownWithExternalThread(
                    std::weak_ptr<client::impl::HazelcastClientInstanceImpl> clientImpl) {

                std::thread shutdownThread([=] {
                    std::shared_ptr<client::impl::HazelcastClientInstanceImpl> clientInstance = clientImpl.lock();
                    if (!clientInstance.get() || !clientInstance->getLifecycleService().isRunning()) {
                        return;
                    }

                    try {
                        clientInstance->getLifecycleService().shutdown();
                    } catch (exception::IException &exception) {
                        clientInstance->getLogger()->severe("Exception during client shutdown task ",
                                                            clientInstance->getName() + ".clientShutdown-", ":",
                                                            exception);
                    }
                });

                shutdownThread.detach();
            }

            void DefaultClientConnectionStrategy::checkShutdown(const std::string &methodName) {
                if (isShutdown) {
                    throw exception::IllegalStateException(methodName, "Client is shutdown.");
                }
            }
        }
    }
}


// Includes for parameters classes


namespace hazelcast {
    namespace client {

        ICountDownLatch::ICountDownLatch(const std::string &objectName, spi::ClientContext *context)
                : proxy::ProxyImpl("hz:impl:atomicLongService", objectName, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&objectName);
            partitionId = getPartitionId(keyData);
        }

        bool ICountDownLatch::await(long timeoutInMillis) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::CountDownLatchAwaitCodec::encodeRequest(getName(), timeoutInMillis);

            return invokeAndGetResult<bool, protocol::codec::CountDownLatchAwaitCodec::ResponseParameters>(request,
                                                                                                           partitionId);
        }

        void ICountDownLatch::countDown() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::CountDownLatchCountDownCodec::encodeRequest(getName());

            invokeOnPartition(request, partitionId);
        }

        int ICountDownLatch::getCount() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::CountDownLatchGetCountCodec::encodeRequest(getName());

            return invokeAndGetResult<int, protocol::codec::CountDownLatchGetCountCodec::ResponseParameters>(request,
                                                                                                             partitionId);
        }

        bool ICountDownLatch::trySetCount(int count) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::CountDownLatchTrySetCountCodec::encodeRequest(getName(), count);

            return invokeAndGetResult<bool, protocol::codec::CountDownLatchTrySetCountCodec::ResponseParameters>(
                    request, partitionId);
        }
    }
}


namespace hazelcast {
    namespace client {

        MapEvent::MapEvent(const Member &member, EntryEventType eventType, const std::string &name,
                           int numberOfEntriesAffected)
                : member(member), eventType(eventType), name(name), numberOfEntriesAffected(numberOfEntriesAffected) {

        }

        Member MapEvent::getMember() const {
            return member;
        }

        EntryEventType MapEvent::getEventType() const {
            return eventType;
        }

        const std::string &MapEvent::getName() const {
            return name;
        }

        int MapEvent::getNumberOfEntriesAffected() const {
            return numberOfEntriesAffected;
        }

        std::ostream &operator<<(std::ostream &os, const MapEvent &event) {
            os << "MapEvent{member: " << event.member << " eventType: " << event.eventType << " name: " << event.name
               << " numberOfEntriesAffected: " << event.numberOfEntriesAffected;
            return os;
        }
    }
}


namespace hazelcast {
    namespace client {
        Endpoint::Endpoint(std::shared_ptr<std::string> uuid, std::shared_ptr<Address> socketAddress) : uuid(uuid),
                                                                                                        socketAddress(
                                                                                                                socketAddress) {}

        const std::shared_ptr<std::string> &Endpoint::getUuid() const {
            return uuid;
        }

        const std::shared_ptr<Address> &Endpoint::getSocketAddress() const {
            return socketAddress;
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace config {

            ClientFlakeIdGeneratorConfig::ClientFlakeIdGeneratorConfig(const std::string &name)
                    : name(name), prefetchCount(DEFAULT_PREFETCH_COUNT),
                      prefetchValidityMillis(DEFAULT_PREFETCH_VALIDITY_MILLIS) {}

            const std::string &ClientFlakeIdGeneratorConfig::getName() const {
                return name;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::setName(const std::string &name) {
                ClientFlakeIdGeneratorConfig::name = name;
                return *this;
            }

            int32_t ClientFlakeIdGeneratorConfig::getPrefetchCount() const {
                return prefetchCount;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::setPrefetchCount(int32_t prefetchCount) {
                std::ostringstream out;
                out << "prefetch-count must be 1.." << MAXIMUM_PREFETCH_COUNT << ", not " << prefetchCount;
                util::Preconditions::checkTrue(prefetchCount > 0 && prefetchCount <= MAXIMUM_PREFETCH_COUNT, out.str());
                ClientFlakeIdGeneratorConfig::prefetchCount = prefetchCount;
                return *this;
            }

            int64_t ClientFlakeIdGeneratorConfig::getPrefetchValidityMillis() const {
                return prefetchValidityMillis;
            }

            ClientFlakeIdGeneratorConfig &
            ClientFlakeIdGeneratorConfig::setPrefetchValidityMillis(int64_t prefetchValidityMillis) {
                util::Preconditions::checkNotNegative(prefetchValidityMillis,
                                                      "prefetchValidityMs must be non negative");
                ClientFlakeIdGeneratorConfig::prefetchValidityMillis = prefetchValidityMillis;
                return *this;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace config {
            SSLConfig::SSLConfig() : enabled(false), sslProtocol(tlsv12) {
            }

            bool SSLConfig::isEnabled() const {
                return enabled;
            }

            SSLConfig &SSLConfig::setEnabled(bool enabled) {
                util::Preconditions::checkSSL("getAwsConfig");
                this->enabled = enabled;
                return *this;
            }

            SSLConfig &SSLConfig::setProtocol(SSLProtocol protocol) {
                this->sslProtocol = protocol;
                return *this;
            }

            SSLProtocol SSLConfig::getProtocol() const {
                return sslProtocol;
            }

            const std::vector<std::string> &SSLConfig::getVerifyFiles() const {
                return clientVerifyFiles;
            }

            SSLConfig &SSLConfig::addVerifyFile(const std::string &filename) {
                this->clientVerifyFiles.push_back(filename);
                return *this;
            }

            const std::string &SSLConfig::getCipherList() const {
                return cipherList;
            }

            SSLConfig &SSLConfig::setCipherList(const std::string &ciphers) {
                this->cipherList = ciphers;
                return *this;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace config {
            int32_t ClientNetworkConfig::CONNECTION_ATTEMPT_PERIOD = 3000;

            ClientNetworkConfig::ClientNetworkConfig()
                    : connectionTimeout(5000), smartRouting(true), connectionAttemptLimit(-1),
                      connectionAttemptPeriod(CONNECTION_ATTEMPT_PERIOD) {
            }

            SSLConfig &ClientNetworkConfig::getSSLConfig() {
                return sslConfig;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSSLConfig(const config::SSLConfig &sslConfig) {
                this->sslConfig = sslConfig;
                return *this;
            }

            int64_t ClientNetworkConfig::getConnectionTimeout() const {
                return connectionTimeout;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionTimeout(int64_t connectionTimeoutInMillis) {
                this->connectionTimeout = connectionTimeoutInMillis;
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::setAwsConfig(const ClientAwsConfig &clientAwsConfig) {
                this->clientAwsConfig = clientAwsConfig;
                return *this;
            }

            ClientAwsConfig &ClientNetworkConfig::getAwsConfig() {
                return clientAwsConfig;
            }

            bool ClientNetworkConfig::isSmartRouting() const {
                return smartRouting;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSmartRouting(bool smartRouting) {
                ClientNetworkConfig::smartRouting = smartRouting;
                return *this;
            }

            int32_t ClientNetworkConfig::getConnectionAttemptLimit() const {
                return connectionAttemptLimit;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionAttemptLimit(int32_t connectionAttemptLimit) {
                if (connectionAttemptLimit < 0) {
                    throw exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptLimit",
                                                              "connectionAttemptLimit cannot be negative");
                }
                this->connectionAttemptLimit = connectionAttemptLimit;
                return *this;
            }

            int32_t ClientNetworkConfig::getConnectionAttemptPeriod() const {
                return connectionAttemptPeriod;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionAttemptPeriod(int32_t connectionAttemptPeriod) {
                if (connectionAttemptPeriod < 0) {
                    throw exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptPeriod",
                                                              "connectionAttemptPeriod cannot be negative");
                }
                this->connectionAttemptPeriod = connectionAttemptPeriod;
                return *this;
            }

            std::vector<Address> ClientNetworkConfig::getAddresses() const {
                return addressList;
            }

            ClientNetworkConfig &ClientNetworkConfig::addAddresses(const std::vector<Address> &addresses) {
                addressList.insert(addressList.end(), addresses.begin(), addresses.end());
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::setAddresses(const std::vector<Address> &addresses) {
                addressList = addresses;
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::addAddress(const Address &address) {
                addressList.push_back(address);
                return *this;
            }

            SocketOptions &ClientNetworkConfig::getSocketOptions() {
                return socketOptions;
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace config {

            const std::string &LoggerConfig::getConfigurationFileName() const {
                return configurationFileName;
            }

            void LoggerConfig::setConfigurationFileName(const std::string &fileName) {
                LoggerConfig::configurationFileName = fileName;
            }

            LoggerConfig::LoggerConfig() : type(Type::EASYLOGGINGPP), logLevel(LoggerLevel::INFO) {}

            LoggerConfig::Type::LoggerType LoggerConfig::getType() const {
                return type;
            }

            void LoggerConfig::setType(LoggerConfig::Type::LoggerType type) {
                LoggerConfig::type = type;
            }

            LoggerLevel::Level LoggerConfig::getLogLevel() const {
                return logLevel;
            }

            void LoggerConfig::setLogLevel(LoggerLevel::Level logLevel) {
                LoggerConfig::logLevel = logLevel;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace config {
            namespace matcher {
                std::shared_ptr<std::string>
                MatchingPointConfigPatternMatcher::matches(const std::vector<std::string> &configPatterns,
                                                           const std::string &itemName) const {
                    std::shared_ptr<std::string> candidate;
                    std::shared_ptr<std::string> duplicate;
                    int lastMatchingPoint = -1;
                    for (const std::string &pattern  : configPatterns) {
                        int matchingPoint = getMatchingPoint(pattern, itemName);
                        if (matchingPoint > -1 && matchingPoint >= lastMatchingPoint) {
                            if (matchingPoint == lastMatchingPoint) {
                                duplicate = candidate;
                            } else {
                                duplicate.reset();
                            }
                            lastMatchingPoint = matchingPoint;
                            candidate.reset(new std::string(pattern));
                        }
                    }
                    if (duplicate.get() != NULL) {
                        throw (exception::ExceptionBuilder<exception::ConfigurationException>(
                                "MatchingPointConfigPatternMatcher::matches") << "Configuration " << itemName
                                                                              << " has duplicate configuration. Candidate:"
                                                                              << *candidate << ", duplicate:"
                                                                              << *duplicate).build();
                    }
                    return candidate;
                }

                int MatchingPointConfigPatternMatcher::getMatchingPoint(const std::string &pattern,
                                                                        const std::string &itemName) const {
                    size_t index = pattern.find('*');
                    if (index == std::string::npos) {
                        return -1;
                    }

                    std::string firstPart = pattern.substr(0, index);
                    if (itemName.find(firstPart) != 0) {
                        return -1;
                    }

                    std::string secondPart = pattern.substr(index + 1);
                    if (itemName.rfind(secondPart) != (itemName.length() - secondPart.length())) {
                        return -1;
                    }

                    return (int) (firstPart.length() + secondPart.length());
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace config {
            ClientConnectionStrategyConfig::ClientConnectionStrategyConfig() : asyncStart(false), reconnectMode(ON) {
            }

            ClientConnectionStrategyConfig::ReconnectMode ClientConnectionStrategyConfig::getReconnectMode() const {
                return reconnectMode;
            }

            bool ClientConnectionStrategyConfig::isAsyncStart() const {
                return asyncStart;
            }

            ClientConnectionStrategyConfig &ClientConnectionStrategyConfig::setAsyncStart(bool asyncStart) {
                this->asyncStart = asyncStart;
                return *this;
            }

            ClientConnectionStrategyConfig &
            ClientConnectionStrategyConfig::setReconnectMode(ReconnectMode reconnectMode) {
                this->reconnectMode = reconnectMode;
                return *this;
            }

        }
    }
}


namespace hazelcast {
    namespace client {
        namespace config {
            const int ReliableTopicConfig::DEFAULT_READ_BATCH_SIZE = 10;

            ReliableTopicConfig::ReliableTopicConfig() {

            }

            ReliableTopicConfig::ReliableTopicConfig(const char *topicName) : readBatchSize(DEFAULT_READ_BATCH_SIZE),
                                                                              name(topicName) {
            }

            const std::string &ReliableTopicConfig::getName() const {
                return name;
            }

            int ReliableTopicConfig::getReadBatchSize() const {
                return readBatchSize;
            }

            ReliableTopicConfig &ReliableTopicConfig::setReadBatchSize(int batchSize) {
                if (batchSize <= 0) {
                    throw exception::IllegalArgumentException("ReliableTopicConfig::setReadBatchSize",
                                                              "readBatchSize should be positive");
                }

                this->readBatchSize = batchSize;

                return *this;
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace config {
            SocketOptions::SocketOptions() : tcpNoDelay(true), keepAlive(true), reuseAddress(true), lingerSeconds(3),
                                             bufferSize(DEFAULT_BUFFER_SIZE_BYTE) {}

            bool SocketOptions::isTcpNoDelay() const {
                return tcpNoDelay;
            }

            SocketOptions &SocketOptions::setTcpNoDelay(bool tcpNoDelay) {
                SocketOptions::tcpNoDelay = tcpNoDelay;
                return *this;
            }

            bool SocketOptions::isKeepAlive() const {
                return keepAlive;
            }

            SocketOptions &SocketOptions::setKeepAlive(bool keepAlive) {
                SocketOptions::keepAlive = keepAlive;
                return *this;
            }

            bool SocketOptions::isReuseAddress() const {
                return reuseAddress;
            }

            SocketOptions &SocketOptions::setReuseAddress(bool reuseAddress) {
                SocketOptions::reuseAddress = reuseAddress;
                return *this;
            }

            int SocketOptions::getLingerSeconds() const {
                return lingerSeconds;
            }

            SocketOptions &SocketOptions::setLingerSeconds(int lingerSeconds) {
                SocketOptions::lingerSeconds = lingerSeconds;
                return *this;
            }

            int SocketOptions::getBufferSizeInBytes() const {
                return bufferSize;
            }

            SocketOptions &SocketOptions::setBufferSizeInBytes(int bufferSize) {
                SocketOptions::bufferSize = bufferSize;
                return *this;
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace config {
            ClientAwsConfig::ClientAwsConfig() : enabled(false), region("us-east-1"), hostHeader("ec2.amazonaws.com"),
                                                 insideAws(false) {
            }

            const std::string &ClientAwsConfig::getAccessKey() const {
                return accessKey;
            }

            ClientAwsConfig &ClientAwsConfig::setAccessKey(const std::string &accessKey) {
                this->accessKey = util::Preconditions::checkHasText(accessKey, "accessKey must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getSecretKey() const {
                return secretKey;
            }

            ClientAwsConfig &ClientAwsConfig::setSecretKey(const std::string &secretKey) {
                this->secretKey = util::Preconditions::checkHasText(secretKey, "secretKey must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getRegion() const {
                return region;
            }

            ClientAwsConfig &ClientAwsConfig::setRegion(const std::string &region) {
                this->region = util::Preconditions::checkHasText(region, "region must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getHostHeader() const {
                return hostHeader;
            }

            ClientAwsConfig &ClientAwsConfig::setHostHeader(const std::string &hostHeader) {
                this->hostHeader = util::Preconditions::checkHasText(hostHeader, "hostHeader must contain text");
                return *this;
            }

            ClientAwsConfig &ClientAwsConfig::setEnabled(bool enabled) {
                util::Preconditions::checkSSL("getAwsConfig");
                this->enabled = enabled;
                return *this;
            }

            bool ClientAwsConfig::isEnabled() const {
                return enabled;
            }

            ClientAwsConfig &ClientAwsConfig::setSecurityGroupName(const std::string &securityGroupName) {
                this->securityGroupName = securityGroupName;
                return *this;
            }

            const std::string &ClientAwsConfig::getSecurityGroupName() const {
                return securityGroupName;
            }

            ClientAwsConfig &ClientAwsConfig::setTagKey(const std::string &tagKey) {
                this->tagKey = tagKey;
                return *this;
            }

            const std::string &ClientAwsConfig::getTagKey() const {
                return tagKey;
            }

            ClientAwsConfig &ClientAwsConfig::setTagValue(const std::string &tagValue) {
                this->tagValue = tagValue;
                return *this;
            }

            const std::string &ClientAwsConfig::getTagValue() const {
                return tagValue;
            }

            const std::string &ClientAwsConfig::getIamRole() const {
                return iamRole;
            }

            ClientAwsConfig &ClientAwsConfig::setIamRole(const std::string &iamRole) {
                this->iamRole = iamRole;
                return *this;
            }

            bool ClientAwsConfig::isInsideAws() const {
                return insideAws;
            }

            ClientAwsConfig &ClientAwsConfig::setInsideAws(bool insideAws) {
                this->insideAws = insideAws;
                return *this;
            }

            std::ostream &operator<<(std::ostream &out, const ClientAwsConfig &config) {
                return out << "ClientAwsConfig{"
                           << "enabled=" << config.isEnabled()
                           << ", region='" << config.getRegion() << '\''
                           << ", securityGroupName='" << config.getSecurityGroupName() << '\''
                           << ", tagKey='" << config.getTagKey() << '\''
                           << ", tagValue='" << config.getTagValue() << '\''
                           << ", hostHeader='" << config.getHostHeader() << '\''
                           << ", iamRole='" << config.getIamRole() << "\'}";
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace mixedtype {
            MultiMap::MultiMap(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::MultiMapImpl(instanceName, context) {
            }

            std::vector<TypedData> MultiMap::keySet() {
                return toTypedDataCollection(proxy::MultiMapImpl::keySetData());
            }

            std::vector<TypedData> MultiMap::values() {
                return toTypedDataCollection(proxy::MultiMapImpl::valuesData());
            }

            std::vector<std::pair<TypedData, TypedData> > MultiMap::entrySet() {
                return toTypedDataEntrySet(proxy::MultiMapImpl::entrySetData());
            }

            int MultiMap::size() {
                return proxy::MultiMapImpl::size();
            }

            void MultiMap::clear() {
                proxy::MultiMapImpl::clear();
            }

            std::string MultiMap::addEntryListener(MixedEntryListener &listener, bool includeValue) {
                spi::ClientClusterService &clusterService = getContext().getClientClusterService();
                serialization::pimpl::SerializationService &ss = getContext().getSerializationService();
                impl::MixedEntryEventHandler<protocol::codec::MultiMapAddEntryListenerCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedEntryEventHandler<protocol::codec::MultiMapAddEntryListenerCodec::AbstractEventHandler>(
                                getName(), clusterService, ss, listener, includeValue);
                return proxy::MultiMapImpl::addEntryListener(entryEventHandler, includeValue);
            }

            bool MultiMap::removeEntryListener(const std::string &registrationId) {
                return proxy::MultiMapImpl::removeEntryListener(registrationId);
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            namespace impl {
                HazelcastClientImpl::HazelcastClientImpl(client::impl::HazelcastClientInstanceImpl &client) : client(
                        client) {
                }

                HazelcastClientImpl::~HazelcastClientImpl() {
                }

                IMap HazelcastClientImpl::getMap(const std::string &name) {
                    map::impl::MapMixedTypeProxyFactory factory(&client.clientContext);
                    std::shared_ptr<spi::ClientProxy> proxy =
                            client.getDistributedObjectForService("hz:impl:mapService", name, factory);
                    return IMap(std::static_pointer_cast<ClientMapProxy>(proxy));
                }

                MultiMap HazelcastClientImpl::getMultiMap(const std::string &name) {
                    return client.getDistributedObject<MultiMap>(name);
                }

                IQueue HazelcastClientImpl::getQueue(const std::string &name) {
                    return client.getDistributedObject<IQueue>(name);
                }

                ISet HazelcastClientImpl::getSet(const std::string &name) {
                    return client.getDistributedObject<ISet>(name);
                }

                IList HazelcastClientImpl::getList(const std::string &name) {
                    return client.getDistributedObject<IList>(name);
                }

                ITopic HazelcastClientImpl::getTopic(const std::string &name) {
                    return client.getDistributedObject<ITopic>(name);
                }

                Ringbuffer HazelcastClientImpl::getRingbuffer(
                        const std::string &instanceName) {
                    return client.getDistributedObject<Ringbuffer>(instanceName);
                }

            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace mixedtype {
            std::string IQueue::addItemListener(MixedItemListener &listener, bool includeValue) {
                spi::ClientClusterService &cs = getContext().getClientClusterService();
                serialization::pimpl::SerializationService &ss = getContext().getSerializationService();
                impl::MixedItemEventHandler<protocol::codec::QueueAddListenerCodec::AbstractEventHandler> *itemEventHandler =
                        new impl::MixedItemEventHandler<protocol::codec::QueueAddListenerCodec::AbstractEventHandler>(
                                getName(), cs, ss, listener);
                return proxy::IQueueImpl::addItemListener(itemEventHandler, includeValue);
            }

            bool IQueue::removeItemListener(const std::string &registrationId) {
                return proxy::IQueueImpl::removeItemListener(registrationId);
            }

            int IQueue::remainingCapacity() {
                return proxy::IQueueImpl::remainingCapacity();
            }

            TypedData IQueue::take() {
                return poll(-1);
            }

            size_t IQueue::drainTo(std::vector<TypedData> &elements) {
                return drainTo(elements, -1);
            }

            size_t IQueue::drainTo(std::vector<TypedData> &elements, int64_t maxElements) {
                typedef std::vector<serialization::pimpl::Data> DATA_VECTOR;
                serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
                size_t numElements = 0;
                for (const DATA_VECTOR::value_type data : proxy::IQueueImpl::drainToData((size_t) maxElements)) {
                    elements.push_back(TypedData(std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(data)), serializationService));
                    ++numElements;
                }
                return numElements;
            }

            TypedData IQueue::poll() {
                return poll(0);
            }

            TypedData IQueue::poll(long timeoutInMillis) {
                return TypedData(proxy::IQueueImpl::pollData(timeoutInMillis), getContext().getSerializationService());
            }

            TypedData IQueue::peek() {
                return TypedData(proxy::IQueueImpl::peekData(), getContext().getSerializationService());
            }

            int IQueue::size() {
                return proxy::IQueueImpl::size();
            }

            bool IQueue::isEmpty() {
                return size() == 0;
            }

            std::vector<TypedData> IQueue::toArray() {
                return toTypedDataCollection(proxy::IQueueImpl::toArrayData());
            }

            void IQueue::clear() {
                proxy::IQueueImpl::clear();
            }

            IQueue::IQueue(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::IQueueImpl(instanceName, context) {
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            IMap::IMap(std::shared_ptr<mixedtype::ClientMapProxy> proxy) : mapImpl(proxy) {
            }

            void IMap::removeAll(const query::Predicate &predicate) {
                mapImpl->removeAll(predicate);
            }

            void IMap::flush() {
                mapImpl->flush();
            }

            void IMap::removeInterceptor(const std::string &id) {
                mapImpl->removeInterceptor(id);
            }

            std::string IMap::addEntryListener(MixedEntryListener &listener, bool includeValue) {
                return mapImpl->addEntryListener(listener, includeValue);
            }

            std::string
            IMap::addEntryListener(MixedEntryListener &listener, const query::Predicate &predicate,
                                   bool includeValue) {
                return mapImpl->addEntryListener(listener, predicate, includeValue);
            }

            bool IMap::removeEntryListener(const std::string &registrationId) {
                return mapImpl->removeEntryListener(registrationId);
            }

            void IMap::evictAll() {
                mapImpl->evictAll();
            }

            std::vector<TypedData> IMap::keySet() {
                return mapImpl->keySet();
            }

            std::vector<TypedData> IMap::keySet(const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->keySet(predicate);
            }

            std::vector<TypedData> IMap::keySet(const query::Predicate &predicate) {
                return mapImpl->keySet(predicate);
            }

            std::vector<TypedData> IMap::values() {
                return mapImpl->values();
            }

            std::vector<TypedData> IMap::values(const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->values(predicate);
            }

            std::vector<TypedData> IMap::values(const query::Predicate &predicate) {
                return mapImpl->values(predicate);
            }

            std::vector<std::pair<TypedData, TypedData> > IMap::entrySet() {
                return mapImpl->entrySet();
            }

            std::vector<std::pair<TypedData, TypedData> >
            IMap::entrySet(const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->entrySet(predicate);
            }

            std::vector<std::pair<TypedData, TypedData> > IMap::entrySet(const query::Predicate &predicate) {
                return mapImpl->entrySet(predicate);
            }

            void IMap::addIndex(const std::string &attribute, bool ordered) {
                mapImpl->addIndex(attribute, ordered);
            }

            int IMap::size() {
                return mapImpl->size();
            }

            bool IMap::isEmpty() {
                return mapImpl->isEmpty();
            }

            void IMap::clear() {
                return mapImpl->clear();
            }

            void IMap::destroy() {
                mapImpl->destroy();
            }

            monitor::LocalMapStats &IMap::getLocalMapStats() {
                return mapImpl->getLocalMapStats();
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            Ringbuffer::Ringbuffer(const std::string &objectName, spi::ClientContext *context) : proxy::ProxyImpl(
                    "hz:impl:ringbufferService", objectName, context), bufferCapacity(-1) {
                partitionId = getPartitionId(toData(objectName));
            }

            Ringbuffer::Ringbuffer(const Ringbuffer &rhs) : proxy::ProxyImpl(rhs), partitionId(rhs.partitionId),
                                                            bufferCapacity(
                                                                    const_cast<Ringbuffer &>(rhs).bufferCapacity.load()) {
            }

            Ringbuffer::~Ringbuffer() {
            }

            int64_t Ringbuffer::capacity() {
                if (-1 == bufferCapacity) {
                    std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferCapacityCodec::encodeRequest(
                            getName());
                    bufferCapacity = invokeAndGetResult<int64_t, protocol::codec::RingbufferCapacityCodec::ResponseParameters>(
                            msg, partitionId);
                }
                return bufferCapacity;
            }

            int64_t Ringbuffer::size() {
                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferSizeCodec::encodeRequest(
                        getName());
                return invokeAndGetResult<int64_t, protocol::codec::RingbufferSizeCodec::ResponseParameters>(msg,
                                                                                                             partitionId);
            }

            int64_t Ringbuffer::tailSequence() {
                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferTailSequenceCodec::encodeRequest(
                        getName());
                return invokeAndGetResult<int64_t, protocol::codec::RingbufferTailSequenceCodec::ResponseParameters>(
                        msg, partitionId);
            }

            int64_t Ringbuffer::headSequence() {
                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferHeadSequenceCodec::encodeRequest(
                        getName());
                return invokeAndGetResult<int64_t, protocol::codec::RingbufferHeadSequenceCodec::ResponseParameters>(
                        msg, partitionId);
            }

            int64_t Ringbuffer::remainingCapacity() {
                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferRemainingCapacityCodec::encodeRequest(
                        getName());
                return invokeAndGetResult<int64_t, protocol::codec::RingbufferRemainingCapacityCodec::ResponseParameters>(
                        msg, partitionId);
            }

            TypedData Ringbuffer::readOne(int64_t sequence) {
                checkSequence(sequence);

                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferReadOneCodec::encodeRequest(
                        getName(), sequence);

                std::unique_ptr<serialization::pimpl::Data> itemData = invokeAndGetResult<
                        std::unique_ptr<serialization::pimpl::Data>, protocol::codec::RingbufferReadOneCodec::ResponseParameters>(
                        msg, partitionId);

                return TypedData(itemData, getContext().getSerializationService());
            }

            void Ringbuffer::checkSequence(int64_t sequence) {
                if (sequence < 0) {
                    throw (exception::ExceptionBuilder<exception::IllegalArgumentException>(
                            "Ringbuffer::checkSequence") << "sequence can't be smaller than 0, but was: "
                                                         << sequence).build();
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            std::string ITopic::addMessageListener(topic::MessageListener &listener) {
                client::impl::BaseEventHandler *topicEventHandler = new mixedtype::topic::impl::TopicEventHandlerImpl(
                        getName(), getContext().getClientClusterService(),
                        getContext().getSerializationService(),
                        listener);
                return proxy::ITopicImpl::addMessageListener(topicEventHandler);
            }

            bool ITopic::removeMessageListener(const std::string &registrationId) {
                return proxy::ITopicImpl::removeMessageListener(registrationId);
            }

            ITopic::ITopic(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ITopicImpl(instanceName, context) {
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            std::string ISet::addItemListener(MixedItemListener &listener, bool includeValue) {
                impl::MixedItemEventHandler<protocol::codec::SetAddListenerCodec::AbstractEventHandler> *itemEventHandler =
                        new impl::MixedItemEventHandler<protocol::codec::SetAddListenerCodec::AbstractEventHandler>(
                                getName(), getContext().getClientClusterService(),
                                getContext().getSerializationService(), listener);
                return proxy::ISetImpl::addItemListener(itemEventHandler, includeValue);
            }

            bool ISet::removeItemListener(const std::string &registrationId) {
                return proxy::ISetImpl::removeItemListener(registrationId);
            }

            int ISet::size() {
                return proxy::ISetImpl::size();
            }

            bool ISet::isEmpty() {
                return proxy::ISetImpl::isEmpty();
            }

            std::vector<TypedData> ISet::toArray() {
                return toTypedDataCollection(proxy::ISetImpl::toArrayData());
            }

            void ISet::clear() {
                proxy::ISetImpl::clear();
            }

            ISet::ISet(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ISetImpl(instanceName, context) {
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace mixedtype {
            NearCachedClientMapProxy::NearCachedClientMapProxy(const std::string &instanceName,
                                                               spi::ClientContext *context,
                                                               const mixedtype::config::MixedNearCacheConfig &config)
                    : ClientMapProxy(instanceName, context), cacheLocalEntries(false), invalidateOnChange(false),
                      keyStateMarker(NULL), nearCacheConfig(config) {
            }

            monitor::LocalMapStats &NearCachedClientMapProxy::getLocalMapStats() {
                monitor::LocalMapStats &localMapStats = ClientMapProxy::getLocalMapStats();
                monitor::NearCacheStats &nearCacheStats = nearCache->getNearCacheStats();
                ((monitor::impl::LocalMapStatsImpl &) localMapStats).setNearCacheStats(nearCacheStats);
                return localMapStats;
            }

            void NearCachedClientMapProxy::onInitialize() {
                ClientMapProxy::onInitialize();

                internal::nearcache::NearCacheManager &nearCacheManager = getContext().getNearCacheManager();
                cacheLocalEntries = nearCacheConfig.isCacheLocalEntries();
                int partitionCount = getContext().getPartitionService().getPartitionCount();
                nearCache = nearCacheManager.getOrCreateNearCache<TypedData, TypedData, serialization::pimpl::Data>(
                        proxy::ProxyImpl::getName(), nearCacheConfig);

                nearCache = map::impl::nearcache::InvalidationAwareWrapper<serialization::pimpl::Data, TypedData>::asInvalidationAware(
                        nearCache, partitionCount);

                keyStateMarker = getKeyStateMarker();

                invalidateOnChange = nearCache->isInvalidatedOnChange();
                if (invalidateOnChange) {
                    std::unique_ptr<client::impl::BaseEventHandler> invalidationHandler(
                            new ClientMapAddNearCacheEventHandler(nearCache));
                    addNearCacheInvalidateListener(invalidationHandler);
                }
            }

            //@Override
            bool NearCachedClientMapProxy::containsKeyInternal(const serialization::pimpl::Data &keyData) {
                std::shared_ptr<serialization::pimpl::Data> key = toShared(keyData);
                std::shared_ptr<TypedData> cached = nearCache->get(key);
                if (cached.get() != NULL) {
                    return internal::nearcache::NearCache<serialization::pimpl::Data, TypedData>::NULL_OBJECT != cached;
                }

                return ClientMapProxy::containsKeyInternal(*key);
            }

            //@override
            std::shared_ptr<TypedData> NearCachedClientMapProxy::getInternal(serialization::pimpl::Data &keyData) {
                std::shared_ptr<serialization::pimpl::Data> key = ClientMapProxy::toShared(keyData);
                std::shared_ptr<TypedData> cached = nearCache->get(key);
                if (cached.get() != NULL) {
                    if (internal::nearcache::NearCache<serialization::pimpl::Data, TypedData>::NULL_OBJECT == cached) {
                        return std::shared_ptr<TypedData>(
                                new TypedData(std::unique_ptr<serialization::pimpl::Data>(),
                                              getSerializationService()));
                    }
                    return cached;
                }

                bool marked = keyStateMarker->tryMark(*key);

                try {
                    std::shared_ptr<TypedData> value = ClientMapProxy::getInternal(*key);
                    if (marked && value->getData().get()) {
                        tryToPutNearCache(key, value);
                    }
                    return value;
                } catch (exception::IException &) {
                    resetToUnmarkedState(key);
                    throw;
                }
            }

            //@Override
            std::unique_ptr<serialization::pimpl::Data> NearCachedClientMapProxy::removeInternal(
                    const serialization::pimpl::Data &key) {
                try {
                    std::unique_ptr<serialization::pimpl::Data> responseData = ClientMapProxy::removeInternal(key);
                    invalidateNearCache(key);
                    return responseData;
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            //@Override
            bool NearCachedClientMapProxy::removeInternal(
                    const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                try {
                    bool response = ClientMapProxy::removeInternal(key, value);
                    invalidateNearCache(key);
                    return response;
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            void NearCachedClientMapProxy::removeAllInternal(const serialization::pimpl::Data &predicateData) {
                try {
                    ClientMapProxy::removeAllInternal(predicateData);

                    nearCache->clear();
                } catch (exception::IException &) {
                    nearCache->clear();
                    throw;
                }
            }

            void NearCachedClientMapProxy::deleteInternal(const serialization::pimpl::Data &key) {
                try {
                    ClientMapProxy::deleteInternal(key);
                    invalidateNearCache(key);
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            bool
            NearCachedClientMapProxy::tryRemoveInternal(const serialization::pimpl::Data &key, long timeoutInMillis) {
                try {
                    bool response = ClientMapProxy::tryRemoveInternal(key, timeoutInMillis);
                    invalidateNearCache(key);
                    return response;
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            bool NearCachedClientMapProxy::tryPutInternal(const serialization::pimpl::Data &key,
                                                          const serialization::pimpl::Data &value,
                                                          long timeoutInMillis) {
                try {
                    bool response = ClientMapProxy::tryPutInternal(key, value, timeoutInMillis);
                    invalidateNearCache(key);
                    return response;
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            std::unique_ptr<serialization::pimpl::Data>
            NearCachedClientMapProxy::putInternal(const serialization::pimpl::Data &key,
                                                  const serialization::pimpl::Data &value,
                                                  long timeoutInMillis) {
                try {
                    std::unique_ptr<serialization::pimpl::Data> previousValue =
                            ClientMapProxy::putInternal(key, value, timeoutInMillis);
                    invalidateNearCache(key);
                    return previousValue;
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            void NearCachedClientMapProxy::tryPutTransientInternal(const serialization::pimpl::Data &key,
                                                                   const serialization::pimpl::Data &value,
                                                                   int64_t ttlInMillis) {
                try {
                    ClientMapProxy::tryPutTransientInternal(key, value, ttlInMillis);
                    invalidateNearCache(key);
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            std::unique_ptr<serialization::pimpl::Data>
            NearCachedClientMapProxy::putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                                          const serialization::pimpl::Data &valueData,
                                                          int64_t ttlInMillis) {
                try {
                    std::unique_ptr<serialization::pimpl::Data> previousValue =
                            ClientMapProxy::putIfAbsentData(keyData, valueData, ttlInMillis);
                    invalidateNearCache(keyData);
                    return previousValue;
                } catch (exception::IException &) {
                    invalidateNearCache(keyData);
                    throw;
                }
            }

            bool NearCachedClientMapProxy::replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                                                 const serialization::pimpl::Data &valueData,
                                                                 const serialization::pimpl::Data &newValueData) {
                try {
                    bool result = proxy::IMapImpl::replace(keyData, valueData, newValueData);
                    invalidateNearCache(keyData);
                    return result;
                } catch (exception::IException &) {
                    invalidateNearCache(keyData);
                    throw;
                }
            }

            std::unique_ptr<serialization::pimpl::Data>
            NearCachedClientMapProxy::replaceInternal(const serialization::pimpl::Data &keyData,
                                                      const serialization::pimpl::Data &valueData) {
                try {
                    std::unique_ptr<serialization::pimpl::Data> value =
                            proxy::IMapImpl::replaceData(keyData, valueData);
                    invalidateNearCache(keyData);
                    return value;
                } catch (exception::IException &) {
                    invalidateNearCache(keyData);
                    throw;
                }
            }

            void NearCachedClientMapProxy::setInternal(const serialization::pimpl::Data &keyData,
                                                       const serialization::pimpl::Data &valueData,
                                                       int64_t ttlInMillis) {
                try {
                    proxy::IMapImpl::set(keyData, valueData, ttlInMillis);
                    invalidateNearCache(keyData);
                } catch (exception::IException &) {
                    invalidateNearCache(keyData);
                    throw;
                }
            }

            bool NearCachedClientMapProxy::evictInternal(const serialization::pimpl::Data &keyData) {
                try {
                    bool evicted = proxy::IMapImpl::evict(keyData);
                    invalidateNearCache(keyData);
                    return evicted;
                } catch (exception::IException &) {
                    invalidateNearCache(keyData);
                    throw;
                }
            }

            EntryVector NearCachedClientMapProxy::getAllInternal(const ClientMapProxy::PID_TO_KEY_MAP &pIdToKeyData) {
                MARKER_MAP markers;
                try {
                    ClientMapProxy::PID_TO_KEY_MAP nonCachedPidToKeyMap;
                    EntryVector result = populateFromNearCache(pIdToKeyData, nonCachedPidToKeyMap, markers);

                    EntryVector responses = ClientMapProxy::getAllInternal(nonCachedPidToKeyMap);
                    for (const EntryVector::value_type &entry : responses) {
                        std::shared_ptr<serialization::pimpl::Data> key = ClientMapProxy::toShared(
                                entry.first);
                        std::shared_ptr<TypedData> value = std::shared_ptr<TypedData>(new TypedData(
                                std::unique_ptr<serialization::pimpl::Data>(
                                        new serialization::pimpl::Data(entry.second)),
                                getSerializationService()));
                        bool marked = false;
                        if (markers.count(key)) {
                            marked = markers[key];
                            markers.erase(key);
                        }

                        if (marked) {
                            tryToPutNearCache(key, value);
                        } else {
                            nearCache->put(key, value);
                        }
                    }

                    unmarkRemainingMarkedKeys(markers);

                    result.insert(result.end(), responses.begin(), responses.end());

                    return result;
                } catch (exception::IException &) {
                    unmarkRemainingMarkedKeys(markers);
                    throw;
                }
            }

            std::unique_ptr<serialization::pimpl::Data>
            NearCachedClientMapProxy::executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                                                           const serialization::pimpl::Data &processor) {
                try {
                    std::unique_ptr<serialization::pimpl::Data> response =
                            ClientMapProxy::executeOnKeyData(keyData, processor);
                    invalidateNearCache(keyData);
                    return response;
                } catch (exception::IException &) {
                    invalidateNearCache(keyData);
                    throw;
                }
            }

            void
            NearCachedClientMapProxy::putAllInternal(const std::map<int, EntryVector> &entries) {
                try {
                    ClientMapProxy::putAllInternal(entries);
                    invalidateEntries(entries);
                } catch (exception::IException &) {
                    invalidateEntries(entries);
                    throw;
                }
            }

            void NearCachedClientMapProxy::invalidateEntries(const std::map<int, EntryVector> &entries) {
                for (std::map<int, EntryVector>::const_iterator it = entries.begin(); it != entries.end(); ++it) {
                    for (EntryVector::const_iterator entryIt = it->second.begin();
                         entryIt != it->second.end(); ++entryIt) {
                        invalidateNearCache(ClientMapProxy::toShared(entryIt->first));
                    }
                }
            }

            map::impl::nearcache::KeyStateMarker *NearCachedClientMapProxy::getKeyStateMarker() {
                return std::static_pointer_cast<
                        map::impl::nearcache::InvalidationAwareWrapper<serialization::pimpl::Data, TypedData> >(
                        nearCache)->
                        getKeyStateMarker();
            }

            void NearCachedClientMapProxy::addNearCacheInvalidateListener(
                    std::unique_ptr<client::impl::BaseEventHandler> &handler) {
                try {
                    invalidationListenerId = std::shared_ptr<std::string>(
                            new std::string(proxy::ProxyImpl::registerListener(
                                    createNearCacheEntryListenerCodec(), handler.release())));

                } catch (exception::IException &e) {
                    std::ostringstream out;
                    out << "-----------------\n Near Cache is not initialized!!! \n-----------------";
                    out << e.what();
                    getContext().getLogger().severe(out.str());
                }
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            NearCachedClientMapProxy::createNearCacheEntryListenerCodec() {
                int32_t listenerFlags = EntryEventType::INVALIDATION;
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new NearCacheEntryListenerMessageCodec(getName(), listenerFlags));
            }

            void NearCachedClientMapProxy::resetToUnmarkedState(std::shared_ptr<serialization::pimpl::Data> &key) {
                if (keyStateMarker->tryUnmark(*key)) {
                    return;
                }

                invalidateNearCache(key);
                keyStateMarker->forceUnmark(*key);
            }

            void NearCachedClientMapProxy::unmarkRemainingMarkedKeys(
                    std::map<std::shared_ptr<serialization::pimpl::Data>, bool> &markers) {
                for (std::map<std::shared_ptr<serialization::pimpl::Data>, bool>::const_iterator it = markers.begin();
                     it != markers.end(); ++it) {
                    if (it->second) {
                        keyStateMarker->forceUnmark(*it->first);
                    }
                }
            }

            void NearCachedClientMapProxy::tryToPutNearCache(std::shared_ptr<serialization::pimpl::Data> &keyData,
                                                             std::shared_ptr<TypedData> &response) {
                try {
                    if (response.get()) {
                        nearCache->put(keyData, response);
                    }
                    resetToUnmarkedState(keyData);
                } catch (exception::IException &) {
                    resetToUnmarkedState(keyData);
                    throw;
                }
            }

            /**
             * This method modifies the key Data internal pointer although it is marked as const
             * @param key The key for which to invalidate the near cache
             */
            void NearCachedClientMapProxy::invalidateNearCache(const serialization::pimpl::Data &key) {
                nearCache->invalidate(ClientMapProxy::toShared(key));
            }

            void NearCachedClientMapProxy::invalidateNearCache(std::shared_ptr<serialization::pimpl::Data> key) {
                nearCache->invalidate(key);
            }

            EntryVector
            NearCachedClientMapProxy::populateFromNearCache(const ClientMapProxy::PID_TO_KEY_MAP &pIdToKeyData,
                                                            PID_TO_KEY_MAP &nonCachedPidToKeyMap, MARKER_MAP &markers) {
                EntryVector result;

                for (const ClientMapProxy::PID_TO_KEY_MAP::value_type &partitionDatas : pIdToKeyData) {
                    typedef std::vector<std::shared_ptr<serialization::pimpl::Data> > SHARED_DATA_VECTOR;
                    SHARED_DATA_VECTOR nonCachedData;
                    for (const SHARED_DATA_VECTOR::value_type &keyData : partitionDatas.second) {
                        std::shared_ptr<TypedData> cached = nearCache->get(keyData);
                        if (cached.get() != NULL && !cached->getData().get() &&
                            internal::nearcache::NearCache<serialization::pimpl::Data, TypedData>::NULL_OBJECT !=
                            cached) {
                            serialization::pimpl::Data valueData(*cached->getData());
                            result.push_back(std::make_pair(*keyData, valueData));
                        } else if (invalidateOnChange) {
                            markers[keyData] = keyStateMarker->tryMark(*keyData);
                            nonCachedData.push_back(keyData);
                        }
                    }
                    nonCachedPidToKeyMap[partitionDatas.first] = nonCachedData;
                }
                return result;
            }

            std::unique_ptr<protocol::ClientMessage>
            NearCachedClientMapProxy::NearCacheEntryListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MapAddNearCacheEntryListenerCodec::encodeRequest(name, listenerFlags,
                                                                                         localOnly);
            }

            std::string NearCachedClientMapProxy::NearCacheEntryListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MapAddNearCacheEntryListenerCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            NearCachedClientMapProxy::NearCacheEntryListenerMessageCodec::encodeRemoveRequest(
                    const std::string &realRegistrationId) const {
                return protocol::codec::MapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool NearCachedClientMapProxy::NearCacheEntryListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MapRemoveEntryListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

            NearCachedClientMapProxy::NearCacheEntryListenerMessageCodec::NearCacheEntryListenerMessageCodec(
                    const std::string &name, int32_t listenerFlags) : name(name), listenerFlags(listenerFlags) {}
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            IList::IList(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::IListImpl(instanceName, context) {
            }

            std::string IList::addItemListener(MixedItemListener &listener, bool includeValue) {
                impl::MixedItemEventHandler<protocol::codec::ListAddListenerCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedItemEventHandler<protocol::codec::ListAddListenerCodec::AbstractEventHandler>(
                                getName(), (spi::ClientClusterService &) getContext().getClientClusterService(),
                                getContext().getSerializationService(), listener);
                return proxy::IListImpl::addItemListener(entryEventHandler, includeValue);
            }

            bool IList::removeItemListener(const std::string &registrationId) {
                return proxy::IListImpl::removeItemListener(registrationId);
            }

            int IList::size() {
                return proxy::IListImpl::size();
            }

            bool IList::isEmpty() {
                return size() == 0;
            }

            std::vector<TypedData> IList::toArray() {
                return toTypedDataCollection(proxy::IListImpl::toArrayData());
            }

            void IList::clear() {
                proxy::IListImpl::clear();
            }

            TypedData IList::get(int index) {
                return TypedData(proxy::IListImpl::getData(index), getContext().getSerializationService());
            }

            TypedData IList::remove(int index) {
                return TypedData(proxy::IListImpl::removeData(index), getContext().getSerializationService());
            }

            std::vector<TypedData> IList::subList(int fromIndex, int toIndex) {
                return toTypedDataCollection(proxy::IListImpl::subListData(fromIndex, toIndex));
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            ClientMapProxy::ClientMapProxy(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::IMapImpl(instanceName, context) {
            }

            void ClientMapProxy::removeAll(const query::Predicate &predicate) {
                serialization::pimpl::Data predicateData = toData(predicate);

                removeAllInternal(predicateData);
            }

            void ClientMapProxy::flush() {
                proxy::IMapImpl::flush();
            }

            void ClientMapProxy::removeInterceptor(const std::string &id) {
                proxy::IMapImpl::removeInterceptor(id);
            }

            std::string ClientMapProxy::addEntryListener(MixedEntryListener &listener, bool includeValue) {
                impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler>(
                                getName(), getContext().getClientClusterService(),
                                getContext().getSerializationService(),
                                listener,
                                includeValue);
                return proxy::IMapImpl::addEntryListener(entryEventHandler, includeValue);
            }

            std::string
            ClientMapProxy::addEntryListener(MixedEntryListener &listener, const query::Predicate &predicate,
                                             bool includeValue) {
                impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler>(
                                getName(), getContext().getClientClusterService(),
                                getContext().getSerializationService(),
                                listener,
                                includeValue);
                return proxy::IMapImpl::addEntryListener(entryEventHandler, predicate, includeValue);
            }

            bool ClientMapProxy::removeEntryListener(const std::string &registrationId) {
                return proxy::IMapImpl::removeEntryListener(registrationId);
            }

            void ClientMapProxy::evictAll() {
                proxy::IMapImpl::evictAll();
            }

            std::vector<TypedData> ClientMapProxy::keySet() {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::keySetData();
                size_t size = dataResult.size();
                std::vector<TypedData> keys(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> keyData(new serialization::pimpl::Data(dataResult[i]));
                    keys[i] = TypedData(keyData, getContext().getSerializationService());
                }
                return keys;
            }

            std::vector<TypedData> ClientMapProxy::keySet(const serialization::IdentifiedDataSerializable &predicate) {
                const query::Predicate *p = (const query::Predicate *) (&predicate);
                return keySet(*p);
            }

            std::vector<TypedData> ClientMapProxy::keySet(const query::Predicate &predicate) {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::keySetData(predicate);
                size_t size = dataResult.size();
                std::vector<TypedData> keys(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> keyData(new serialization::pimpl::Data(dataResult[i]));
                    keys[i] = TypedData(keyData, getContext().getSerializationService());
                }
                return keys;
            }

            std::vector<TypedData> ClientMapProxy::values() {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::valuesData();
                size_t size = dataResult.size();
                std::vector<TypedData> values(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i]));
                    values[i] = TypedData(valueData, getContext().getSerializationService());
                }
                return values;
            }

            std::vector<TypedData> ClientMapProxy::values(const serialization::IdentifiedDataSerializable &predicate) {
                const query::Predicate *p = (const query::Predicate *) (&predicate);
                return values(*p);
            }

            std::vector<TypedData> ClientMapProxy::values(const query::Predicate &predicate) {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::valuesData(predicate);
                size_t size = dataResult.size();
                std::vector<TypedData> values(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i]));
                    values[i] = TypedData(valueData, getContext().getSerializationService());
                }
                return values;
            }

            std::vector<std::pair<TypedData, TypedData> > ClientMapProxy::entrySet() {
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData();
                size_t size = dataResult.size();
                std::vector<std::pair<TypedData, TypedData> > entries(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> keyData(
                            new serialization::pimpl::Data(dataResult[i].first));
                    std::unique_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i].second));
                    serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
                    entries[i] = std::make_pair(TypedData(keyData, serializationService), TypedData(valueData,
                                                                                                    serializationService));
                }
                return entries;
            }

            std::vector<std::pair<TypedData, TypedData> >
            ClientMapProxy::entrySet(const serialization::IdentifiedDataSerializable &predicate) {
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData(
                        predicate);
                size_t size = dataResult.size();
                std::vector<std::pair<TypedData, TypedData> > entries(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> keyData(
                            new serialization::pimpl::Data(dataResult[i].first));
                    std::unique_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i].second));
                    serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
                    entries[i] = std::make_pair(TypedData(keyData, serializationService),
                                                TypedData(valueData, serializationService));
                }
                return entries;
            }

            std::vector<std::pair<TypedData, TypedData> > ClientMapProxy::entrySet(const query::Predicate &predicate) {
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData(
                        predicate);
                size_t size = dataResult.size();
                std::vector<std::pair<TypedData, TypedData> > entries(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> keyData(
                            new serialization::pimpl::Data(dataResult[i].first));
                    std::unique_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i].second));
                    serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
                    entries[i] = std::make_pair(TypedData(keyData, serializationService),
                                                TypedData(valueData, serializationService));
                }
                return entries;
            }

            void ClientMapProxy::addIndex(const std::string &attribute, bool ordered) {
                proxy::IMapImpl::addIndex(attribute, ordered);
            }

            int ClientMapProxy::size() {
                return proxy::IMapImpl::size();
            }

            bool ClientMapProxy::isEmpty() {
                return proxy::IMapImpl::isEmpty();
            }

            void ClientMapProxy::clear() {
                proxy::IMapImpl::clear();
            }

            serialization::pimpl::SerializationService &ClientMapProxy::getSerializationService() {
                return getContext().getSerializationService();
            }

            monitor::LocalMapStats &ClientMapProxy::getLocalMapStats() {
                return stats;
            }

            std::shared_ptr<TypedData> ClientMapProxy::getInternal(serialization::pimpl::Data &keyData) {
                std::unique_ptr<serialization::pimpl::Data> valueData = proxy::IMapImpl::getData(keyData);
                return std::shared_ptr<TypedData>(new TypedData(valueData, getContext().getSerializationService()));
            }

            bool ClientMapProxy::containsKeyInternal(const serialization::pimpl::Data &keyData) {
                return proxy::IMapImpl::containsKey(keyData);
            }

            std::unique_ptr<serialization::pimpl::Data> ClientMapProxy::removeInternal(
                    const serialization::pimpl::Data &keyData) {
                return proxy::IMapImpl::removeData(keyData);
            }

            bool ClientMapProxy::removeInternal(
                    const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData) {
                return proxy::IMapImpl::remove(keyData, valueData);
            }

            void ClientMapProxy::removeAllInternal(const serialization::pimpl::Data &predicateData) {
                return proxy::IMapImpl::removeAll(predicateData);
            }

            void ClientMapProxy::deleteInternal(const serialization::pimpl::Data &keyData) {
                proxy::IMapImpl::deleteEntry(keyData);
            }

            bool
            ClientMapProxy::tryRemoveInternal(const serialization::pimpl::Data &keyData, long timeoutInMillis) {
                return proxy::IMapImpl::tryRemove(keyData, timeoutInMillis);
            }

            bool ClientMapProxy::tryPutInternal(const serialization::pimpl::Data &keyData,
                                                const serialization::pimpl::Data &valueData,
                                                long timeoutInMillis) {
                return proxy::IMapImpl::tryPut(keyData, valueData, timeoutInMillis);
            }

            std::unique_ptr<serialization::pimpl::Data>
            ClientMapProxy::putInternal(const serialization::pimpl::Data &keyData,
                                        const serialization::pimpl::Data &valueData, long timeoutInMillis) {
                return proxy::IMapImpl::putData(keyData, valueData, timeoutInMillis);
            }

            void ClientMapProxy::tryPutTransientInternal(const serialization::pimpl::Data &keyData,
                                                         const serialization::pimpl::Data &valueData,
                                                         int64_t ttlInMillis) {
                proxy::IMapImpl::tryPut(keyData, valueData, ttlInMillis);
            }

            std::unique_ptr<serialization::pimpl::Data>
            ClientMapProxy::putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                                const serialization::pimpl::Data &valueData,
                                                int64_t ttlInMillis) {
                return proxy::IMapImpl::putIfAbsentData(keyData, valueData, ttlInMillis);
            }

            bool ClientMapProxy::replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                                       const serialization::pimpl::Data &valueData,
                                                       const serialization::pimpl::Data &newValueData) {
                return proxy::IMapImpl::replace(keyData, valueData, newValueData);
            }

            std::unique_ptr<serialization::pimpl::Data>
            ClientMapProxy::replaceInternal(const serialization::pimpl::Data &keyData,
                                            const serialization::pimpl::Data &valueData) {
                return proxy::IMapImpl::replaceData(keyData, valueData);

            }

            void ClientMapProxy::setInternal(const serialization::pimpl::Data &keyData,
                                             const serialization::pimpl::Data &valueData,
                                             int64_t ttlInMillis) {
                proxy::IMapImpl::set(keyData, valueData, ttlInMillis);
            }

            bool ClientMapProxy::evictInternal(const serialization::pimpl::Data &keyData) {
                return proxy::IMapImpl::evict(keyData);
            }

            EntryVector ClientMapProxy::getAllInternal(const PID_TO_KEY_MAP &partitionToKeyData) {
                std::map<int, std::vector<serialization::pimpl::Data> > datas;
                for (PID_TO_KEY_MAP::const_iterator it = partitionToKeyData.begin();
                     it != partitionToKeyData.end(); ++it) {
                    const std::vector<std::shared_ptr<serialization::pimpl::Data> > &valueDatas = it->second;
                    for (std::vector<std::shared_ptr<serialization::pimpl::Data> >::const_iterator valueIt = valueDatas.begin();
                         valueIt != valueDatas.end(); ++valueIt) {
                        datas[it->first].push_back(*(*valueIt));
                    }
                }
                return proxy::IMapImpl::getAllData(datas);
            }

            std::unique_ptr<serialization::pimpl::Data>
            ClientMapProxy::executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                                                 const serialization::pimpl::Data &processor) {
                return proxy::IMapImpl::executeOnKeyData(keyData, processor);
            }

            std::unique_ptr<serialization::pimpl::Data>
            ClientMapProxy::submitToKeyDecoder(protocol::ClientMessage &response) {
                return protocol::codec::MapExecuteOnKeyCodec::ResponseParameters::decode(response).response;
            }

            void
            ClientMapProxy::putAllInternal(const std::map<int, EntryVector> &entries) {
                proxy::IMapImpl::putAllData(entries);
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        const int Address::ID = cluster::impl::ADDRESS;

        const byte Address::IPV4 = 4;
        const byte Address::IPV6 = 6;

        Address::Address() : host("localhost"), type(IPV4) {
        }

        Address::Address(const std::string &url, int port)
                : host(url), port(port), type(IPV4) {
        }

        Address::Address(const std::string &hostname, int port, unsigned long scopeId) : host(hostname), port(port),
                                                                                         type(IPV6), scopeId(scopeId) {
        }

        bool Address::operator==(const Address &rhs) const {
            return rhs.port == port && rhs.type == type && 0 == rhs.host.compare(host);
        }

        bool Address::operator!=(const Address &rhs) const {
            return !(*this == rhs);
        }

        int Address::getPort() const {
            return port;
        }

        const std::string &Address::getHost() const {
            return host;
        }

        int Address::getFactoryId() const {
            return cluster::impl::F_ID;
        }

        int Address::getClassId() const {
            return ID;
        }

        void Address::writeData(serialization::ObjectDataOutput &out) const {
            out.writeInt(port);
            out.writeByte(type);
            int len = (int) host.size();
            out.writeInt(len);
            out.writeBytes((const byte *) host.c_str(), len);
        }

        void Address::readData(serialization::ObjectDataInput &in) {
            port = in.readInt();
            type = in.readByte();
            int len = in.readInt();
            if (len > 0) {
                std::vector<byte> bytes;
                in.readFully(bytes);
                host.clear();
                host.append(bytes.begin(), bytes.end());
            }
        }

        bool Address::operator<(const Address &rhs) const {
            if (host < rhs.host) {
                return true;
            }
            if (rhs.host < host) {
                return false;
            }
            if (port < rhs.port) {
                return true;
            }
            if (rhs.port < port) {
                return false;
            }
            return type < rhs.type;
        }

        bool Address::isIpV4() const {
            return type == IPV4;
        }

        unsigned long Address::getScopeId() const {
            return scopeId;
        }

        std::string Address::toString() const {
            std::ostringstream out;
            out << "Address[" << getHost() << ":" << getPort() << "]";
            return out.str();
        }

        std::ostream &operator<<(std::ostream &stream, const Address &address) {
            return stream << address.toString();
        }

    }
}


namespace hazelcast {
    namespace client {
        MembershipEvent::MembershipEvent(Cluster &cluster, const Member &member, MembershipEventType eventType,
                                         const std::vector<Member> &membersList) :
                cluster(&cluster), member(member), eventType(eventType), members(membersList) {
        }

        MembershipEvent::~MembershipEvent() {
        }

        const std::vector<Member> MembershipEvent::getMembers() const {
            return members;
        }

        const Cluster &MembershipEvent::getCluster() const {
            return *cluster;
        }

        MembershipEvent::MembershipEventType MembershipEvent::getEventType() const {
            return eventType;
        }

        const Member &MembershipEvent::getMember() const {
            return member;
        }
    }
}


namespace hazelcast {
    namespace client {
        TransactionContext::TransactionContext(spi::impl::ClientTransactionManagerServiceImpl &transactionManager,
                                               const TransactionOptions &txnOptions) : options(txnOptions),
                                                                                       txnConnection(
                                                                                               transactionManager.connect()),
                                                                                       transaction(options,
                                                                                                   transactionManager.getClient(),
                                                                                                   txnConnection) {
        }

        std::string TransactionContext::getTxnId() const {
            return transaction.getTxnId();
        }

        void TransactionContext::beginTransaction() {
            transaction.begin();
        }

        void TransactionContext::commitTransaction() {
            transaction.commit();
        }

        void TransactionContext::rollbackTransaction() {
            transaction.rollback();
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            ClassDefinitionBuilder::ClassDefinitionBuilder(int factoryId, int classId, int version)
                    : factoryId(factoryId), classId(classId), version(version), index(0), done(false) {

            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addIntField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_INT);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addLongField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_LONG);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addUTFField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_UTF);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addBooleanField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_BOOLEAN);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addByteField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_BYTE);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addCharField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_CHAR);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addDoubleField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_DOUBLE);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addFloatField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_FLOAT);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addShortField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_SHORT);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addByteArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addBooleanArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_BOOLEAN_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addCharArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addIntArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_INT_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addLongArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_LONG_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addDoubleArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addFloatArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addShortArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addPortableField(const std::string &fieldName,
                                                                             std::shared_ptr<ClassDefinition> def) {
                check();
                if (def->getClassId() == 0) {
                    throw exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField",
                                                              "Portable class id cannot be zero!");
                }
                FieldDefinition fieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE, def->getFactoryId(),
                                                def->getClassId(), def->getVersion());
                fieldDefinitions.push_back(fieldDefinition);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addPortableArrayField(const std::string &fieldName,
                                                                                  std::shared_ptr<ClassDefinition> def) {
                check();
                if (def->getClassId() == 0) {
                    throw exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField",
                                                              "Portable class id cannot be zero!");
                }
                FieldDefinition fieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE_ARRAY,
                                                def->getFactoryId(), def->getClassId(), def->getVersion());
                fieldDefinitions.push_back(fieldDefinition);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addField(FieldDefinition &fieldDefinition) {
                check();
                int defIndex = fieldDefinition.getIndex();
                if (index != defIndex) {
                    char buf[100];
                    util::hz_snprintf(buf, 100, "Invalid field index. Index in definition:%d, being added at index:%d",
                                      defIndex, index);
                    throw exception::IllegalArgumentException("ClassDefinitionBuilder::addField", buf);
                }
                index++;
                fieldDefinitions.push_back(fieldDefinition);
                return *this;
            }

            std::shared_ptr<ClassDefinition> ClassDefinitionBuilder::build() {
                done = true;
                std::shared_ptr<ClassDefinition> cd(new ClassDefinition(factoryId, classId, version));

                std::vector<FieldDefinition>::iterator fdIt;
                for (fdIt = fieldDefinitions.begin(); fdIt != fieldDefinitions.end(); fdIt++) {
                    cd->addFieldDef(*fdIt);
                }
                return cd;
            }

            void ClassDefinitionBuilder::check() {
                if (done) {
                    throw exception::HazelcastSerializationException("ClassDefinitionBuilder::check",
                                                                     "ClassDefinition is already built for " +
                                                                     util::IOUtil::to_string(classId));
                }
            }

            void ClassDefinitionBuilder::addField(const std::string &fieldName, FieldType const &fieldType) {
                check();
                FieldDefinition fieldDefinition(index++, fieldName, fieldType, version);
                fieldDefinitions.push_back(fieldDefinition);
            }

            int ClassDefinitionBuilder::getFactoryId() {
                return factoryId;
            }

            int ClassDefinitionBuilder::getClassId() {
                return classId;
            }

            int ClassDefinitionBuilder::getVersion() {
                return version;
            }
        }
    }
}

//
//  FieldDefinition.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace serialization {
            FieldDefinition::FieldDefinition()
                    : index(0), classId(0), factoryId(0), version(-1) {
            }

            FieldDefinition::FieldDefinition(int index, const std::string &fieldName, FieldType const &type,
                                             int version)
                    : index(index), fieldName(fieldName), type(type), classId(0), factoryId(0), version(version) {
            }

            FieldDefinition::FieldDefinition(int index, const std::string &fieldName, FieldType const &type,
                                             int factoryId, int classId, int version)
                    : index(index), fieldName(fieldName), type(type), classId(classId), factoryId(factoryId),
                      version(version) {
            }

            const FieldType &FieldDefinition::getType() const {
                return type;
            }

            std::string FieldDefinition::getName() const {
                return fieldName;
            }

            int FieldDefinition::getIndex() const {
                return index;
            }

            int FieldDefinition::getFactoryId() const {
                return factoryId;
            }

            int FieldDefinition::getClassId() const {
                return classId;
            }

            void FieldDefinition::writeData(pimpl::DataOutput &dataOutput) {
                dataOutput.writeInt(index);
                dataOutput.writeUTF(&fieldName);
                dataOutput.writeByte(type.getId());
                dataOutput.writeInt(factoryId);
                dataOutput.writeInt(classId);
            }

            void FieldDefinition::readData(pimpl::DataInput &dataInput) {
                index = dataInput.readInt();
                fieldName = *dataInput.readUTF();
                type.id = dataInput.readByte();
                factoryId = dataInput.readInt();
                classId = dataInput.readInt();
            }

            bool FieldDefinition::operator==(const FieldDefinition &rhs) const {
                return fieldName == rhs.fieldName &&
                       type == rhs.type &&
                       classId == rhs.classId &&
                       factoryId == rhs.factoryId &&
                       version == rhs.version;
            }

            bool FieldDefinition::operator!=(const FieldDefinition &rhs) const {
                return !(rhs == *this);
            }

            std::ostream &operator<<(std::ostream &os, const FieldDefinition &definition) {
                os << "FieldDefinition{" << "index: " << definition.index << " fieldName: " << definition.fieldName
                   << " type: " << definition.type << " classId: " << definition.classId << " factoryId: "
                   << definition.factoryId << " version: " << definition.version;
                return os;
            }

        }
    }
}
//
//  ObjectDataInput.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace serialization {

            ObjectDataInput::ObjectDataInput(pimpl::DataInput &dataInput, pimpl::SerializerHolder &serializerHolder)
                    : dataInput(dataInput), serializerHolder(serializerHolder) {
            }

            void ObjectDataInput::readFully(std::vector<byte> &bytes) {
                dataInput.readFully(bytes);
            }

            int ObjectDataInput::skipBytes(int i) {
                return dataInput.skipBytes(i);
            }

            bool ObjectDataInput::readBoolean() {
                return dataInput.readBoolean();
            }

            byte ObjectDataInput::readByte() {
                return dataInput.readByte();
            }

            int16_t ObjectDataInput::readShort() {
                return dataInput.readShort();
            }

            char ObjectDataInput::readChar() {
                return dataInput.readChar();
            }

            int32_t ObjectDataInput::readInt() {
                return dataInput.readInt();
            }

            int64_t ObjectDataInput::readLong() {
                return dataInput.readLong();
            }

            float ObjectDataInput::readFloat() {
                return dataInput.readFloat();
            }

            double ObjectDataInput::readDouble() {
                return dataInput.readDouble();
            }

            std::unique_ptr<std::string> ObjectDataInput::readUTF() {
                return dataInput.readUTF();
            }

            pimpl::Data ObjectDataInput::readData() {
                return pimpl::Data(dataInput.readByteArray());
            }

            int ObjectDataInput::position() {
                return dataInput.position();
            }

            void ObjectDataInput::position(int newPos) {
                dataInput.position(newPos);
            }

            std::unique_ptr<std::vector<byte> > ObjectDataInput::readByteArray() {
                return dataInput.readByteArray();
            }

            std::unique_ptr<std::vector<bool> > ObjectDataInput::readBooleanArray() {
                return dataInput.readBooleanArray();
            }

            std::unique_ptr<std::vector<char> > ObjectDataInput::readCharArray() {
                return dataInput.readCharArray();
            }

            std::unique_ptr<std::vector<int32_t> > ObjectDataInput::readIntArray() {
                return dataInput.readIntArray();
            }

            std::unique_ptr<std::vector<int64_t> > ObjectDataInput::readLongArray() {
                return dataInput.readLongArray();
            }

            std::unique_ptr<std::vector<double> > ObjectDataInput::readDoubleArray() {
                return dataInput.readDoubleArray();
            }

            std::unique_ptr<std::vector<float> > ObjectDataInput::readFloatArray() {
                return dataInput.readFloatArray();
            }

            std::unique_ptr<std::vector<int16_t> > ObjectDataInput::readShortArray() {
                return dataInput.readShortArray();
            }

            std::unique_ptr<std::vector<std::string> > ObjectDataInput::readUTFArray() {
                return dataInput.readUTFArray();
            }

            std::unique_ptr<std::vector<std::string *> > ObjectDataInput::readUTFPointerArray() {
                return dataInput.readUTFPointerArray();
            }

            template<>
            std::vector<std::string> *ObjectDataInput::getBackwardCompatiblePointer(void *actualData,
                                                                                    const std::vector<std::string> *typePointer) const {
                std::unique_ptr<std::vector<std::string> > result(new std::vector<std::string>());
                typedef std::vector<std::string *> STRING_PONTER_ARRAY;
                std::vector<std::string *> *data = reinterpret_cast<std::vector<std::string *> *>(actualData);
                // it is guaranteed that the data will not be null
                for (STRING_PONTER_ARRAY::value_type value  : *data) {
                    if ((std::string *) NULL == value) {
                        result->push_back("");
                    } else {
                        result->push_back(*value);
                    }
                }
                return result.release();
            }

            template<>
            std::unique_ptr<HazelcastJsonValue>
            ObjectDataInput::readObjectInternal(int32_t typeId, const std::shared_ptr<SerializerBase> &serializer) {
                std::shared_ptr<StreamSerializer> streamSerializer = std::static_pointer_cast<StreamSerializer>(
                        serializer);

                return std::unique_ptr<HazelcastJsonValue>(
                        getBackwardCompatiblePointer<HazelcastJsonValue>(streamSerializer->read(*this),
                                                                         (HazelcastJsonValue *) NULL));
            }
        }
    }
}

//
//  Created by ihsan demir on 9/9/15.
//  Copyright (c) 2015 ihsan demir. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace serialization {
            FieldType::FieldType() : id(0) {
            }

            FieldType::FieldType(int type) : id((byte) type) {
            }

            FieldType::FieldType(FieldType const &rhs) : id(rhs.id) {
            }

            const byte FieldType::getId() const {
                return id;
            }

            FieldType &FieldType::operator=(FieldType const &rhs) {
                this->id = rhs.id;
                return (*this);
            }

            bool FieldType::operator==(FieldType const &rhs) const {
                if (id != rhs.id) return false;
                return true;
            }

            bool FieldType::operator!=(FieldType const &rhs) const {
                if (id == rhs.id) return false;
                return true;
            }

            std::ostream &operator<<(std::ostream &os, const FieldType &type) {
                os << "FieldType{id: " << type.id << "}";
                return os;
            }
        }
    }
}

//
//  Created by ihsan demir on 9/9/15.
//  Copyright (c) 2015 ihsan demir. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace serialization {
            SerializerBase::~SerializerBase() {
            }

            void SerializerBase::destroy() {
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            ObjectDataOutput::ObjectDataOutput(pimpl::DataOutput &dataOutput, pimpl::SerializerHolder *serializerHolder)
                    : dataOutput(&dataOutput), serializerHolder(serializerHolder), isEmpty(false) {

            }

            ObjectDataOutput::ObjectDataOutput()
                    : dataOutput(NULL), serializerHolder(NULL), isEmpty(true) {
            }

            std::unique_ptr<std::vector<byte> > ObjectDataOutput::toByteArray() {
                if (isEmpty)
                    return std::unique_ptr<std::vector<byte> >((std::vector<byte> *) NULL);
                return dataOutput->toByteArray();
            }

            void ObjectDataOutput::write(const std::vector<byte> &bytes) {
                if (isEmpty) return;
                dataOutput->write(bytes);
            }

            void ObjectDataOutput::writeBoolean(bool i) {
                if (isEmpty) return;
                dataOutput->writeBoolean(i);
            }

            void ObjectDataOutput::writeByte(int32_t i) {
                if (isEmpty) return;
                dataOutput->writeByte(i);
            }

            void ObjectDataOutput::writeBytes(const byte *bytes, size_t len) {
                if (isEmpty) return;
                dataOutput->writeBytes(bytes, len);
            }

            void ObjectDataOutput::writeShort(int32_t v) {
                if (isEmpty) return;
                dataOutput->writeShort(v);
            }

            void ObjectDataOutput::writeChar(int32_t i) {
                if (isEmpty) return;
                dataOutput->writeChar(i);
            }

            void ObjectDataOutput::writeInt(int32_t v) {
                if (isEmpty) return;
                dataOutput->writeInt(v);
            }

            void ObjectDataOutput::writeLong(int64_t l) {
                if (isEmpty) return;
                dataOutput->writeLong(l);
            }

            void ObjectDataOutput::writeFloat(float x) {
                if (isEmpty) return;
                dataOutput->writeFloat(x);
            }

            void ObjectDataOutput::writeDouble(double v) {
                if (isEmpty) return;
                dataOutput->writeDouble(v);
            }

            void ObjectDataOutput::writeUTF(const std::string *str) {
                if (isEmpty) return;

                if (NULL == str) {
                    writeInt(util::Bits::NULL_ARRAY);
                } else {
                    dataOutput->writeUTF(str);
                }
            }

            void ObjectDataOutput::writeByteArray(const std::vector<byte> *value) {
                if (isEmpty) return;
                dataOutput->writeByteArray(value);
            }

            void ObjectDataOutput::writeCharArray(const std::vector<char> *data) {
                if (isEmpty) return;
                dataOutput->writeCharArray(data);
            }

            void ObjectDataOutput::writeBooleanArray(const std::vector<bool> *data) {
                if (isEmpty) return;
                dataOutput->writeBooleanArray(data);
            }

            void ObjectDataOutput::writeShortArray(const std::vector<int16_t> *data) {
                if (isEmpty) return;
                dataOutput->writeShortArray(data);
            }

            void ObjectDataOutput::writeIntArray(const std::vector<int32_t> *data) {
                if (isEmpty) return;
                dataOutput->writeIntArray(data);
            }

            void ObjectDataOutput::writeLongArray(const std::vector<int64_t> *data) {
                if (isEmpty) return;
                dataOutput->writeLongArray(data);
            }

            void ObjectDataOutput::writeFloatArray(const std::vector<float> *data) {
                if (isEmpty) return;
                dataOutput->writeFloatArray(data);
            }

            void ObjectDataOutput::writeDoubleArray(const std::vector<double> *data) {
                if (isEmpty) return;
                dataOutput->writeDoubleArray(data);
            }

            void ObjectDataOutput::writeUTFArray(const std::vector<std::string *> *strings) {
                if (isEmpty) return;

                int32_t len = NULL != strings ? (int32_t) strings->size() : util::Bits::NULL_ARRAY;

                writeInt(len);

                if (len > 0) {
                    for (std::vector<std::string *>::const_iterator it = strings->begin(); it != strings->end(); ++it) {
                        writeUTF(*it);
                    }
                }
            }

            void ObjectDataOutput::writeData(const pimpl::Data *data) {
                if (NULL == data || 0 == data->dataSize()) {
                    writeInt(util::Bits::NULL_ARRAY);
                } else {
                    writeByteArray(&data->toByteArray());
                }

            }

            size_t ObjectDataOutput::position() {
                return dataOutput->position();
            }

            void ObjectDataOutput::position(size_t newPos) {
                dataOutput->position(newPos);
            }

            pimpl::DataOutput *ObjectDataOutput::getDataOutput() const {
                return dataOutput;
            }

            template<>
            void ObjectDataOutput::writeInternal(const std::vector<std::string> *object,
                                                 std::shared_ptr<StreamSerializer> &streamSerializer) {
                std::vector<std::string> *stringVector = const_cast<std::vector<std::string> *>(object);
                std::unique_ptr<std::vector<std::string *> > result(new std::vector<std::string *>());
                for (std::vector<std::string>::iterator it = stringVector->begin(); it != stringVector->end(); ++it) {
                    result->push_back(&(*it));
                }

                streamSerializer->write(*this, result.get());
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        HazelcastJsonValue::HazelcastJsonValue(const std::string &jsonString) : jsonString(jsonString) {
        }

        HazelcastJsonValue::~HazelcastJsonValue() {
        }

        const std::string &HazelcastJsonValue::toString() const {
            return jsonString;
        }

        bool HazelcastJsonValue::operator==(const HazelcastJsonValue &rhs) const {
            return jsonString == rhs.jsonString;
        }

        bool HazelcastJsonValue::operator!=(const HazelcastJsonValue &rhs) const {
            return !(rhs == *this);
        }

        std::ostream &operator<<(std::ostream &os, const HazelcastJsonValue &value) {
            os << "jsonString: " << value.jsonString;
            return os;
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableReader::PortableReader(pimpl::PortableContext &context, ObjectDataInput &input,
                                           std::shared_ptr<ClassDefinition> cd, bool isDefaultReader)
                    : isDefaultReader(isDefaultReader) {
                if (isDefaultReader) {
                    defaultPortableReader.reset(new pimpl::DefaultPortableReader(context, input, cd));
                } else {
                    morphingPortableReader.reset(new pimpl::MorphingPortableReader(context, input, cd));
                }
            }

            PortableReader::PortableReader(const PortableReader &reader)
                    : isDefaultReader(reader.isDefaultReader),
                      defaultPortableReader(reader.defaultPortableReader.release()),
                      morphingPortableReader(reader.morphingPortableReader.release()) {

            }

            PortableReader &PortableReader::operator=(const PortableReader &reader) {
                this->isDefaultReader = reader.isDefaultReader;
                this->defaultPortableReader.reset(reader.defaultPortableReader.release());
                this->morphingPortableReader.reset(reader.morphingPortableReader.release());
                return *this;
            }

            int32_t PortableReader::readInt(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readInt(fieldName);
                return morphingPortableReader->readInt(fieldName);
            }

            int64_t PortableReader::readLong(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readLong(fieldName);
                return morphingPortableReader->readLong(fieldName);
            }

            bool PortableReader::readBoolean(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readBoolean(fieldName);
                return morphingPortableReader->readBoolean(fieldName);
            }

            byte PortableReader::readByte(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readByte(fieldName);
                return morphingPortableReader->readByte(fieldName);
            }

            char PortableReader::readChar(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readChar(fieldName);
                return morphingPortableReader->readChar(fieldName);
            }

            double PortableReader::readDouble(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readDouble(fieldName);
                return morphingPortableReader->readDouble(fieldName);
            }

            float PortableReader::readFloat(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readFloat(fieldName);
                return morphingPortableReader->readFloat(fieldName);
            }

            int16_t PortableReader::readShort(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readShort(fieldName);
                return morphingPortableReader->readShort(fieldName);
            }

            std::unique_ptr<std::string> PortableReader::readUTF(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readUTF(fieldName);
                return morphingPortableReader->readUTF(fieldName);
            }

            std::unique_ptr<std::vector<byte> > PortableReader::readByteArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readByteArray(fieldName);
                return morphingPortableReader->readByteArray(fieldName);
            }


            std::unique_ptr<std::vector<bool> > PortableReader::readBooleanArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readBooleanArray(fieldName);
                return morphingPortableReader->readBooleanArray(fieldName);
            }

            std::unique_ptr<std::vector<char> > PortableReader::readCharArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readCharArray(fieldName);
                return morphingPortableReader->readCharArray(fieldName);
            }

            std::unique_ptr<std::vector<int32_t> > PortableReader::readIntArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readIntArray(fieldName);
                return morphingPortableReader->readIntArray(fieldName);
            }

            std::unique_ptr<std::vector<int64_t> > PortableReader::readLongArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readLongArray(fieldName);
                return morphingPortableReader->readLongArray(fieldName);
            }

            std::unique_ptr<std::vector<double> > PortableReader::readDoubleArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readDoubleArray(fieldName);
                return morphingPortableReader->readDoubleArray(fieldName);
            }

            std::unique_ptr<std::vector<float> > PortableReader::readFloatArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readFloatArray(fieldName);
                return morphingPortableReader->readFloatArray(fieldName);
            }

            std::unique_ptr<std::vector<int16_t> > PortableReader::readShortArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readShortArray(fieldName);
                return morphingPortableReader->readShortArray(fieldName);
            }

            ObjectDataInput &PortableReader::getRawDataInput() {
                if (isDefaultReader)
                    return defaultPortableReader->getRawDataInput();
                return morphingPortableReader->getRawDataInput();
            }

            void PortableReader::end() {
                if (isDefaultReader)
                    return defaultPortableReader->end();
                return morphingPortableReader->end();

            }
        }
    }
}
//
//  ClassDefinition.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

namespace hazelcast {
    namespace client {
        namespace serialization {
            ClassDefinition::ClassDefinition()
                    : factoryId(0), classId(0), version(-1), binary(new std::vector<byte>) {
            }

            ClassDefinition::ClassDefinition(int factoryId, int classId, int version)
                    : factoryId(factoryId), classId(classId), version(version), binary(new std::vector<byte>) {
            }

            void ClassDefinition::addFieldDef(FieldDefinition &fd) {
                fieldDefinitionsMap[fd.getName()] = fd;
            }

            const FieldDefinition &ClassDefinition::getField(const char *name) const {
                std::map<std::string, FieldDefinition>::const_iterator it;
                it = fieldDefinitionsMap.find(name);
                if (it != fieldDefinitionsMap.end()) {
                    return fieldDefinitionsMap.find(name)->second;
                }
                char msg[200];
                util::hz_snprintf(msg, 200, "Field (%s) does not exist", NULL != name ? name : "");
                throw exception::IllegalArgumentException("ClassDefinition::getField", msg);
            }

            bool ClassDefinition::hasField(const char *fieldName) const {
                return fieldDefinitionsMap.count(fieldName) != 0;
            }

            FieldType ClassDefinition::getFieldType(const char *fieldName) const {
                FieldDefinition const &fd = getField(fieldName);
                return fd.getType();
            }

            int ClassDefinition::getFieldCount() const {
                return (int) fieldDefinitionsMap.size();
            }


            int ClassDefinition::getFactoryId() const {
                return factoryId;
            }

            int ClassDefinition::getClassId() const {
                return classId;
            }

            int ClassDefinition::getVersion() const {
                return version;
            }

            void ClassDefinition::setVersionIfNotSet(int version) {
                if (getVersion() < 0) {
                    this->version = version;
                }
            }

            void ClassDefinition::writeData(pimpl::DataOutput &dataOutput) {
                dataOutput.writeInt(factoryId);
                dataOutput.writeInt(classId);
                dataOutput.writeInt(version);
                dataOutput.writeShort(fieldDefinitionsMap.size());
                for (std::map<std::string, FieldDefinition>::iterator it = fieldDefinitionsMap.begin();
                     it != fieldDefinitionsMap.end(); ++it) {
                    it->second.writeData(dataOutput);
                }
            }

            void ClassDefinition::readData(pimpl::DataInput &dataInput) {
                factoryId = dataInput.readInt();
                classId = dataInput.readInt();
                version = dataInput.readInt();
                int size = dataInput.readShort();
                for (int i = 0; i < size; i++) {
                    FieldDefinition fieldDefinition;
                    fieldDefinition.readData(dataInput);
                    addFieldDef(fieldDefinition);
                }
            }

            bool ClassDefinition::operator==(const ClassDefinition &rhs) const {
                return factoryId == rhs.factoryId &&
                       classId == rhs.classId &&
                       version == rhs.version &&
                       fieldDefinitionsMap == rhs.fieldDefinitionsMap;
            }

            bool ClassDefinition::operator!=(const ClassDefinition &rhs) const {
                return !(rhs == *this);
            }

            std::ostream &operator<<(std::ostream &os, const ClassDefinition &definition) {
                os << "ClassDefinition{" << "factoryId: " << definition.factoryId << " classId: " << definition.classId
                   << " version: "
                   << definition.version << " fieldDefinitions: {";

                for (std::map<std::string, FieldDefinition>::const_iterator it = definition.fieldDefinitionsMap.begin();
                     it != definition.fieldDefinitionsMap.end(); ++it) {
                    os << it->second;
                }
                os << "} }";
                return os;
            }
        }
    }
}

//
//  Data.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


using namespace hazelcast::util;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                //first 4 byte is partition hash code and next last 4 byte is type id
                unsigned int Data::PARTITION_HASH_OFFSET = 0;

                unsigned int Data::TYPE_OFFSET = Data::PARTITION_HASH_OFFSET + Bits::INT_SIZE_IN_BYTES;

                unsigned int Data::DATA_OFFSET = Data::TYPE_OFFSET + Bits::INT_SIZE_IN_BYTES;

                unsigned int Data::DATA_OVERHEAD = Data::DATA_OFFSET;

                Data::Data() : cachedHashValue(-1) {
                }

                Data::Data(std::unique_ptr<std::vector<byte> > &buffer) : Data::Data(std::move(buffer)) {}

                Data::Data(std::unique_ptr<std::vector<byte> > &&buffer) : data(std::move(buffer)),
                                                                           cachedHashValue(-1) {
                    if (data.get()) {
                        size_t size = data->size();
                        if (size > 0 && size < Data::DATA_OVERHEAD) {
                            throw (exception::ExceptionBuilder<exception::IllegalArgumentException>("Data::setBuffer")
                                    << "Provided buffer should be either empty or should contain more than "
                                    << Data::DATA_OVERHEAD << " bytes! Provided buffer size:" << size).build();
                        }

                        cachedHashValue = calculateHash();
                    }
                }

                size_t Data::dataSize() const {
                    return (size_t) std::max<int>((int) totalSize() - (int) Data::DATA_OVERHEAD, 0);
                }

                size_t Data::totalSize() const {
                    return data.get() != 0 ? data->size() : 0;
                }

                int Data::getPartitionHash() const {
                    return cachedHashValue;
                }

                bool Data::hasPartitionHash() const {
                    if (data.get() == NULL) {
                        return false;
                    }
                    return data->size() >= Data::DATA_OVERHEAD &&
                           *reinterpret_cast<int *>(&((*data)[PARTITION_HASH_OFFSET])) != 0;
                }

                std::vector<byte> &Data::toByteArray() const {
                    return *data;
                }

                int32_t Data::getType() const {
                    if (totalSize() == 0) {
                        return SerializationConstants::CONSTANT_TYPE_NULL;
                    }
                    return Bits::readIntB(*data, Data::TYPE_OFFSET);
                }

                int Data::hash() const {
                    return cachedHashValue;
                }

                int Data::calculateHash() const {
                    size_t size = dataSize();
                    if (size == 0) {
                        return 0;
                    }

                    if (hasPartitionHash()) {
                        return Bits::readIntB(*data, Data::PARTITION_HASH_OFFSET);
                    }

                    return MurmurHash3_x86_32((void *) &((*data)[Data::DATA_OFFSET]), (int) size);
                }

                bool Data::operator<(const Data &rhs) const {
                    return cachedHashValue < rhs.cachedHashValue;
                }
            }
        }
    }
}

bool std::less<std::shared_ptr<hazelcast::client::serialization::pimpl::Data>>::operator()(
        const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &lhs,
        const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &rhs) const noexcept {
    const hazelcast::client::serialization::pimpl::Data *leftPtr = lhs.get();
    const hazelcast::client::serialization::pimpl::Data *rightPtr = rhs.get();
    if (leftPtr == rightPtr) {
        return false;
    }

    if (leftPtr == NULL) {
        return true;
    }

    if (rightPtr == NULL) {
        return false;
    }

    return lhs->hash() < rhs->hash();
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                SerializerHolder::SerializerHolder(const std::shared_ptr<StreamSerializer> &globalSerializer)
                        : active(true), globalSerializer(globalSerializer) {
                }

                bool SerializerHolder::registerSerializer(const std::shared_ptr<StreamSerializer> &serializer) {
                    std::shared_ptr<SerializerBase> available = serializers.putIfAbsent(
                            serializer->getHazelcastTypeId(), serializer);
                    return available.get() == NULL;
                }

                std::shared_ptr<StreamSerializer> SerializerHolder::serializerFor(int typeId) {
                    std::shared_ptr<StreamSerializer> serializer = serializers.get(typeId);

                    if (serializer.get()) {
                        return serializer;
                    }

                    serializer = lookupGlobalSerializer(typeId);

                    if (!serializer.get()) {
                        if (active) {
                            std::ostringstream out;
                            out << "There is no suitable serializer for " << typeId;
                            throw exception::HazelcastSerializationException("SerializerHolder::registerSerializer",
                                                                             out.str());
                        }
                        throw exception::HazelcastClientNotActiveException("SerializerHolder::registerSerializer");
                    }
                    return serializer;
                }

                void SerializerHolder::dispose() {
                    active.store(false);

                    for (std::shared_ptr<StreamSerializer> serializer : serializers.values()) {
                        serializer->destroy();
                    }

                    serializers.clear();
                }

                std::shared_ptr<StreamSerializer> SerializerHolder::lookupGlobalSerializer(int typeId) {
                    if (!globalSerializer.get()) {
                        return std::shared_ptr<StreamSerializer>();
                    }

                    serializers.putIfAbsent(typeId, globalSerializer);
                    return globalSerializer;
                }

            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                DataInput::DataInput(const std::vector<byte> &buf) : buffer(buf), pos(0) {
                }

                DataInput::DataInput(const std::vector<byte> &buf, int offset)
                        : buffer(buf), pos(offset) {
                }

                DataInput::~DataInput() {
                }

                void DataInput::readFully(std::vector<byte> &bytes) {
                    size_t length = bytes.size();
                    checkAvailable(length);
                    memcpy(&(bytes[0]), &(buffer[pos]), length);
                    pos += length;
                }

                int DataInput::skipBytes(int i) {
                    checkAvailable(i);
                    pos += i;
                    return i;
                }

                bool DataInput::readBoolean() {
                    checkAvailable(1);
                    return readBooleanUnchecked();
                }

                bool DataInput::readBooleanUnchecked() { return readByteUnchecked() != 0; }

                byte DataInput::readByte() {
                    checkAvailable(1);
                    return readByteUnchecked();
                }

                byte DataInput::readByteUnchecked() { return buffer[pos++]; }

                int16_t DataInput::readShort() {
                    checkAvailable(util::Bits::SHORT_SIZE_IN_BYTES);
                    return readShortUnchecked();
                }

                int16_t DataInput::readShortUnchecked() {
                    int16_t result;
                    util::Bits::bigEndianToNative2(&buffer[pos], &result);
                    pos += util::Bits::SHORT_SIZE_IN_BYTES;
                    return result;
                }

                char DataInput::readChar() {
                    checkAvailable(util::Bits::CHAR_SIZE_IN_BYTES);
                    return readCharUnchecked();
                }

                char DataInput::readCharUnchecked() {
                    // skip the first byte
                    byte b = buffer[pos + 1];
                    pos += util::Bits::CHAR_SIZE_IN_BYTES;
                    return b;
                }

                int32_t DataInput::readInt() {
                    checkAvailable(util::Bits::INT_SIZE_IN_BYTES);
                    return readIntUnchecked();
                }

                int32_t DataInput::readIntUnchecked() {
                    int32_t result;
                    util::Bits::bigEndianToNative4(&buffer[pos], &result);
                    pos += util::Bits::INT_SIZE_IN_BYTES;
                    return result;
                }

                int64_t DataInput::readLong() {
                    checkAvailable(util::Bits::LONG_SIZE_IN_BYTES);
                    return readLongUnchecked();
                }

                int64_t DataInput::readLongUnchecked() {
                    int64_t result;
                    util::Bits::bigEndianToNative8(&buffer[pos], &result);
                    pos += util::Bits::LONG_SIZE_IN_BYTES;
                    return result;
                }

                float DataInput::readFloat() {
                    checkAvailable(util::Bits::FLOAT_SIZE_IN_BYTES);
                    return readFloatUnchecked();
                }

                float DataInput::readFloatUnchecked() {
                    union {
                        int32_t i;
                        float f;
                    } u;
                    u.i = readIntUnchecked();
                    return u.f;
                }

                double DataInput::readDouble() {
                    checkAvailable(util::Bits::DOUBLE_SIZE_IN_BYTES);
                    return readDoubleUnchecked();
                }

                double DataInput::readDoubleUnchecked() {
                    union {
                        double d;
                        int64_t l;
                    } u;
                    u.l = readLongUnchecked();
                    return u.d;
                }

                std::unique_ptr<std::string> DataInput::readUTF() {
                    int32_t charCount = readInt();
                    if (util::Bits::NULL_ARRAY == charCount) {
                        return std::unique_ptr<std::string>();
                    } else {
                        utfBuffer.clear();
                        utfBuffer.reserve((size_t) MAX_UTF_CHAR_SIZE * charCount);
                        byte b;
                        for (int i = 0; i < charCount; ++i) {
                            b = readByte();
                            util::UTFUtil::readUTF8Char(*this, b, utfBuffer);
                        }

                        return std::unique_ptr<std::string>(new std::string(utfBuffer.begin(), utfBuffer.end()));
                    }
                }

                int DataInput::position() {
                    return pos;
                }

                void DataInput::position(int position) {
                    if (position > pos) {
                        checkAvailable((size_t) (position - pos));
                    }
                    pos = position;
                }
                //private functions

                std::unique_ptr<std::vector<byte> > DataInput::readByteArray() {
                    return readArray<byte>();
                }

                std::unique_ptr<std::vector<bool> > DataInput::readBooleanArray() {
                    return readArray<bool>();
                }

                std::unique_ptr<std::vector<char> > DataInput::readCharArray() {
                    return readArray<char>();
                }

                std::unique_ptr<std::vector<int32_t> > DataInput::readIntArray() {
                    return readArray<int32_t>();
                }

                std::unique_ptr<std::vector<int64_t> > DataInput::readLongArray() {
                    return readArray<int64_t>();
                }

                std::unique_ptr<std::vector<double> > DataInput::readDoubleArray() {
                    return readArray<double>();
                }

                std::unique_ptr<std::vector<float> > DataInput::readFloatArray() {
                    return readArray<float>();
                }

                std::unique_ptr<std::vector<int16_t> > DataInput::readShortArray() {
                    return readArray<int16_t>();
                }

                std::unique_ptr<std::vector<std::string> > DataInput::readUTFArray() {
                    int32_t len = readInt();
                    if (util::Bits::NULL_ARRAY == len) {
                        return std::unique_ptr<std::vector<std::string> >();
                    }

                    std::unique_ptr<std::vector<std::string> > values(
                            new std::vector<std::string>());
                    for (int32_t i = 0; i < len; ++i) {
                        std::unique_ptr<std::string> value = readUTF();
                        // handle null pointer possibility
                        if ((std::string *) NULL == value.get()) {
                            values->push_back(std::string(""));
                        } else {
                            values->push_back(*value);
                        }
                    }
                    return values;
                }

                std::unique_ptr<std::vector<std::string *> > DataInput::readUTFPointerArray() {
                    int32_t len = readInt();
                    if (util::Bits::NULL_ARRAY == len) {
                        return std::unique_ptr<std::vector<std::string *> >();
                    }

                    std::unique_ptr<std::vector<std::string *> > values(
                            new std::vector<std::string *>());
                    try {
                        for (int32_t i = 0; i < len; ++i) {
                            values->push_back(readUTF().release());
                        }
                    } catch (exception::IException &) {
                        // clean resources to avoid any leaks
                        typedef std::vector<std::string *> STRING_ARRAY;
                        for (STRING_ARRAY::value_type value  : *values) {
                            delete value;
                        }
                        throw;
                    }
                    return values;
                }

                void DataInput::checkAvailable(size_t requestedLength) {
                    size_t available = buffer.size() - pos;

                    if (requestedLength > available) {
                        char msg[100];
                        util::hz_snprintf(msg, 100,
                                          "Not enough bytes in internal buffer. Available:%lu bytes but needed %lu bytes",
                                          (unsigned long) available, (unsigned long) requestedLength);
                        throw exception::IOException("DataInput::checkBoundary", msg);
                    }
                }

                template<>
                byte DataInput::read() {
                    return readByteUnchecked();
                }

                template<>
                char DataInput::read() {
                    return readCharUnchecked();
                }

                template<>
                bool DataInput::read() {
                    return readBooleanUnchecked();
                }

                template<>
                int16_t DataInput::read() {
                    return readShortUnchecked();
                }

                template<>
                int32_t DataInput::read() {
                    return readIntUnchecked();
                }

                template<>
                int64_t DataInput::read() {
                    return readLongUnchecked();
                }

                template<>
                float DataInput::read() {
                    return readFloatUnchecked();
                }

                template<>
                double DataInput::read() {
                    return readDoubleUnchecked();
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                PortableReaderBase::PortableReaderBase(PortableContext &portableContext, ObjectDataInput &input,
                                                       std::shared_ptr<ClassDefinition> cd)
                        : cd(cd), dataInput(input), serializerHolder(portableContext.getSerializerHolder()),
                          raw(false) {
                    int fieldCount;
                    try {
                        // final position after portable is read
                        finalPosition = input.readInt();
                        // field count
                        fieldCount = input.readInt();
                    } catch (exception::IException &e) {
                        throw exception::HazelcastSerializationException(
                                "[DefaultPortableReader::DefaultPortableReader]", e.what());
                    }
                    if (fieldCount != cd->getFieldCount()) {
                        char msg[50];
                        util::hz_snprintf(msg, 50, "Field count[%d] in stream does not match %d", fieldCount,
                                          cd->getFieldCount());
                        throw new exception::IllegalStateException("[DefaultPortableReader::DefaultPortableReader]",
                                                                   msg);
                    }
                    this->offset = input.position();
                }

                PortableReaderBase::~PortableReaderBase() {

                }

                int32_t PortableReaderBase::readInt(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_INT);
                    return dataInput.readInt();
                }

                int64_t PortableReaderBase::readLong(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG);
                    return dataInput.readLong();
                }

                bool PortableReaderBase::readBoolean(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN);
                    return dataInput.readBoolean();
                }

                hazelcast::byte PortableReaderBase::readByte(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE);
                    return dataInput.readByte();
                }

                char PortableReaderBase::readChar(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR);
                    return dataInput.readChar();
                }

                double PortableReaderBase::readDouble(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE);
                    return dataInput.readDouble();
                }

                float PortableReaderBase::readFloat(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT);
                    return dataInput.readFloat();
                }

                int16_t PortableReaderBase::readShort(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT);
                    return dataInput.readShort();
                }

                std::unique_ptr<std::string> PortableReaderBase::readUTF(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_UTF);
                    return dataInput.readUTF();
                }

                std::unique_ptr<std::vector<byte> > PortableReaderBase::readByteArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                    return dataInput.readByteArray();
                }

                std::unique_ptr<std::vector<bool> > PortableReaderBase::readBooleanArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN_ARRAY);
                    return dataInput.readBooleanArray();
                }

                std::unique_ptr<std::vector<char> > PortableReaderBase::readCharArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                    return dataInput.readCharArray();
                }

                std::unique_ptr<std::vector<int32_t> > PortableReaderBase::readIntArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_INT_ARRAY);
                    return dataInput.readIntArray();
                }

                std::unique_ptr<std::vector<int64_t> > PortableReaderBase::readLongArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG_ARRAY);
                    return dataInput.readLongArray();
                }

                std::unique_ptr<std::vector<double> > PortableReaderBase::readDoubleArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                    return dataInput.readDoubleArray();
                }

                std::unique_ptr<std::vector<float> > PortableReaderBase::readFloatArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                    return dataInput.readFloatArray();
                }

                std::unique_ptr<std::vector<int16_t> > PortableReaderBase::readShortArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                    return dataInput.readShortArray();
                }

                void PortableReaderBase::setPosition(char const *fieldName, FieldType const &fieldType) {
                    dataInput.position(readPosition(fieldName, fieldType));
                }

                int PortableReaderBase::readPosition(const char *fieldName, FieldType const &fieldType) {
                    if (raw) {
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                         "Cannot read Portable fields after getRawDataInput() is called!");
                    }
                    if (!cd->hasField(fieldName)) {
                        // TODO: if no field def found, java client reads nested position:
                        // readNestedPosition(fieldName, type);
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                         "Don't have a field named " +
                                                                         std::string(fieldName));
                    }

                    if (cd->getFieldType(fieldName) != fieldType) {
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                         "Field type did not matched for " +
                                                                         std::string(fieldName));
                    }

                    dataInput.position(offset + cd->getField(fieldName).getIndex() * util::Bits::INT_SIZE_IN_BYTES);
                    int32_t pos = dataInput.readInt();

                    dataInput.position(pos);
                    int16_t len = dataInput.readShort();

                    // name + len + type
                    return pos + util::Bits::SHORT_SIZE_IN_BYTES + len + 1;
                }

                hazelcast::client::serialization::ObjectDataInput &PortableReaderBase::getRawDataInput() {
                    if (!raw) {
                        dataInput.position(offset + cd->getFieldCount() * util::Bits::INT_SIZE_IN_BYTES);
                        int32_t pos = dataInput.readInt();
                        dataInput.position(pos);
                    }
                    raw = true;
                    return dataInput;
                }

                void PortableReaderBase::end() {
                    dataInput.position(finalPosition);
                }

                void
                PortableReaderBase::checkFactoryAndClass(FieldDefinition fd, int32_t factoryId, int32_t classId) const {
                    if (factoryId != fd.getFactoryId()) {
                        char msg[100];
                        util::hz_snprintf(msg, 100, "Invalid factoryId! Expected: %d, Current: %d", fd.getFactoryId(),
                                          factoryId);
                        throw exception::HazelcastSerializationException("DefaultPortableReader::checkFactoryAndClass ",
                                                                         std::string(msg));
                    }
                    if (classId != fd.getClassId()) {
                        char msg[100];
                        util::hz_snprintf(msg, 100, "Invalid classId! Expected: %d, Current: %d", fd.getClassId(),
                                          classId);
                        throw exception::HazelcastSerializationException("DefaultPortableReader::checkFactoryAndClass ",
                                                                         std::string(msg));
                    }
                }

            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                int32_t IntegerSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_INTEGER;
                }

                void IntegerSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeInt(*(static_cast<const int32_t *>(object)));
                }

                void *IntegerSerializer::read(ObjectDataInput &in) {
                    return new int32_t(in.readInt());
                }

                int32_t ByteSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BYTE;
                }

                void ByteSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeByte(*(static_cast<const byte *>(object)));
                }

                void *ByteSerializer::read(ObjectDataInput &in) {
                    return new byte(in.readByte());
                }

                int32_t BooleanSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
                }

                void BooleanSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeBoolean(*(static_cast<const bool *>(object)));
                }

                void *BooleanSerializer::read(ObjectDataInput &in) {
                    return new bool(in.readBoolean());
                }

                int32_t CharSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_CHAR;
                }

                void CharSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeChar(*(static_cast<const char *>(object)));
                }

                void *CharSerializer::read(ObjectDataInput &in) {
                    return new char(in.readChar());
                }

                int32_t ShortSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_SHORT;
                }

                void ShortSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeShort(*(static_cast<const int16_t *>(object)));
                }

                void *ShortSerializer::read(ObjectDataInput &in) {
                    return new int16_t(in.readShort());
                }

                int32_t LongSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_LONG;
                }

                void LongSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeLong(*(static_cast<const int64_t *>(object)));
                }

                void *LongSerializer::read(ObjectDataInput &in) {
                    return new int64_t(in.readLong());;
                }

                int32_t FloatSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_FLOAT;
                }

                void FloatSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeFloat(*(static_cast<const float *>(object)));
                }

                void *FloatSerializer::read(ObjectDataInput &in) {
                    return new float(in.readFloat());
                }


                int32_t DoubleSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DOUBLE;
                }

                void DoubleSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeDouble(*(static_cast<const double *>(object)));
                }

                void *DoubleSerializer::read(ObjectDataInput &in) {
                    return new double(in.readDouble());
                }

                int32_t StringSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_STRING;
                }

                void StringSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTF(static_cast<const std::string *>(object));
                }

                void *StringSerializer::read(ObjectDataInput &in) {
                    return in.readUTF().release();
                }

                int32_t NullSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_NULL;
                }

                void *NullSerializer::read(ObjectDataInput &in) {
                    return NULL;
                }

                void NullSerializer::write(ObjectDataOutput &out, const void *object) {
                }

                int32_t TheByteArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
                }

                void TheByteArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeByteArray(static_cast<const std::vector<byte> *>(object));
                }

                void *TheByteArraySerializer::read(ObjectDataInput &in) {
                    return in.readByteArray().release();
                }

                int32_t BooleanArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY;
                }

                void BooleanArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeBooleanArray(static_cast<const std::vector<bool> *>(object));
                }

                void *BooleanArraySerializer::read(ObjectDataInput &in) {
                    return in.readBooleanArray().release();
                }

                int32_t CharArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
                }

                void CharArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeCharArray(static_cast<const std::vector<char> *>(object));
                }

                void *CharArraySerializer::read(ObjectDataInput &in) {
                    return in.readCharArray().release();
                }

                int32_t ShortArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
                }

                void ShortArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeShortArray(static_cast<const std::vector<int16_t> *>(object));
                }

                void *ShortArraySerializer::read(ObjectDataInput &in) {
                    return in.readShortArray().release();
                }

                int32_t IntegerArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
                }

                void IntegerArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeIntArray(static_cast<const std::vector<int32_t> *>(object));
                }

                void *IntegerArraySerializer::read(ObjectDataInput &in) {
                    return in.readIntArray().release();
                }

                int32_t LongArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
                }

                void LongArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeLongArray(static_cast<const std::vector<int64_t> *>(object));
                }

                void *LongArraySerializer::read(ObjectDataInput &in) {
                    return in.readLongArray().release();
                }

                int32_t FloatArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
                }

                void FloatArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeFloatArray(static_cast<const std::vector<float> *>(object));
                }

                void *FloatArraySerializer::read(ObjectDataInput &in) {
                    return in.readFloatArray().release();
                }

                int32_t DoubleArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
                }

                void DoubleArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeDoubleArray(static_cast<const std::vector<double> *>(object));
                }

                void *DoubleArraySerializer::read(ObjectDataInput &in) {
                    return in.readDoubleArray().release();
                }

                int32_t StringArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
                }

                void StringArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTFArray(static_cast<const std::vector<std::string *> *>(object));
                }

                void *StringArraySerializer::read(ObjectDataInput &in) {
                    return in.readUTFPointerArray().release();
                }

                int32_t HazelcastJsonValueSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::JAVASCRIPT_JSON_SERIALIZATION_TYPE;
                }

                void HazelcastJsonValueSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTF(&(static_cast<const HazelcastJsonValue *>(object)->toString()));
                }

                void *HazelcastJsonValueSerializer::read(ObjectDataInput &in) {
                    return new HazelcastJsonValue(*in.readUTF());
                }
            }
        }
    }
}
//
//  MorphingPortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                MorphingPortableReader::MorphingPortableReader(PortableContext &portableContext, ObjectDataInput &input,
                                                               std::shared_ptr<ClassDefinition> cd)
                        : PortableReaderBase(portableContext, input, cd) {
                }

                int32_t MorphingPortableReader::readInt(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                int64_t MorphingPortableReader::readLong(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_LONG) {
                        return PortableReaderBase::readLong(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                bool MorphingPortableReader::readBoolean(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return false;
                    }
                    return PortableReaderBase::readBoolean(fieldName);
                }

                byte MorphingPortableReader::readByte(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    return PortableReaderBase::readByte(fieldName);
                }

                char MorphingPortableReader::readChar(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }

                    return PortableReaderBase::readChar(fieldName);
                }

                double MorphingPortableReader::readDouble(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0.0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_FLOAT) {
                        return PortableReaderBase::readFloat(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_DOUBLE) {
                        return PortableReaderBase::readDouble(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_LONG) {
                        return PortableReaderBase::readLong(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                float MorphingPortableReader::readFloat(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0.0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_FLOAT) {
                        return PortableReaderBase::readFloat(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_INT) {
                        return (float) PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return (float) PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return (float) PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return (float) PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                int16_t MorphingPortableReader::readShort(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                std::unique_ptr<std::string> MorphingPortableReader::readUTF(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::string>(new std::string(""));
                    }
                    return PortableReaderBase::readUTF(fieldName);
                }

                std::unique_ptr<std::vector<byte> > MorphingPortableReader::readByteArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<byte> >(new std::vector<byte>(1, 0));
                    }
                    return PortableReaderBase::readByteArray(fieldName);
                }

                std::unique_ptr<std::vector<char> > MorphingPortableReader::readCharArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<char> >(new std::vector<char>(1, 0));
                    }
                    return PortableReaderBase::readCharArray(fieldName);
                }

                std::unique_ptr<std::vector<int32_t> > MorphingPortableReader::readIntArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<int32_t> >(new std::vector<int32_t>(1, 0));
                    }
                    return PortableReaderBase::readIntArray(fieldName);
                }

                std::unique_ptr<std::vector<int64_t> > MorphingPortableReader::readLongArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<int64_t> >(new std::vector<int64_t>(1, 0));
                    }
                    return PortableReaderBase::readLongArray(fieldName);
                }

                std::unique_ptr<std::vector<double> > MorphingPortableReader::readDoubleArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<double> >(new std::vector<double>(1, 0));
                    }
                    return PortableReaderBase::readDoubleArray(fieldName);
                }

                std::unique_ptr<std::vector<float> > MorphingPortableReader::readFloatArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<float> >(new std::vector<float>(1, 0));
                    }
                    return PortableReaderBase::readFloatArray(fieldName);
                }

                std::unique_ptr<std::vector<int16_t> > MorphingPortableReader::readShortArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<int16_t> >(new std::vector<int16_t>(1, 0));
                    }
                    return PortableReaderBase::readShortArray(fieldName);
                }


            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                int PortableVersionHelper::getVersion(const Portable *portable, int defaultVersion) {
                    int version = defaultVersion;
                    if (const VersionedPortable *versionedPortable = dynamic_cast<const VersionedPortable *>(portable)) {
                        version = versionedPortable->getClassVersion();
                        if (version < 0) {
                            throw exception::IllegalArgumentException("PortableVersionHelper:getVersion",
                                                                      "Version cannot be negative!");
                        }
                    }
                    return version;
                }
            }
        }
    }
}
//
//  PortableContext.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

using namespace hazelcast::util;
using namespace hazelcast::client::serialization;
using namespace std;



namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                PortableContext::PortableContext(const SerializationConfig &serializationConf) :
                        serializationConfig(serializationConf),
                        serializerHolder(serializationConf.getGlobalSerializer()) {
                }

                int PortableContext::getClassVersion(int factoryId, int classId) {
                    return getClassDefinitionContext(factoryId).getClassVersion(classId);
                }

                void PortableContext::setClassVersion(int factoryId, int classId, int version) {
                    getClassDefinitionContext(factoryId).setClassVersion(classId, version);
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::lookupClassDefinition(int factoryId, int classId, int version) {
                    return getClassDefinitionContext(factoryId).lookup(classId, version);
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::readClassDefinition(ObjectDataInput &in, int factoryId, int classId, int version) {
                    bool shouldRegister = true;
                    ClassDefinitionBuilder builder(factoryId, classId, version);

                    // final position after portable is read
                    in.readInt();

                    // field count
                    int fieldCount = in.readInt();
                    int offset = in.position();
                    for (int i = 0; i < fieldCount; i++) {
                        in.position(offset + i * Bits::INT_SIZE_IN_BYTES);
                        int pos = in.readInt();
                        in.position(pos);

                        short len = in.readShort();
                        vector<byte> chars(len);
                        in.readFully(chars);
                        chars.push_back('\0');

                        FieldType type(in.readByte());
                        std::string name((char *) &(chars[0]));
                        int fieldFactoryId = 0;
                        int fieldClassId = 0;
                        int fieldVersion = version;
                        if (type == FieldTypes::TYPE_PORTABLE) {
                            // is null
                            if (in.readBoolean()) {
                                shouldRegister = false;
                            }
                            fieldFactoryId = in.readInt();
                            fieldClassId = in.readInt();

                            // TODO: what if there's a null inner Portable field
                            if (shouldRegister) {
                                fieldVersion = in.readInt();
                                readClassDefinition(in, fieldFactoryId, fieldClassId, fieldVersion);
                            }
                        } else if (type == FieldTypes::TYPE_PORTABLE_ARRAY) {
                            int k = in.readInt();
                            if (k > 0) {
                                fieldFactoryId = in.readInt();
                                fieldClassId = in.readInt();

                                int p = in.readInt();
                                in.position(p);

                                // TODO: what if there's a null inner Portable field
                                fieldVersion = in.readInt();
                                readClassDefinition(in, fieldFactoryId, fieldClassId, fieldVersion);
                            } else {
                                shouldRegister = false;
                            }

                        }
                        FieldDefinition fieldDef(i, name, type, fieldFactoryId, fieldClassId, fieldVersion);
                        builder.addField(fieldDef);
                    }
                    std::shared_ptr<ClassDefinition> classDefinition = builder.build();
                    if (shouldRegister) {
                        classDefinition = registerClassDefinition(classDefinition);
                    }
                    return classDefinition;
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::registerClassDefinition(std::shared_ptr<ClassDefinition> cd) {
                    return getClassDefinitionContext(cd->getFactoryId()).registerClassDefinition(cd);
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::lookupOrRegisterClassDefinition(const Portable &portable) {
                    int portableVersion = PortableVersionHelper::getVersion(&portable,
                                                                            serializationConfig.getPortableVersion());
                    std::shared_ptr<ClassDefinition> cd = lookupClassDefinition(portable.getFactoryId(),
                                                                                portable.getClassId(), portableVersion);
                    if (cd.get() == NULL) {
                        ClassDefinitionBuilder classDefinitionBuilder(portable.getFactoryId(), portable.getClassId(),
                                                                      portableVersion);
                        ClassDefinitionWriter cdw(*this, classDefinitionBuilder);
                        PortableWriter portableWriter(&cdw);
                        portable.writePortable(portableWriter);
                        cd = cdw.registerAndGet();
                    }
                    return cd;
                }

                int PortableContext::getVersion() {
                    return serializationConfig.getPortableVersion();
                }

                SerializerHolder &PortableContext::getSerializerHolder() {
                    return serializerHolder;
                }

                ClassDefinitionContext &PortableContext::getClassDefinitionContext(int factoryId) {
                    std::shared_ptr<ClassDefinitionContext> value = classDefContextMap.get(factoryId);
                    if (value == NULL) {
                        value = std::shared_ptr<ClassDefinitionContext>(new ClassDefinitionContext(factoryId, this));
                        std::shared_ptr<ClassDefinitionContext> current = classDefContextMap.putIfAbsent(factoryId,
                                                                                                         value);
                        if (current != NULL) {
                            value = current;
                        }
                    }
                    return *value;
                }

                const SerializationConfig &PortableContext::getSerializationConfig() const {
                    return serializationConfig;
                }

            }
        }
    }
}



//
//  SerializationService.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                ObjectType::ObjectType() : typeId(0), factoryId(-1), classId(-1) {}

                std::ostream &operator<<(std::ostream &os, const ObjectType &type) {
                    os << "typeId: " << type.typeId << " factoryId: " << type.factoryId << " classId: "
                       << type.classId;
                    return os;
                }

                SerializationService::SerializationService(const SerializationConfig &serializationConfig)
                        : portableContext(serializationConfig),
                          serializationConfig(serializationConfig) {
                    registerConstantSerializers();

                    std::vector<std::shared_ptr<SerializerBase> > const &serializers = serializationConfig.getSerializers();
                    for (std::vector<std::shared_ptr<SerializerBase> >::const_iterator it = serializers.begin();
                         it < serializers.end(); ++it) {
                        registerSerializer(std::static_pointer_cast<StreamSerializer>(*it));
                    }
                }

                SerializerHolder &SerializationService::getSerializerHolder() {
                    return portableContext.getSerializerHolder();
                }

                bool SerializationService::registerSerializer(std::shared_ptr<StreamSerializer> serializer) {
                    return getSerializerHolder().registerSerializer(serializer);
                }

                bool SerializationService::isNullData(const Data &data) {
                    return data.dataSize() == 0 && data.getType() == SerializationConstants::CONSTANT_TYPE_NULL;
                }

                const byte SerializationService::getVersion() const {
                    return 1;
                }

                ObjectType SerializationService::getObjectType(const Data *data) {
                    ObjectType type;

                    if (NULL == data) {
                        return type;
                    }

                    type.typeId = data->getType();

                    // Constant 4 is Data::TYPE_OFFSET. Windows DLL export does not
                    // let usage of static member.
                    DataInput dataInput(data->toByteArray(), 4);

                    ObjectDataInput objectDataInput(dataInput, getSerializerHolder());

                    if (SerializationConstants::CONSTANT_TYPE_DATA == type.typeId ||
                        SerializationConstants::CONSTANT_TYPE_PORTABLE == type.typeId) {
                        int32_t objectTypeId = objectDataInput.readInt();
                        assert(type.typeId == objectTypeId);

                        if (SerializationConstants::CONSTANT_TYPE_DATA == type.typeId) {
                            bool identified = objectDataInput.readBoolean();
                            if (!identified) {
                                throw exception::HazelcastSerializationException("SerializationService::getObjectType",
                                                                                 " DataSerializable is not identified");
                            }
                        }

                        type.factoryId = objectDataInput.readInt();
                        type.classId = objectDataInput.readInt();
                    }

                    return type;
                }

                void SerializationService::registerConstantSerializers() {
                    registerSerializer(std::shared_ptr<StreamSerializer>(new NullSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new DataSerializer(serializationConfig)));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new PortableSerializer(portableContext)));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new HazelcastJsonValueSerializer()));
                    //primitives and String
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::ByteSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::BooleanSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::CharSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::ShortSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::IntegerSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::LongSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::FloatSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::DoubleSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::StringSerializer));
                    //Arrays of primitives and String
                    registerSerializer(std::shared_ptr<StreamSerializer>(new TheByteArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new BooleanArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new CharArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new ShortArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new IntegerArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new LongArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new FloatArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new DoubleArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new StringArraySerializer()));
                }

                void SerializationService::dispose() {
                    getSerializerHolder().dispose();
                }

                template<>
                Data SerializationService::toData(const TypedData *object) {
                    if (!object) {
                        return Data();
                    }

                    const std::shared_ptr<Data> data = object->getData();
                    if ((Data *) NULL == data.get()) {
                        return Data();
                    }

                    return Data(*data);
                }

            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                size_t const DataOutput::DEFAULT_SIZE = 4 * 1024;

                DataOutput::DataOutput()
                        : outputStream(new std::vector<byte>()) {
                    outputStream->reserve(DEFAULT_SIZE);
                }


                DataOutput::~DataOutput() {
                }

                DataOutput::DataOutput(DataOutput const &rhs) {
                    //private
                }

                DataOutput &DataOutput::operator=(DataOutput const &rhs) {
                    //private
                    return *this;
                }

                std::unique_ptr<std::vector<byte> > DataOutput::toByteArray() {
                    std::unique_ptr<std::vector<byte> > byteArrayPtr(new std::vector<byte>(*outputStream));
                    return byteArrayPtr;
                }

                void DataOutput::write(const std::vector<byte> &bytes) {
                    outputStream->insert(outputStream->end(), bytes.begin(), bytes.end());
                }

                void DataOutput::writeBoolean(bool i) {
                    writeByte((byte) i);
                }

                void DataOutput::writeByte(int index, int32_t i) {
                    (*outputStream)[index] = byte(0xff & i);
                }

                void DataOutput::writeByte(int32_t i) {
                    outputStream->push_back(byte(0xff & i));
                }

                void DataOutput::writeShort(int32_t v) {
                    int16_t value = (int16_t) v;
                    int16_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian2(&value, target);
                    outputStream->insert(outputStream->end(), target, target + util::Bits::SHORT_SIZE_IN_BYTES);
                }

                void DataOutput::writeChar(int32_t i) {
                    writeByte((byte) (i >> 8));
                    writeByte((byte) i);
                }

                void DataOutput::writeInt(int32_t v) {
                    int32_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian4(&v, target);
                    outputStream->insert(outputStream->end(), target, target + util::Bits::INT_SIZE_IN_BYTES);
                }

                void DataOutput::writeLong(int64_t l) {
                    int64_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian8(&l, target);
                    outputStream->insert(outputStream->end(), target, target + util::Bits::LONG_SIZE_IN_BYTES);
                }

                void DataOutput::writeFloat(float x) {
                    union {
                        float f;
                        int32_t i;
                    } u;
                    u.f = x;
                    writeInt(u.i);
                }

                void DataOutput::writeDouble(double v) {
                    union {
                        double d;
                        int64_t l;
                    } u;
                    u.d = v;
                    writeLong(u.l);
                }

                void DataOutput::writeUTF(const std::string *str) {
                    int32_t len = util::Bits::NULL_ARRAY;
                    if (str) {
                        len = util::UTFUtil::isValidUTF8(*str);
                        if (len < 0) {
                            throw (exception::ExceptionBuilder<exception::UTFDataFormatException>(
                                    "DataOutput::writeUTF")
                                    << "String \"" << (*str) << "\" is not UTF-8 formatted !!!").build();
                        }
                    }

                    writeInt(len);
                    if (len > 0) {
                        outputStream->insert(outputStream->end(), str->begin(), str->end());
                    }
                }

                void DataOutput::writeInt(int index, int32_t v) {
                    int32_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian4(&v, &result);
                    (*outputStream)[index++] = *(target++);
                    (*outputStream)[index++] = *(target++);
                    (*outputStream)[index++] = *(target++);
                    (*outputStream)[index] = *target;
                }

                void DataOutput::writeBytes(const byte *bytes, size_t len) {
                    outputStream->insert(outputStream->end(), bytes, bytes + len);
                }

                void DataOutput::writeByteArray(const std::vector<byte> *data) {
                    writeArray<byte>(data);
                }

                void DataOutput::writeCharArray(const std::vector<char> *data) {
                    writeArray<char>(data);
                }

                void DataOutput::writeBooleanArray(const std::vector<bool> *data) {
                    writeArray<bool>(data);
                }

                void DataOutput::writeShortArray(const std::vector<int16_t> *data) {
                    writeArray<int16_t>(data);
                }

                void DataOutput::writeIntArray(const std::vector<int32_t> *data) {
                    writeArray<int32_t>(data);
                }

                void DataOutput::writeLongArray(const std::vector<int64_t> *data) {
                    writeArray<int64_t>(data);
                }

                void DataOutput::writeFloatArray(const std::vector<float> *data) {
                    writeArray<float>(data);
                }

                void DataOutput::writeDoubleArray(const std::vector<double> *data) {
                    writeArray<double>(data);
                }

                void DataOutput::writeUTFArray(const std::vector<std::string> *data) {
                    writeArray<std::string>(data);
                }

                void DataOutput::writeZeroBytes(int numberOfBytes) {
                    for (int k = 0; k < numberOfBytes; k++) {
                        writeByte(0);
                    }
                }

                size_t DataOutput::position() {
                    return outputStream->size();
                }

                void DataOutput::position(size_t newPos) {
                    if (outputStream->size() < newPos)
                        outputStream->resize(newPos, 0);
                }

                int DataOutput::getUTF8CharCount(const std::string &str) {
                    int size = 0;
                    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
                        // Any additional byte for an UTF character has a bit mask of 10xxxxxx
                        size += (*it & 0xC0) != 0x80;
                    }

                    return size;
                }

                template<>
                void DataOutput::write(const byte &value) {
                    writeByte(value);
                }

                template<>
                void DataOutput::write(const char &value) {
                    writeChar(value);
                }

                template<>
                void DataOutput::write(const bool &value) {
                    writeBoolean(value);
                }

                template<>
                void DataOutput::write(const int16_t &value) {
                    writeShort(value);
                }

                template<>
                void DataOutput::write(const int32_t &value) {
                    writeInt(value);
                }

                template<>
                void DataOutput::write(const int64_t &value) {
                    writeLong(value);
                }

                template<>
                void DataOutput::write(const float &value) {
                    writeFloat(value);
                }

                template<>
                void DataOutput::write(const double &value) {
                    writeDouble(value);
                }

                template<>
                void DataOutput::write(const std::string &value) {
                    writeUTF(&value);
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DataSerializer::DataSerializer(const SerializationConfig &serializationConfig)
                        : serializationConfig(serializationConfig) {
                }

                DataSerializer::~DataSerializer() {
                }

                void DataSerializer::checkIfIdentifiedDataSerializable(ObjectDataInput &in) const {
                    bool identified = in.readBoolean();
                    if (!identified) {
                        throw exception::HazelcastSerializationException("void DataSerializer::read",
                                                                         " DataSerializable is not identified");
                    }
                }

                int32_t DataSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DATA;
                }

                void DataSerializer::write(ObjectDataOutput &out, const IdentifiedDataSerializable *dataSerializable) {
                    out.writeBoolean(true);
                    out.writeInt(dataSerializable->getFactoryId());
                    out.writeInt(dataSerializable->getClassId());
                    dataSerializable->writeData(out);
                }

                void DataSerializer::write(ObjectDataOutput &out, const void *object) {
                    // should not be called
                    assert(0);
                }

                void *DataSerializer::read(ObjectDataInput &in) {
                    // should not be called
                    assert(0);
                    return NULL;
                }

                int32_t DataSerializer::readInt(ObjectDataInput &in) const {
                    return in.readInt();
                }
            }
        }
    }
}


//
//  PortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DefaultPortableReader::DefaultPortableReader(PortableContext &portableContext,
                                                             ObjectDataInput &input,
                                                             std::shared_ptr<ClassDefinition> cd)
                        : PortableReaderBase(portableContext, input, cd) {
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                ClassDefinitionContext::ClassDefinitionContext(int factoryId, PortableContext *portableContext)
                        : factoryId(factoryId), portableContext(portableContext) {
                }

                int ClassDefinitionContext::getClassVersion(int classId) {
                    std::shared_ptr<int> version = currentClassVersions.get(classId);
                    return version != NULL ? *version : -1;
                }

                void ClassDefinitionContext::setClassVersion(int classId, int version) {
                    std::shared_ptr<int> current = currentClassVersions.putIfAbsent(classId, std::shared_ptr<int>(
                            new int(version)));
                    if (current != NULL && *current != version) {
                        std::stringstream error;
                        error << "Class-id: " << classId << " is already registered!";
                        throw exception::IllegalArgumentException("ClassDefinitionContext::setClassVersion",
                                                                  error.str());
                    }
                }

                std::shared_ptr<ClassDefinition> ClassDefinitionContext::lookup(int classId, int version) {
                    long long key = combineToLong(classId, version);
                    return versionedDefinitions.get(key);

                }

                std::shared_ptr<ClassDefinition>
                ClassDefinitionContext::registerClassDefinition(std::shared_ptr<ClassDefinition> cd) {
                    if (cd.get() == NULL) {
                        return std::shared_ptr<ClassDefinition>();
                    }
                    if (cd->getFactoryId() != factoryId) {
                        throw (exception::ExceptionBuilder<exception::HazelcastSerializationException>(
                                "ClassDefinitionContext::registerClassDefinition") << "Invalid factory-id! "
                                                                                   << factoryId << " -> "
                                                                                   << cd).build();
                    }

                    cd->setVersionIfNotSet(portableContext->getVersion());

                    long long versionedClassId = combineToLong(cd->getClassId(), cd->getVersion());
                    std::shared_ptr<ClassDefinition> currentCd = versionedDefinitions.putIfAbsent(versionedClassId, cd);
                    if (currentCd.get() == NULL) {
                        return cd;
                    }

                    if (currentCd.get() != cd.get() && *currentCd != *cd) {
                        throw (exception::ExceptionBuilder<exception::HazelcastSerializationException>(
                                "ClassDefinitionContext::registerClassDefinition")
                                << "Incompatible class-definitions with same class-id: " << *cd << " VS "
                                << *currentCd).build();
                    }

                    return currentCd;
                }

                int64_t ClassDefinitionContext::combineToLong(int x, int y) const {
                    return ((int64_t) x) << 32 | (((int64_t) y) & 0xFFFFFFFL);
                }

            }
        }
    }
}

//
//  PortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DefaultPortableWriter::DefaultPortableWriter(PortableContext &portableContext,
                                                             std::shared_ptr<ClassDefinition> cd,
                                                             ObjectDataOutput &output)
                        : raw(false), serializerHolder(portableContext.getSerializerHolder()),
                          dataOutput(*output.getDataOutput()), objectDataOutput(output), begin(dataOutput.position()),
                          cd(cd) {
                    // room for final offset
                    dataOutput.writeZeroBytes(4);

                    objectDataOutput.writeInt(cd->getFieldCount());

                    offset = dataOutput.position();
                    // one additional for raw data
                    int fieldIndexesLength = (cd->getFieldCount() + 1) * util::Bits::INT_SIZE_IN_BYTES;
                    dataOutput.writeZeroBytes(fieldIndexesLength);
                }

                void DefaultPortableWriter::writeInt(const char *fieldName, int32_t value) {
                    setPosition(fieldName, FieldTypes::TYPE_INT);
                    dataOutput.writeInt(value);
                }

                void DefaultPortableWriter::writeLong(const char *fieldName, int64_t value) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG);
                    dataOutput.writeLong(value);
                }

                void DefaultPortableWriter::writeBoolean(const char *fieldName, bool value) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN);
                    dataOutput.writeBoolean(value);
                }

                void DefaultPortableWriter::writeByte(const char *fieldName, byte value) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE);
                    dataOutput.writeByte(value);
                }

                void DefaultPortableWriter::writeChar(const char *fieldName, int32_t value) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR);
                    dataOutput.writeChar(value);
                }

                void DefaultPortableWriter::writeDouble(const char *fieldName, double value) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE);
                    dataOutput.writeDouble(value);
                }

                void DefaultPortableWriter::writeFloat(const char *fieldName, float value) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT);
                    dataOutput.writeFloat(value);
                }

                void DefaultPortableWriter::writeShort(const char *fieldName, int32_t value) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT);
                    dataOutput.writeShort(value);
                }

                void DefaultPortableWriter::writeUTF(const char *fieldName, const std::string *value) {
                    setPosition(fieldName, FieldTypes::TYPE_UTF);
                    dataOutput.writeUTF(value);
                }

                void DefaultPortableWriter::writeByteArray(const char *fieldName, const std::vector<byte> *bytes) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                    dataOutput.writeByteArray(bytes);
                }

                void DefaultPortableWriter::writeBooleanArray(const char *fieldName, const std::vector<bool> *bytes) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN_ARRAY);
                    dataOutput.writeBooleanArray(bytes);
                }

                void DefaultPortableWriter::writeCharArray(const char *fieldName, const std::vector<char> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                    dataOutput.writeCharArray(data);
                }

                void DefaultPortableWriter::writeShortArray(const char *fieldName, const std::vector<int16_t> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                    dataOutput.writeShortArray(data);
                }

                void DefaultPortableWriter::writeIntArray(const char *fieldName, const std::vector<int32_t> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_INT_ARRAY);
                    dataOutput.writeIntArray(data);
                }

                void DefaultPortableWriter::writeLongArray(const char *fieldName, const std::vector<int64_t> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG_ARRAY);
                    dataOutput.writeLongArray(data);
                }

                void DefaultPortableWriter::writeFloatArray(const char *fieldName, const std::vector<float> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                    dataOutput.writeFloatArray(data);
                }

                void DefaultPortableWriter::writeDoubleArray(const char *fieldName, const std::vector<double> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                    dataOutput.writeDoubleArray(data);
                }

                FieldDefinition const &DefaultPortableWriter::setPosition(const char *fieldName, FieldType fieldType) {
                    if (raw) {
                        throw exception::HazelcastSerializationException("PortableWriter::setPosition",
                                                                         "Cannot write Portable fields after getRawDataOutput() is called!");
                    }

                    try {
                        FieldDefinition const &fd = cd->getField(fieldName);

                        if (writtenFields.count(fieldName) != 0) {
                            throw exception::HazelcastSerializationException("PortableWriter::setPosition",
                                                                             "Field '" + std::string(fieldName) +
                                                                             "' has already been written!");
                        }

                        writtenFields.insert(fieldName);
                        size_t pos = dataOutput.position();
                        int32_t index = fd.getIndex();
                        dataOutput.writeInt((int32_t) (offset + index * util::Bits::INT_SIZE_IN_BYTES), (int32_t) pos);
                        size_t nameLen = strlen(fieldName);
                        dataOutput.writeShort(nameLen);
                        dataOutput.writeBytes((byte *) fieldName, nameLen);
                        dataOutput.writeByte(fieldType.getId());

                        return fd;

                    } catch (exception::IllegalArgumentException &iae) {
                        std::stringstream error;
                        error << "HazelcastSerializationException( Invalid field name: '" << fieldName;
                        error << "' for ClassDefinition {class id: " << util::IOUtil::to_string(cd->getClassId());
                        error << ", factoryId:" + util::IOUtil::to_string(cd->getFactoryId());
                        error << ", version: " << util::IOUtil::to_string(cd->getVersion()) << "}. Error:";
                        error << iae.what();

                        throw exception::HazelcastSerializationException("PortableWriter::setPosition", error.str());
                    }

                }


                ObjectDataOutput &DefaultPortableWriter::getRawDataOutput() {
                    if (!raw) {
                        size_t pos = dataOutput.position();
                        int32_t index = cd->getFieldCount(); // last index
                        dataOutput.writeInt((int32_t) (offset + index * util::Bits::INT_SIZE_IN_BYTES), (int32_t) pos);
                    }
                    raw = true;
                    return objectDataOutput;
                }

                void DefaultPortableWriter::end() {
                    dataOutput.writeInt((int32_t) begin, (int32_t) dataOutput.position()); // write final offset
                }

                void DefaultPortableWriter::write(const Portable &p) {
                    std::shared_ptr<PortableSerializer> serializer = std::static_pointer_cast<PortableSerializer>(
                            serializerHolder.serializerFor(SerializationConstants::CONSTANT_TYPE_PORTABLE));
                    serializer->writeInternal(objectDataOutput, &p);
                }


                void
                DefaultPortableWriter::checkPortableAttributes(const FieldDefinition &fd, const Portable &portable) {
                    if (fd.getFactoryId() != portable.getFactoryId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                                     << " Expected factory-id: " << fd.getFactoryId() << ", Actual factory-id: "
                                     << portable.getFactoryId();
                        throw exception::HazelcastSerializationException(
                                "DefaultPortableWriter::::checkPortableAttributes", errorMessage.str());
                    }
                    if (fd.getClassId() != portable.getClassId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                                     << "Expected class-id: " << fd.getClassId() << ", Actual class-id: "
                                     << portable.getClassId();
                        throw exception::HazelcastSerializationException(
                                "DefaultPortableWriter::::checkPortableAttributes", errorMessage.str());
                    }
                }
            }
        }
    }
}
//
//  ClassDefinitionWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                ClassDefinitionWriter::ClassDefinitionWriter(PortableContext &portableContext,
                                                             ClassDefinitionBuilder &builder)
                        : builder(builder), context(portableContext) {
                }

                std::shared_ptr<ClassDefinition> ClassDefinitionWriter::registerAndGet() {
                    std::shared_ptr<ClassDefinition> cd = builder.build();
                    return context.registerClassDefinition(cd);
                }

                void ClassDefinitionWriter::writeInt(const char *fieldName, int32_t value) {
                    builder.addIntField(fieldName);
                }

                void ClassDefinitionWriter::writeLong(const char *fieldName, int64_t value) {
                    builder.addLongField(fieldName);
                }

                void ClassDefinitionWriter::writeBoolean(const char *fieldName, bool value) {

                    builder.addBooleanField(fieldName);
                }

                void ClassDefinitionWriter::writeByte(const char *fieldName, byte value) {
                    builder.addByteField(fieldName);
                }

                void ClassDefinitionWriter::writeChar(const char *fieldName, int32_t value) {
                    builder.addCharField(fieldName);
                }

                void ClassDefinitionWriter::writeDouble(const char *fieldName, double value) {

                    builder.addDoubleField(fieldName);
                }

                void ClassDefinitionWriter::writeFloat(const char *fieldName, float value) {
                    builder.addFloatField(fieldName);
                }

                void ClassDefinitionWriter::writeShort(const char *fieldName, int16_t value) {
                    builder.addShortField(fieldName);
                }

                void ClassDefinitionWriter::writeUTF(const char *fieldName, const std::string *value) {

                    builder.addUTFField(fieldName);
                }

                void ClassDefinitionWriter::writeByteArray(const char *fieldName, const std::vector<byte> *values) {
                    builder.addByteArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeBooleanArray(const char *fieldName, const std::vector<bool> *values) {
                    builder.addBooleanArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeCharArray(const char *fieldName, const std::vector<char> *values) {
                    builder.addCharArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeIntArray(const char *fieldName, const std::vector<int32_t> *values) {
                    builder.addIntArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeLongArray(const char *fieldName, const std::vector<int64_t> *values) {
                    builder.addLongArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeDoubleArray(const char *fieldName, const std::vector<double> *values) {
                    builder.addDoubleArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeFloatArray(const char *fieldName, const std::vector<float> *values) {
                    builder.addFloatArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeShortArray(const char *fieldName, const std::vector<int16_t> *values) {
                    builder.addShortArrayField(fieldName);
                }

                ObjectDataOutput &ClassDefinitionWriter::getRawDataOutput() {
                    return emptyDataOutput;
                }

                void ClassDefinitionWriter::end() {

                }

                std::shared_ptr<ClassDefinition> ClassDefinitionWriter::createNestedClassDef(const Portable &portable) {
                    int version = pimpl::PortableVersionHelper::getVersion(&portable, context.getVersion());
                    ClassDefinitionBuilder definitionBuilder(portable.getFactoryId(), portable.getClassId(), version);

                    ClassDefinitionWriter nestedWriter(context, definitionBuilder);
                    PortableWriter portableWriter(&nestedWriter);
                    portable.writePortable(portableWriter);
                    return context.registerClassDefinition(definitionBuilder.build());
                }
            }
        }
    }
}
//
//  PortableSerializer.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                PortableSerializer::PortableSerializer(PortableContext &portableContext)
                        : context(portableContext) {
                }

                void
                PortableSerializer::read(ObjectDataInput &in, Portable &portable, int32_t factoryId, int32_t classId) {
                    int version = in.readInt();

                    int portableVersion = findPortableVersion(factoryId, classId, portable);

                    PortableReader reader = createReader(in, factoryId, classId, version, portableVersion);
                    portable.readPortable(reader);
                    reader.end();
                }

                PortableReader
                PortableSerializer::createReader(ObjectDataInput &input, int factoryId, int classId, int version,
                                                 int portableVersion) const {

                    int effectiveVersion = version;
                    if (version < 0) {
                        effectiveVersion = context.getVersion();
                    }

                    std::shared_ptr<ClassDefinition> cd = context.lookupClassDefinition(factoryId, classId,
                                                                                        effectiveVersion);
                    if (cd == NULL) {
                        int begin = input.position();
                        cd = context.readClassDefinition(input, factoryId, classId, effectiveVersion);
                        input.position(begin);
                    }

                    if (portableVersion == effectiveVersion) {
                        PortableReader reader(context, input, cd, true);
                        return reader;
                    } else {
                        PortableReader reader(context, input, cd, false);
                        return reader;
                    }
                }

                int
                PortableSerializer::findPortableVersion(int factoryId, int classId, const Portable &portable) const {
                    int currentVersion = context.getClassVersion(factoryId, classId);
                    if (currentVersion < 0) {
                        currentVersion = PortableVersionHelper::getVersion(&portable, context.getVersion());
                        if (currentVersion > 0) {
                            context.setClassVersion(factoryId, classId, currentVersion);
                        }
                    }
                    return currentVersion;
                }

                std::unique_ptr<Portable>
                PortableSerializer::createNewPortableInstance(int32_t factoryId, int32_t classId) {
                    const std::map<int32_t, std::shared_ptr<PortableFactory> > &portableFactories =
                            context.getSerializationConfig().getPortableFactories();
                    std::map<int, std::shared_ptr<hazelcast::client::serialization::PortableFactory> >::const_iterator factoryIt =
                            portableFactories.find(factoryId);

                    if (portableFactories.end() == factoryIt) {
                        return std::unique_ptr<Portable>();
                    }

                    return factoryIt->second->create(classId);
                }

                int32_t PortableSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_PORTABLE;
                }

                void PortableSerializer::write(ObjectDataOutput &out, const void *object) {
                    const Portable *p = static_cast<const Portable *>(object);

                    if (p->getClassId() == 0) {
                        throw exception::IllegalArgumentException("Portable class ID cannot be zero!");
                    }

                    out.writeInt(p->getFactoryId());
                    out.writeInt(p->getClassId());

                    writeInternal(out, p);
                }

                void PortableSerializer::writeInternal(ObjectDataOutput &out, const Portable *p) const {
                    std::shared_ptr<ClassDefinition> cd = context.lookupOrRegisterClassDefinition(*p);
                    out.writeInt(cd->getVersion());

                    DefaultPortableWriter dpw(context, cd, out);
                    PortableWriter portableWriter(&dpw);
                    p->writePortable(portableWriter);
                    portableWriter.end();
                }

                void *PortableSerializer::read(ObjectDataInput &in) {
                    // should not be called
                    assert(0);
                    return NULL;
                }

                int32_t PortableSerializer::readInt(ObjectDataInput &in) const {
                    return in.readInt();
                }
            }
        }
    }
}


//


namespace hazelcast {
    namespace client {
        namespace serialization {
            int32_t getHazelcastTypeId(const Portable *portable) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE;
            }

            int32_t getHazelcastTypeId(const IdentifiedDataSerializable *identifiedDataSerializable) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_DATA;
            }

            int32_t getHazelcastTypeId(const char *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_CHAR;
            }

            int32_t getHazelcastTypeId(const bool *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BOOLEAN;
            }

            int32_t getHazelcastTypeId(const byte *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BYTE;
            }

            int32_t getHazelcastTypeId(const int16_t *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_SHORT;
            }

            int32_t getHazelcastTypeId(const int32_t *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_INTEGER;
            }

            int32_t getHazelcastTypeId(const int64_t *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_LONG;
            }

            int32_t getHazelcastTypeId(const float *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_FLOAT;
            }

            int32_t getHazelcastTypeId(const double *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_DOUBLE;
            }

            int32_t getHazelcastTypeId(const std::string *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_STRING;
            }

            int32_t getHazelcastTypeId(const std::vector<char> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<bool> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<byte> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<int16_t> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<int32_t> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<int64_t> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<float> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<double> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<std::string *> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<std::string> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
            }

            int32_t getHazelcastTypeId(const void *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_VOID_POINTER;
            }

            int32_t getHazelcastTypeId(const HazelcastJsonValue *object) {
                return pimpl::SerializationConstants::JAVASCRIPT_JSON_SERIALIZATION_TYPE;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            PortableWriter::PortableWriter(pimpl::DefaultPortableWriter *defaultPortableWriter)
                    : defaultPortableWriter(defaultPortableWriter), classDefinitionWriter(NULL), isDefaultWriter(true) {

            }

            PortableWriter::PortableWriter(pimpl::ClassDefinitionWriter *classDefinitionWriter)
                    : defaultPortableWriter(NULL), classDefinitionWriter(classDefinitionWriter),
                      isDefaultWriter(false) {

            }

            void PortableWriter::writeInt(const char *fieldName, int32_t value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeInt(fieldName, value);
                return classDefinitionWriter->writeInt(fieldName, value);
            }

            void PortableWriter::writeLong(const char *fieldName, int64_t value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeLong(fieldName, value);
                return classDefinitionWriter->writeLong(fieldName, value);
            }

            void PortableWriter::writeBoolean(const char *fieldName, bool value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeBoolean(fieldName, value);
                return classDefinitionWriter->writeBoolean(fieldName, value);
            }

            void PortableWriter::writeByte(const char *fieldName, byte value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeByte(fieldName, value);
                return classDefinitionWriter->writeByte(fieldName, value);
            }

            void PortableWriter::writeChar(const char *fieldName, int32_t value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeChar(fieldName, value);
                return classDefinitionWriter->writeChar(fieldName, value);
            }

            void PortableWriter::writeDouble(const char *fieldName, double value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeDouble(fieldName, value);
                return classDefinitionWriter->writeDouble(fieldName, value);
            }

            void PortableWriter::writeFloat(const char *fieldName, float value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeFloat(fieldName, value);
                return classDefinitionWriter->writeFloat(fieldName, value);
            }

            void PortableWriter::writeShort(const char *fieldName, int16_t value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeShort(fieldName, value);
                return classDefinitionWriter->writeShort(fieldName, value);
            }

            void PortableWriter::writeUTF(const char *fieldName, const std::string *str) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeUTF(fieldName, str);
                return classDefinitionWriter->writeUTF(fieldName, str);
            }

            void PortableWriter::writeByteArray(const char *fieldName, const std::vector<byte> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeByteArray(fieldName, data);
                return classDefinitionWriter->writeByteArray(fieldName, data);
            }

            void PortableWriter::writeBooleanArray(const char *fieldName, const std::vector<bool> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeBooleanArray(fieldName, data);
                return classDefinitionWriter->writeBooleanArray(fieldName, data);
            }

            void PortableWriter::writeCharArray(const char *fieldName, const std::vector<char> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeCharArray(fieldName, data);
                return classDefinitionWriter->writeCharArray(fieldName, data);
            }

            void PortableWriter::writeShortArray(const char *fieldName, const std::vector<int16_t> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeShortArray(fieldName, data);
                return classDefinitionWriter->writeShortArray(fieldName, data);
            }

            void PortableWriter::writeIntArray(const char *fieldName, const std::vector<int32_t> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeIntArray(fieldName, data);
                return classDefinitionWriter->writeIntArray(fieldName, data);
            }

            void PortableWriter::writeLongArray(const char *fieldName, const std::vector<int64_t> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeLongArray(fieldName, data);
                return classDefinitionWriter->writeLongArray(fieldName, data);
            }

            void PortableWriter::writeFloatArray(const char *fieldName, const std::vector<float> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeFloatArray(fieldName, data);
                return classDefinitionWriter->writeFloatArray(fieldName, data);
            }

            void PortableWriter::writeDoubleArray(const char *fieldName, const std::vector<double> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeDoubleArray(fieldName, data);
                return classDefinitionWriter->writeDoubleArray(fieldName, data);
            }

            void PortableWriter::end() {
                if (isDefaultWriter)
                    return defaultPortableWriter->end();
                return classDefinitionWriter->end();
            }

            ObjectDataOutput &PortableWriter::getRawDataOutput() {
                if (isDefaultWriter)
                    return defaultPortableWriter->getRawDataOutput();
                return classDefinitionWriter->getRawDataOutput();
            }

        }
    }
}


namespace hazelcast {
    namespace client {
        SocketInterceptor::~SocketInterceptor() {
        }
    }
}


namespace hazelcast {
    namespace client {

        MemberAttributeEvent::MemberAttributeEvent(Cluster &cluster, const Member &member,
                                                   MemberAttributeEvent::MemberAttributeOperationType operationType,
                                                   const std::string &key, const std::string &value)
                : MembershipEvent(cluster, member, MembershipEvent::MEMBER_ATTRIBUTE_CHANGED, std::vector<Member>()),
                  operationType(operationType), key(key), value(value) {
        }

        MemberAttributeEvent::MemberAttributeOperationType MemberAttributeEvent::getOperationType() const {
            return operationType;
        }

        const std::string &MemberAttributeEvent::getKey() const {
            return key;
        }

        const std::string &MemberAttributeEvent::getValue() const {
            return value;
        }
    }
}

namespace hazelcast {
    namespace client {
        LoadBalancer::~LoadBalancer() {
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    namespace record {
                        NearCacheDataRecord::NearCacheDataRecord(
                                const std::shared_ptr<serialization::pimpl::Data> &dataValue,
                                int64_t createTime, int64_t expiryTime)
                                : AbstractNearCacheRecord<serialization::pimpl::Data>(dataValue,
                                                                                      createTime,
                                                                                      expiryTime) {
                        }
                    }
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                NearCacheManager::NearCacheManager(serialization::pimpl::SerializationService &ss,
                                                   util::ILogger &logger)
                        : serializationService(ss), logger(logger) {
                }

                bool NearCacheManager::clearNearCache(const std::string &name) {
                    std::shared_ptr<BaseNearCache> nearCache = nearCacheMap.get(name);
                    if (nearCache.get() != NULL) {
                        nearCache->clear();
                    }
                    return nearCache.get() != NULL;
                }

                void NearCacheManager::clearAllNearCaches() {
                    std::vector<std::shared_ptr<BaseNearCache> > caches = nearCacheMap.values();
                    for (std::vector<std::shared_ptr<BaseNearCache> >::iterator it = caches.begin();
                         it != caches.end(); ++it) {
                        (*it)->clear();
                    }
                }

                bool NearCacheManager::destroyNearCache(const std::string &name) {
                    std::shared_ptr<BaseNearCache> nearCache = nearCacheMap.remove(name);
                    if (nearCache.get() != NULL) {
                        nearCache->destroy();
                    }
                    return nearCache.get() != NULL;
                }

                void NearCacheManager::destroyAllNearCaches() {
                    std::vector<std::shared_ptr<BaseNearCache> > caches = nearCacheMap.values();
                    for (std::vector<std::shared_ptr<BaseNearCache> >::iterator it = caches.begin();
                         it != caches.end(); ++it) {
                        (*it)->destroy();
                    }
                }

                std::vector<std::shared_ptr<BaseNearCache> > NearCacheManager::listAllNearCaches() {
                    return nearCacheMap.values();
                }

            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    KeyStateMarkerImpl::KeyStateMarkerImpl(int count) : markCount(count),
                                                                        marks(new std::atomic<int32_t>[count]) {
                        for (int i = 0; i < count; ++i) {
                            marks[i] = 0;
                        }
                    }

                    KeyStateMarkerImpl::~KeyStateMarkerImpl() {
                        delete[] marks;
                    }

                    bool KeyStateMarkerImpl::tryMark(const serialization::pimpl::Data &key) {
                        return casState(key, UNMARKED, MARKED);
                    }

                    bool KeyStateMarkerImpl::tryUnmark(const serialization::pimpl::Data &key) {
                        return casState(key, MARKED, UNMARKED);
                    }

                    bool KeyStateMarkerImpl::tryRemove(const serialization::pimpl::Data &key) {
                        return casState(key, MARKED, REMOVED);
                    }

                    void KeyStateMarkerImpl::forceUnmark(const serialization::pimpl::Data &key) {
                        int slot = getSlot(key);
                        marks[slot] = UNMARKED;
                    }

                    void KeyStateMarkerImpl::init() {
                        for (int i = 0; i < markCount; ++i) {
                            marks[i] = UNMARKED;
                        }
                    }

                    bool
                    KeyStateMarkerImpl::casState(const serialization::pimpl::Data &key, STATE expect, STATE update) {
                        int slot = getSlot(key);
                        int expected = expect;
                        return marks[slot].compare_exchange_strong(expected, update);
                    }

                    int KeyStateMarkerImpl::getSlot(const serialization::pimpl::Data &key) {
                        return util::HashUtil::hashToIndex(key.getPartitionHash(), markCount);
                    }
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace internal {
            namespace partition {
                namespace strategy {

                    std::string StringPartitioningStrategy::getBaseName(const std::string &name) {
                        size_t indexOf = name.find('@');
                        if (indexOf == std::string::npos) {
                            return name;
                        }
                        return name.substr(0, indexOf);
                    }

                    std::string StringPartitioningStrategy::getPartitionKey(const std::string &key) {
                        size_t firstIndexOf = key.find('@');
                        if (firstIndexOf == std::string::npos) {
                            return key;
                        } else {
                            return key.substr(firstIndexOf + 1);
                        }
                    }
                }
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace internal {
            namespace socket {
                SocketFactory::SocketFactory(spi::ClientContext &clientContext) : clientContext(clientContext) {
                }

                bool SocketFactory::start() {
#ifdef HZ_BUILD_WITH_SSL
                    const client::config::SSLConfig &sslConfig = clientContext.getClientConfig().getNetworkConfig().getSSLConfig();
                    if (sslConfig.isEnabled()) {
                        sslContext = std::unique_ptr<asio::ssl::context>(new asio::ssl::context(
                                (asio::ssl::context_base::method) sslConfig.getProtocol()));

                        const std::vector<std::string> &verifyFiles = sslConfig.getVerifyFiles();
                        bool success = true;
                        util::ILogger &logger = clientContext.getLogger();
                        for (std::vector<std::string>::const_iterator it = verifyFiles.begin(); it != verifyFiles.end();
                             ++it) {
                            asio::error_code ec;
                            sslContext->load_verify_file(*it, ec);
                            if (ec) {
                                logger.warning(
                                        std::string("SocketFactory::start: Failed to load CA "
                                                    "verify file at ") + *it + " "
                                        + ec.message());
                                success = false;
                            }
                        }

                        if (!success) {
                            sslContext.reset();
                            logger.warning("SocketFactory::start: Failed to load one or more "
                                           "configured CA verify files (PEM files). Please "
                                           "correct the files and retry.");
                            return false;
                        }

                        // set cipher list if the list is set
                        const std::string &cipherList = sslConfig.getCipherList();
                        if (!cipherList.empty()) {
                            if (!SSL_CTX_set_cipher_list(sslContext->native_handle(), cipherList.c_str())) {
                                logger.warning(
                                        std::string("SocketFactory::start: Could not load any "
                                                    "of the ciphers in the config provided "
                                                    "ciphers:") + cipherList);
                                return false;
                            }
                        }

                    }
#else
                    (void) clientContext;
#endif

                    return true;
                }

                std::unique_ptr<Socket> SocketFactory::create(const Address &address) {
#ifdef HZ_BUILD_WITH_SSL
                    if (sslContext.get()) {
                        return std::unique_ptr<Socket>(new internal::socket::SSLSocket(ioService, *sslContext, address,
                                                                                       clientContext.getClientConfig().getNetworkConfig().getSocketOptions()));
                    }
#endif

                    return std::unique_ptr<Socket>(new internal::socket::TcpSocket(address,
                                                                                   &clientContext.getClientConfig().getNetworkConfig().getSocketOptions()));
                }
            }
        }
    }
}


#ifdef HZ_BUILD_WITH_SSL

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#endif




namespace hazelcast {
    namespace client {
        namespace internal {
            namespace socket {
                SSLSocket::SSLSocket(asio::io_service &ioService, asio::ssl::context &context,
                                     const client::Address &address, client::config::SocketOptions &socketOptions)
                        : remoteEndpoint(address), socket(ioService, context), ioService(ioService),
                          deadline(ioService), socketId(-1), socketOptions(socketOptions), isOpen(true) {
                }

                SSLSocket::~SSLSocket() {
                    close();
                }

                void SSLSocket::handleConnect(const asio::error_code &error) {
                    errorCode = error;
                }

                void SSLSocket::checkDeadline(const asio::error_code &ec) {
                    // The timer may return an error, e.g. operation_aborted when we cancel it. would_block is OK,
                    // since we set it at the start of the connection.
                    if (ec && ec != asio::error::would_block) {
                        return;
                    }

                    // Check whether the deadline has passed. We compare the deadline against
                    // the current time since a new asynchronous operation may have moved the
                    // deadline before this actor had a chance to run.
                    if (deadline.expires_at() <= std::chrono::system_clock::now()) {
                        // The deadline has passed. The socket is closed so that any outstanding
                        // asynchronous operations are cancelled. This allows the blocked
                        // connect(), read_line() or write_line() functions to return.
                        asio::error_code ignored_ec;
                        socket.lowest_layer().close(ignored_ec);

                        return;
                    }

                    // Put the actor back to sleep. _1 is for passing the error_code to the method.
                    deadline.async_wait(std::bind(&SSLSocket::checkDeadline, this, std::placeholders::_1));
                }

                int SSLSocket::connect(int timeoutInMillis) {
                    try {
                        asio::ip::tcp::resolver resolver(ioService);
                        std::ostringstream out;
                        out << remoteEndpoint.getPort();
                        asio::ip::tcp::resolver::query query(remoteEndpoint.getHost(), out.str());
                        asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

                        deadline.expires_from_now(std::chrono::milliseconds(timeoutInMillis));

                        // Set up the variable that receives the result of the asynchronous
                        // operation. The error code is set to would_block to signal that the
                        // operation is incomplete. Asio guarantees that its asynchronous
                        // operations will never fail with would_block, so any other value in
                        // errorCode indicates completion.
                        errorCode = asio::error::would_block;

                        checkDeadline(errorCode);

                        // Start the asynchronous operation itself. a callback will update the ec variable when the
                        // operation completes.
                        asio::async_connect(socket.lowest_layer(), iterator,
                                            std::bind(&SSLSocket::handleConnect, this, std::placeholders::_1));

                        // Block until the asynchronous operation has completed.
                        asio::error_code ioRunErrorCode;

                        // the restart is needed for the other connection attempts to work since the ioservice goes
                        // into the stopped state following the loop
                        ioService.restart();
                        do {
                            ioService.run_one(ioRunErrorCode);
                        } while (!ioRunErrorCode && (errorCode == asio::error::would_block));

                        // cancel the deadline timer
                        deadline.cancel();

                        // Cancel async connect operation if it is still in operation
                        socket.lowest_layer().cancel();

                        if (ioRunErrorCode) {
                            return ioRunErrorCode.value();
                        }

                        if (errorCode) {
                            return errorCode.value();
                        }

                        // Determine whether a connection was successfully established. The
                        // deadline actor may have had a chance to run and close our socket, even
                        // though the connect operation notionally succeeded. Therefore we must
                        // check whether the socket is still open before deciding if we succeeded
                        // or failed.
                        if (!socket.lowest_layer().is_open()) {
                            return asio::error::operation_aborted;
                        }

                        socket.handshake(asio::ssl::stream<asio::ip::tcp::socket>::client);

                        setSocketOptions();

                        setBlocking(false);
                        socketId = socket.lowest_layer().native_handle();
                    } catch (asio::system_error &e) {
                        return e.code().value();
                    }

                    return 0;
                }

                void SSLSocket::setBlocking(bool blocking) {
                    socket.lowest_layer().non_blocking(!blocking);
                }

                std::vector<SSLSocket::CipherInfo> SSLSocket::getCiphers() {
                    STACK_OF(SSL_CIPHER) *ciphers = SSL_get_ciphers(socket.native_handle());
                    std::vector<CipherInfo> supportedCiphers;
                    for (int i = 0; i < sk_SSL_CIPHER_num(ciphers); ++i) {
                        struct SSLSocket::CipherInfo info;
                        SSL_CIPHER *cipher = const_cast<SSL_CIPHER *>(sk_SSL_CIPHER_value(ciphers, i));
                        info.name = SSL_CIPHER_get_name(cipher);
                        info.numberOfBits = SSL_CIPHER_get_bits(cipher, 0);
                        info.version = SSL_CIPHER_get_version(cipher);
                        char descBuf[256];
                        info.description = SSL_CIPHER_description(cipher, descBuf, 256);
                        supportedCiphers.push_back(info);
                    }
                    return supportedCiphers;
                }

                int SSLSocket::send(const void *buffer, int len, int flag) {
                    size_t size = 0;
                    asio::error_code ec;

                    if (flag == MSG_WAITALL) {
                        size = asio::write(socket, asio::buffer(buffer, (size_t) len),
                                           asio::transfer_exactly((size_t) len), ec);
                    } else {
                        size = socket.write_some(asio::buffer(buffer, (size_t) len), ec);
                    }

                    return handleError("SSLSocket::send", size, ec);
                }

                int SSLSocket::receive(void *buffer, int len, int flag) {
                    asio::error_code ec;
                    size_t size = 0;

                    ReadHandler readHandler(size, ec);
                    asio::error_code ioRunErrorCode;
                    ioService.restart();
                    if (flag == MSG_WAITALL) {
                        asio::async_read(socket, asio::buffer(buffer, (size_t) len),
                                         asio::transfer_exactly((size_t) len), readHandler);
                        do {
                            ioService.run_one(ioRunErrorCode);
                            handleError("SSLSocket::receive", size, ec);
                        } while (!ioRunErrorCode && readHandler.getNumRead() < (size_t) len);

                        return (int) readHandler.getNumRead();
                    } else {
                        size = asio::read(socket, asio::buffer(buffer, (size_t) len),
                                          asio::transfer_exactly((size_t) len), ec);
                    }

                    return handleError("SSLSocket::receive", size, ec);
                }

                int SSLSocket::getSocketId() const {
                    return socketId;
                }

                client::Address SSLSocket::getAddress() const {
                    return client::Address(socket.lowest_layer().remote_endpoint().address().to_string(),
                                           remoteEndpoint.getPort());
                }

                std::unique_ptr<Address> SSLSocket::localSocketAddress() const {
                    asio::error_code ec;
                    asio::ip::basic_endpoint<asio::ip::tcp> localEndpoint = socket.lowest_layer().local_endpoint(ec);
                    if (ec) {
                        return std::unique_ptr<Address>();
                    }
                    return std::unique_ptr<Address>(
                            new Address(localEndpoint.address().to_string(), localEndpoint.port()));
                }

                void SSLSocket::close() {
                    asio::error_code ec;
                    // Call the non-exception throwing versions of the following method
                    socket.lowest_layer().close(ec);
                }

                int SSLSocket::handleError(const std::string &source, size_t numBytes,
                                           const asio::error_code &error) const {
                    if (error && error != asio::error::try_again && error != asio::error::would_block) {
                        throw exception::IOException(source, error.message());
                    }
                    return (int) numBytes;
                }

                void SSLSocket::setSocketOptions() {
                    auto &lowestLayer = socket.lowest_layer();

                    lowestLayer.set_option(asio::ip::tcp::no_delay(socketOptions.isTcpNoDelay()));

                    lowestLayer.set_option(asio::socket_base::keep_alive(socketOptions.isKeepAlive()));

                    lowestLayer.set_option(asio::socket_base::reuse_address(socketOptions.isReuseAddress()));

                    int lingerSeconds = socketOptions.getLingerSeconds();
                    if (lingerSeconds > 0) {
                        lowestLayer.set_option(asio::socket_base::linger(true, lingerSeconds));
                    }

                    int bufferSize = socketOptions.getBufferSizeInBytes();
                    if (bufferSize > 0) {
                        lowestLayer.set_option(asio::socket_base::receive_buffer_size(bufferSize));
                        lowestLayer.set_option(asio::socket_base::send_buffer_size(bufferSize));
                    }

                    // SO_NOSIGPIPE seems to be internally handled by asio on connect and accept. no such option
                    // is defined at the api, hence not setting this option
                }

                SSLSocket::ReadHandler::ReadHandler(size_t &numRead, asio::error_code &ec) : numRead(numRead),
                                                                                             errorCode(ec) {}

                void SSLSocket::ReadHandler::operator()(const asio::error_code &err, std::size_t bytes_transferred) {
                    errorCode = err;
                    numRead += bytes_transferred;
                }

                size_t &SSLSocket::ReadHandler::getNumRead() const {
                    return numRead;
                }

                asio::error_code &SSLSocket::ReadHandler::getErrorCode() const {
                    return errorCode;
                }

                std::ostream &operator<<(std::ostream &out, const SSLSocket::CipherInfo &info) {
                    out << "Cipher{"
                           "Name: " << info.name <<
                        ", Bits:" << info.numberOfBits <<
                        ", Version:" << info.version <<
                        ", Description:" << info.description << "}";

                    return out;
                }
            }
        }
    }
}



#endif // HZ_BUILD_WITH_SSL




namespace hazelcast {
    namespace client {
        namespace internal {
            namespace socket {
                TcpSocket::TcpSocket(const client::Address &address, const client::config::SocketOptions *socketOptions)
                        : configAddress(address) {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    int n = WSAStartup(MAKEWORD(2, 0), &wsa_data);
                    if(n == -1) throw exception::IOException("TcpSocket::TcpSocket ", "WSAStartup error");
#endif
                    struct addrinfo hints;
                    memset(&hints, 0, sizeof(hints));
                    hints.ai_family = AF_UNSPEC;
                    hints.ai_socktype = SOCK_STREAM;
                    hints.ai_flags = AI_PASSIVE;

                    int status;
                    serverInfo = NULL;
                    if ((status = getaddrinfo(address.getHost().c_str(),
                                              hazelcast::util::IOUtil::to_string(address.getPort()).c_str(), &hints,
                                              &serverInfo)) != 0) {
                        std::string message = util::IOUtil::to_string(address) + " getaddrinfo error: " +
                                              std::string(gai_strerror(status));
                        throw client::exception::IOException("TcpSocket::TcpSocket", message);
                    }

                    socketId = ::socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
                    if (-1 == socketId) {
                        throwIOException("TcpSocket", "[TcpSocket::TcpSocket] Failed to obtain socket.");
                    }

                    if (socketOptions) {
                        setSocketOptions(*socketOptions);
                    }

                    isOpen.store(true);
                }

                TcpSocket::TcpSocket(int socketId)
                        : serverInfo(NULL), socketId(socketId), isOpen(true) {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    int n= WSAStartup(MAKEWORD(2, 0), &wsa_data);
                    if(n == -1) throw exception::IOException("TcpSocket::TcpSocket ", "WSAStartup error");
#endif
                }

                TcpSocket::~TcpSocket() {
                    close();
                }

                int TcpSocket::connect(int timeoutInMillis) {
                    assert(serverInfo != NULL && "Socket is already connected");
                    setBlocking(false);

                    if (::connect(socketId, serverInfo->ai_addr, serverInfo->ai_addrlen)) {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                        int error = WSAGetLastError();
                        if (WSAEWOULDBLOCK != error && WSAEINPROGRESS != error && WSAEALREADY != error) {
#else
                        int error = errno;
                        if (EINPROGRESS != error && EALREADY != error) {
#endif
                            throwIOException(error, "connect",
                                             "Failed to connect the socket. Error at ::connect system call.");
                        }
                    }

                    struct timeval tv;
                    tv.tv_sec = timeoutInMillis / 1000;
                    tv.tv_usec = (timeoutInMillis - (int) tv.tv_sec * 1000) * 1000;
                    fd_set mySet, err;
                    FD_ZERO(&mySet);
                    FD_ZERO(&err);
                    FD_SET(socketId, &mySet);
                    FD_SET(socketId, &err);
                    errno = 0;
                    if (select(socketId + 1, NULL, &mySet, &err, &tv) > 0) {
                        return 0;
                    }
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    int error = WSAGetLastError();
#else
                    int error = errno;
#endif

                    if (error) {
                        throwIOException(error, "connect",
                                         "Failed to connect the socket. Error at ::select system call");
                    } else {
                        char msg[200];
                        util::hz_snprintf(msg, 200, "Failed to connect to %s:%d in %d milliseconds",
                                          configAddress.getHost().c_str(), configAddress.getPort(), timeoutInMillis);
                        throw exception::IOException("TcpSocket::connect ", msg);
                    }

                    return -1;
                }

                void TcpSocket::setBlocking(bool blocking) {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    unsigned long iMode;
                    if(blocking){
                        iMode = 0l;
                    } else {
                        iMode = 1l;
                    }
                    if (ioctlsocket(socketId, FIONBIO, &iMode)) {
                        char errorMsg[200];
                        int error = WSAGetLastError();
                        util::strerror_s(error, errorMsg, 200, "Failed to set the blocking mode.");
                        throw client::exception::IOException("TcpSocket::setBlocking", errorMsg);
                    }
#else
                    int arg = fcntl(socketId, F_GETFL, NULL);
                    if (-1 == arg) {
                        char errorMsg[200];
                        util::strerror_s(errno, errorMsg, 200, "Could not get the value of socket flags.");
                        throw client::exception::IOException("TcpSocket::setBlocking", errorMsg);
                    }
                    if (blocking) {
                        arg &= (~O_NONBLOCK);
                    } else {
                        arg |= O_NONBLOCK;
                    }
                    if (-1 == fcntl(socketId, F_SETFL, arg)) {
                        char errorMsg[200];
                        util::strerror_s(errno, errorMsg, 200, "Could not set the blocking value of socket flags.");
                        throw client::exception::IOException("TcpSocket::setBlocking", errorMsg);
                    }
#endif
                }

                int TcpSocket::send(const void *buffer, int len, int flag) {
#if !defined(WIN32) && !defined(_WIN32) && !defined(WIN64) && !defined(_WIN64)
                    errno = 0;
#endif

                    int bytesSend = 0;
                    /**
                     * In linux, sometimes SIGBUS may be received during this call when the server closes the connection.
                     * The returned error code is still error when this flag is set. Hence, it is safe to use.
                     * MSG_NOSIGNAL (since Linux 2.2)
                     * Requests not to send SIGPIPE on errors on stream oriented sockets when the other end breaks the connection.
                     * The EPIPE error is still returned.
                     */

                    if (flag == MSG_WAITALL) {
                        setBlocking(true);
                    }

                    if ((bytesSend = ::send(socketId, (char *) buffer, (size_t) len, MSG_NOSIGNAL)) == -1) {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                        int error = WSAGetLastError();
                        if (WSAEWOULDBLOCK == error) {
#else
                        int error = errno;
                        if (EAGAIN == error) {
#endif
                            if (flag == MSG_WAITALL) {
                                setBlocking(false);
                            }
                            return 0;
                        }

                        if (flag == MSG_WAITALL) {
                            setBlocking(false);
                        }
                        throwIOException(error, "send", "Send failed.");
                    }
                    if (flag == MSG_WAITALL) {
                        setBlocking(false);
                    }
                    return bytesSend;
                }

                int TcpSocket::receive(void *buffer, int len, int flag) {
#if !defined(WIN32) && !defined(_WIN32) && !defined(WIN64) && !defined(_WIN64)
                    errno = 0;
#endif

                    int size = ::recv(socketId, (char *) buffer, (size_t) len, flag);

                    if (size == -1) {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                        int error = WSAGetLastError();
                        if (WSAEWOULDBLOCK == error) {
#else
                        int error = errno;
                        if (EAGAIN == error) {
#endif
                            return 0;
                        }

                        throwIOException(error, "receive", "Receive failed.");
                    } else if (size == 0) {
                        throw client::exception::IOException("TcpSocket::receive", "Connection closed by remote");
                    }
                    return size;
                }

                int TcpSocket::getSocketId() const {
                    return socketId;
                }

                client::Address TcpSocket::getAddress() const {
                    char host[1024];
                    char service[20];
                    getnameinfo(serverInfo->ai_addr, serverInfo->ai_addrlen, host, sizeof host, service, sizeof service,
                                NI_NUMERICHOST | NI_NUMERICSERV);
                    Address address(host, atoi(service));
                    return address;
                }

                void TcpSocket::close() {
                    bool expected = true;
                    if (isOpen.compare_exchange_strong(expected, false)) {
                        if (serverInfo != NULL)
                            ::freeaddrinfo(serverInfo);

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                        ::shutdown(socketId, SD_RECEIVE);
                        char buffer[1];
                        ::recv(socketId, buffer, 1, MSG_WAITALL);
                        WSACleanup();
                        closesocket(socketId);
#else
                        ::shutdown(socketId, SHUT_RD);
                        char buffer[1];
                        ::recv(socketId, buffer, 1, MSG_WAITALL);
                        ::close(socketId);
#endif
                    }
                }

                void TcpSocket::throwIOException(const char *methodName, const char *prefix) const {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    int error = WSAGetLastError();
#else
                    int error = errno;
#endif

                    throwIOException(error, methodName, prefix);
                }

                void TcpSocket::throwIOException(int error, const char *methodName, const char *prefix) const {
                    char errorMsg[200];
                    util::strerror_s(error, errorMsg, 200, prefix);
                    throw client::exception::IOException(std::string("TcpSocket::") + methodName, errorMsg);
                }

                std::unique_ptr<Address> TcpSocket::localSocketAddress() const {
                    struct sockaddr_in sin;
                    socklen_t addrlen = sizeof(sin);
                    if (getsockname(socketId, (struct sockaddr *) &sin, &addrlen) == 0 &&
                        sin.sin_family == AF_INET &&
                        addrlen == sizeof(sin)) {
                        int localPort = ntohs(sin.sin_port);
                        char *localIp = inet_ntoa(sin.sin_addr);
                        return std::unique_ptr<Address>(new Address(localIp ? localIp : "", localPort));
                    } else {
                        return std::unique_ptr<Address>();
                    }
                }

                void TcpSocket::setSocketOptions(const client::config::SocketOptions &socketOptions) {
                    int optionValue = socketOptions.getBufferSizeInBytes();
                    if (::setsockopt(socketId, SOL_SOCKET, SO_RCVBUF, (char *) &optionValue, sizeof(optionValue))) {
                        throwIOException("setSocketOptions", "Failed to set socket receive buffer size.");
                    }
                    if (::setsockopt(socketId, SOL_SOCKET, SO_SNDBUF, (char *) &optionValue, sizeof(optionValue))) {
                        throwIOException("setSocketOptions", "Failed to set socket send buffer size.");
                    }

                    optionValue = socketOptions.isTcpNoDelay();
                    if (::setsockopt(socketId, IPPROTO_TCP, TCP_NODELAY, (char *) &optionValue, sizeof(optionValue))) {
                        throwIOException("setSocketOptions", "Failed to set TCP_NODELAY option on the socket.");
                    }

                    optionValue = socketOptions.isKeepAlive();
                    if (::setsockopt(socketId, SOL_SOCKET, SO_KEEPALIVE, (char *) &optionValue, sizeof(optionValue))) {
                        throwIOException("setSocketOptions", "Failed to set SO_KEEPALIVE option on the socket.");
                    }

                    optionValue = socketOptions.isReuseAddress();
                    if (::setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, (char *) &optionValue, sizeof(optionValue))) {
                        throwIOException("setSocketOptions", "Failed to set SO_REUSEADDR option on the socket.");
                    }

                    optionValue = socketOptions.getLingerSeconds();
                    if (optionValue > 0) {
                        struct linger so_linger;
                        so_linger.l_onoff = 1;
                        so_linger.l_linger = optionValue;

                        if (::setsockopt(socketId, SOL_SOCKET, SO_LINGER, (char *) &so_linger, sizeof(so_linger))) {
                            throwIOException("setSocketOptions", "Failed to set SO_LINGER option on the socket.");
                        }
                    }

#if defined(SO_NOSIGPIPE)
                    optionValue = 1;
                    if (setsockopt(socketId, SOL_SOCKET, SO_NOSIGPIPE, (char *) &optionValue, sizeof(optionValue))) {
                        throwIOException("TcpSocket", "Failed to set socket option SO_NOSIGPIPE.");
                    }
#endif

                }
            }
        }
    }
}



namespace hazelcast {
    namespace client {
        namespace internal {
            namespace eviction {
                bool EvictAlways::isEvictionRequired() const {
                    // Evict always at any case
                    return true;
                }

                const std::unique_ptr<EvictionChecker> EvictionChecker::EVICT_ALWAYS = std::unique_ptr<EvictionChecker>(
                        new EvictAlways());
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        Client::Client(const std::shared_ptr<std::string> &uuid, const std::shared_ptr<Address> &socketAddress,
                       const std::string &name) : Endpoint(uuid, socketAddress), name(name) {}

        const std::string &Client::getName() const {
            return name;
        }
    }
}


#define SECONDS_IN_A_MINUTE     60

namespace hazelcast {
    namespace client {
        TransactionOptions::TransactionOptions()
                : timeoutSeconds(2 * SECONDS_IN_A_MINUTE)//2 minutes
                , durability(1), transactionType(TransactionType::TWO_PHASE) {

        }

        TransactionType TransactionOptions::getTransactionType() const {
            return transactionType;
        }

        TransactionOptions &TransactionOptions::setTransactionType(TransactionType transactionType) {
            this->transactionType = transactionType;
            return *this;
        }

        int TransactionOptions::getTimeout() const {
            return timeoutSeconds;
        }

        TransactionOptions &TransactionOptions::setTimeout(int timeoutInSeconds) {
            if (timeoutInSeconds <= 0) {
                throw exception::IllegalStateException("TransactionOptions::setTimeout", "Timeout must be positive!");
            }
            this->timeoutSeconds = timeoutInSeconds;
            return *this;
        }

        int TransactionOptions::getDurability() const {
            return durability;
        }

        TransactionOptions &TransactionOptions::setDurability(int durability) {
            if (durability < 0) {
                throw exception::IllegalStateException("TransactionOptions::setDurability",
                                                       "Durability cannot be negative!");
            }
            this->durability = durability;
            return *this;
        }

        int TransactionOptions::getTimeoutMillis() const {
            return timeoutSeconds * 1000;
        }

        TransactionType::TransactionType(Type value) : value(value) {
        }

        TransactionType::operator int() const {
            return value;
        }

        void TransactionType::operator=(int i) {
            if (i == TWO_PHASE) {
                value = TWO_PHASE;
            } else {
                value = LOCAL;
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace topic {
            namespace impl {
                namespace reliable {
                    ReliableTopicMessage::ReliableTopicMessage() {
                    }

                    ReliableTopicMessage::ReliableTopicMessage(
                            hazelcast::client::serialization::pimpl::Data payloadData,
                            std::unique_ptr<Address> &address)
                            : publishTime(util::currentTimeMillis()), publisherAddress(std::move(address)),
                              payload(payloadData) {
                    }

                    int64_t ReliableTopicMessage::getPublishTime() const {
                        return publishTime;
                    }

                    const Address *ReliableTopicMessage::getPublisherAddress() const {
                        return publisherAddress.get();
                    }

                    const serialization::pimpl::Data &ReliableTopicMessage::getPayload() const {
                        return payload;
                    }

                    int ReliableTopicMessage::getFactoryId() const {
                        return F_ID;
                    }

                    int ReliableTopicMessage::getClassId() const {
                        return RELIABLE_TOPIC_MESSAGE;
                    }

                    void ReliableTopicMessage::writeData(serialization::ObjectDataOutput &out) const {
                        out.writeLong(publishTime);
                        out.writeObject<serialization::IdentifiedDataSerializable>(publisherAddress.get());
                        out.writeData(&payload);
                    }

                    void ReliableTopicMessage::readData(serialization::ObjectDataInput &in) {
                        publishTime = in.readLong();
                        publisherAddress = in.readObject<Address>();
                        payload = in.readData();
                    }
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace topic {
            namespace impl {
                namespace reliable {
                    ReliableTopicExecutor::ReliableTopicExecutor(Ringbuffer<ReliableTopicMessage> &rb,
                                                                 util::ILogger &logger)
                            : ringbuffer(rb),
                              runnerThread(std::shared_ptr<util::Runnable>(new Task(ringbuffer, q, shutdown)), logger),
                              q(10), shutdown(false) {
                    }

                    ReliableTopicExecutor::~ReliableTopicExecutor() {
                        stop();
                    }

                    void ReliableTopicExecutor::start() {
                        runnerThread.start();
                    }

                    void ReliableTopicExecutor::stop() {
                        bool expected = false;
                        if (!shutdown.compare_exchange_strong(expected, true)) {
                            return;
                        }

                        topic::impl::reliable::ReliableTopicExecutor::Message m;
                        m.type = topic::impl::reliable::ReliableTopicExecutor::CANCEL;
                        m.callback = NULL;
                        m.sequence = -1;
                        execute(m);
                        runnerThread.join();
                    }

                    void ReliableTopicExecutor::execute(const Message &m) {
                        q.push(m);
                    }

                    void ReliableTopicExecutor::Task::run() {
                        while (!shutdown) {
                            Message m = q.pop();
                            if (CANCEL == m.type) {
                                // exit the thread
                                return;
                            }
                            try {
                                proxy::ClientRingbufferProxy<ReliableTopicMessage> &ringbuffer =
                                        static_cast<proxy::ClientRingbufferProxy<ReliableTopicMessage> &>(rb);
                                std::shared_ptr<spi::impl::ClientInvocationFuture> future = ringbuffer.readManyAsync(
                                        m.sequence, 1, m.maxCount);
                                std::shared_ptr<protocol::ClientMessage> responseMsg;
                                do {
                                    if (future->get(1000, TimeUnit::MILLISECONDS())) {
                                        responseMsg = future->get(); // every one second
                                    }
                                } while (!shutdown && (protocol::ClientMessage *) NULL == responseMsg.get());

                                if (!shutdown) {
                                    std::shared_ptr<DataArray<ReliableTopicMessage> > allMessages(
                                            ringbuffer.getReadManyAsyncResponseObject(
                                                    responseMsg));

                                    m.callback->onResponse(allMessages);
                                }
                            } catch (exception::IException &e) {
                                m.callback->onFailure(std::shared_ptr<exception::IException>(e.clone()));
                            }
                        }

                    }

                    const std::string ReliableTopicExecutor::Task::getName() const {
                        return "ReliableTopicExecutor Task";
                    }

                    ReliableTopicExecutor::Task::Task(Ringbuffer<ReliableTopicMessage> &rb,
                                                      util::BlockingConcurrentQueue<ReliableTopicExecutor::Message> &q,
                                                      util::AtomicBoolean &shutdown) : rb(rb), q(q),
                                                                                       shutdown(shutdown) {}
                }
            }
        }
    }
}

/*
 * ClientMessage.cpp
 *
 *  Created on: Mar 17, 2015
 *      Author: ihsan
 */



namespace hazelcast {
    namespace client {
        namespace protocol {
            const std::string ClientTypes::CPP = "CPP";

            ClientMessage::ClientMessage(int32_t size) : LittleEndianBufferWrapper(size), retryable(false) {
                setFrameLength(size);
            }

            ClientMessage::~ClientMessage() {
            }

            void ClientMessage::wrapForEncode(int32_t size) {
                wrapForWrite(size, HEADER_SIZE);

                setFrameLength(size);
                setVersion(HAZELCAST_CLIENT_PROTOCOL_VERSION);
                addFlag(BEGIN_AND_END_FLAGS);
                setCorrelationId(0);
                setPartitionId(-1);
                setDataOffset(HEADER_SIZE);
            }

            std::unique_ptr<ClientMessage> ClientMessage::createForEncode(int32_t size) {
                std::unique_ptr<ClientMessage> msg(new ClientMessage(size));
                msg->wrapForEncode(size);
                return msg;
            }

            std::unique_ptr<ClientMessage> ClientMessage::createForDecode(const ClientMessage &msg) {
                // copy constructor does not do deep copy of underlying buffer but just uses a shared_ptr
                std::unique_ptr<ClientMessage> copy(new ClientMessage(msg));
                copy->wrapForRead(copy->getCapacity(), ClientMessage::HEADER_SIZE);
                return copy;
            }

            std::unique_ptr<ClientMessage> ClientMessage::create(int32_t size) {
                return std::unique_ptr<ClientMessage>(new ClientMessage(size));
            }

            //----- Setter methods begin --------------------------------------
            void ClientMessage::setFrameLength(int32_t length) {
                util::Bits::nativeToLittleEndian4(&length, &(*buffer)[FRAME_LENGTH_FIELD_OFFSET]);
            }

            void ClientMessage::setMessageType(uint16_t type) {
                util::Bits::nativeToLittleEndian2(&type, &(*buffer)[TYPE_FIELD_OFFSET]);
            }

            void ClientMessage::setVersion(uint8_t value) {
                (*buffer)[VERSION_FIELD_OFFSET] = value;
            }

            uint8_t ClientMessage::getFlags() {
                return (*buffer)[FLAGS_FIELD_OFFSET];
            }

            void ClientMessage::addFlag(uint8_t flags) {
                (*buffer)[FLAGS_FIELD_OFFSET] = getFlags() | flags;
            }

            void ClientMessage::setCorrelationId(int64_t id) {
                util::Bits::nativeToLittleEndian8(&id, &(*buffer)[CORRELATION_ID_FIELD_OFFSET]);
            }

            void ClientMessage::setPartitionId(int32_t partitionId) {
                util::Bits::nativeToLittleEndian4(&partitionId, &(*buffer)[PARTITION_ID_FIELD_OFFSET]);
            }

            void ClientMessage::setDataOffset(uint16_t offset) {
                util::Bits::nativeToLittleEndian2(&offset, &(*buffer)[DATA_OFFSET_FIELD_OFFSET]);
            }

            void ClientMessage::updateFrameLength() {
                setFrameLength(getIndex());
            }

            void ClientMessage::set(const std::string *value) {
                setNullable<std::string>(value);
            }


            void ClientMessage::set(const Address &data) {
                codec::AddressCodec::encode(data, *this);
            }

            void ClientMessage::set(const serialization::pimpl::Data &value) {
                setArray<byte>(value.toByteArray());
            }

            void ClientMessage::set(const serialization::pimpl::Data *value) {
                setNullable<serialization::pimpl::Data>(value);
            }

            //----- Setter methods end ---------------------

            int32_t ClientMessage::fillMessageFrom(util::ByteBuffer &byteBuff, int32_t offset, int32_t frameLen) {
                size_t numToRead = (size_t) (frameLen - offset);
                size_t numRead = byteBuff.readBytes(&(*buffer)[offset], numToRead);

                if (numRead == numToRead) {
                    wrapForRead(frameLen, ClientMessage::HEADER_SIZE);
                }

                return (int32_t) numRead;
            }

            //----- Getter methods begin -------------------
            int32_t ClientMessage::getFrameLength() const {
                int32_t result;

                util::Bits::littleEndianToNative4(&(*buffer)[FRAME_LENGTH_FIELD_OFFSET], &result);

                return result;
            }

            uint16_t ClientMessage::getMessageType() const {
                uint16_t type;

                util::Bits::littleEndianToNative2(&(*buffer)[TYPE_FIELD_OFFSET], &type);

                return type;
            }

            uint8_t ClientMessage::getVersion() {
                return (*buffer)[VERSION_FIELD_OFFSET];
            }

            int64_t ClientMessage::getCorrelationId() const {
                int64_t value;
                util::Bits::littleEndianToNative8(&(*buffer)[CORRELATION_ID_FIELD_OFFSET], &value);
                return value;
            }

            int32_t ClientMessage::getPartitionId() const {
                int32_t value;
                util::Bits::littleEndianToNative4(&(*buffer)[PARTITION_ID_FIELD_OFFSET], &value);
                return value;
            }

            uint16_t ClientMessage::getDataOffset() const {
                uint16_t value;
                util::Bits::littleEndianToNative2(&(*buffer)[DATA_OFFSET_FIELD_OFFSET], &value);
                return value;
            }

            bool ClientMessage::isFlagSet(uint8_t flag) const {
                return flag == ((*buffer)[FLAGS_FIELD_OFFSET] & flag);
            }

            template<>
            uint8_t ClientMessage::get() {
                return getUint8();
            }

            template<>
            bool ClientMessage::get() {
                return getBoolean();
            }

            template<>
            int32_t ClientMessage::get() {
                return getInt32();
            }

            template<>
            int64_t ClientMessage::get() {
                return getInt64();
            }

            template<>
            std::string ClientMessage::get() {
                return getStringUtf8();
            }

            template<>
            Address ClientMessage::get() {
                return codec::AddressCodec::decode(*this);
            }

            template<>
            util::UUID ClientMessage::get() {
                return codec::UUIDCodec::decode(*this);
            }

            template<>
            Member ClientMessage::get() {
                return codec::MemberCodec::decode(*this);
            }

            template<>
            map::DataEntryView ClientMessage::get() {
                return codec::DataEntryViewCodec::decode(*this);
            }

            template<>
            serialization::pimpl::Data ClientMessage::get() {
                int32_t len = getInt32();

                assert(checkReadAvailable(len));

                byte *start = ix();
                std::unique_ptr<std::vector<byte> > bytes = std::unique_ptr<std::vector<byte> >(
                        new std::vector<byte>(start,
                                              start +
                                              len));
                index += len;

                return serialization::pimpl::Data(bytes);
            }

            template<>
            codec::StackTraceElement ClientMessage::get() {
                return codec::StackTraceElementCodec::decode(*this);
            }

            template<>
            std::pair<serialization::pimpl::Data, serialization::pimpl::Data> ClientMessage::get() {
                serialization::pimpl::Data key = get<serialization::pimpl::Data>();
                serialization::pimpl::Data value = get<serialization::pimpl::Data>();

                return std::pair<serialization::pimpl::Data, serialization::pimpl::Data>(key, value);
            }

            template<>
            std::pair<Address, std::vector<int64_t> > ClientMessage::get() {
                Address address = codec::AddressCodec::decode(*this);
                std::vector<int64_t> values = getArray<int64_t>();
                return std::make_pair(address, values);
            }

            template<>
            std::pair<Address, std::vector<int32_t> > ClientMessage::get() {
                Address address = codec::AddressCodec::decode(*this);
                std::vector<int32_t> partitions = getArray<int32_t>();
                return std::make_pair(address, partitions);
            }

            template<>
            std::pair<std::string, int64_t> ClientMessage::get() {
                std::string key = get<std::string>();
                int64_t value = get<int64_t>();
                return std::make_pair(key, value);
            }
            //----- Getter methods end --------------------------

            //----- Data size calculation functions BEGIN -------
            int32_t ClientMessage::calculateDataSize(uint8_t param) {
                return UINT8_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(bool param) {
                return UINT8_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(int32_t param) {
                return INT32_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(int64_t param) {
                return INT64_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(const std::string &param) {
                return INT32_SIZE +  // bytes for the length field
                       (int32_t) param.length();
            }

            int32_t ClientMessage::calculateDataSize(const std::string *param) {
                return calculateDataSizeNullable<std::string>(param);
            }

            int32_t ClientMessage::calculateDataSize(const serialization::pimpl::Data &param) {
                return INT32_SIZE +  // bytes for the length field
                       (int32_t) param.totalSize();
            }

            int32_t ClientMessage::calculateDataSize(const serialization::pimpl::Data *param) {
                return calculateDataSizeNullable<serialization::pimpl::Data>(param);
            }
            //----- Data size calculation functions END ---------

            void ClientMessage::append(const ClientMessage *msg) {
                // no need to double check if correlation ids match here,
                // since we make sure that this is guaranteed at the caller that they are matching !
                int32_t dataSize = msg->getDataSize();
                int32_t existingFrameLen = getFrameLength();
                int32_t newFrameLen = existingFrameLen + dataSize;
                ensureBufferSize(newFrameLen);
                memcpy(&(*buffer)[existingFrameLen], &(*msg->buffer)[0], (size_t) dataSize);
                setFrameLength(newFrameLen);
            }

            int32_t ClientMessage::getDataSize() const {
                return this->getFrameLength() - getDataOffset();
            }

            void ClientMessage::ensureBufferSize(int32_t requiredCapacity) {
                int32_t currentCapacity = getCapacity();
                if (requiredCapacity > currentCapacity) {
                    // allocate new memory
                    int32_t newSize = findSuitableCapacity(requiredCapacity, currentCapacity);

                    buffer->resize(newSize, 0);
                    wrapForWrite(newSize, getIndex());
                } else {
                    // Should never be here
                    assert(0);
                }
            }

            int32_t ClientMessage::findSuitableCapacity(int32_t requiredCapacity, int32_t existingCapacity) const {
                int32_t size = existingCapacity;
                do {
                    size <<= 1;
                } while (size < requiredCapacity);

                return size;
            }

            bool ClientMessage::isRetryable() const {
                return retryable;
            }

            void ClientMessage::setRetryable(bool shouldRetry) {
                retryable = shouldRetry;
            }

            int32_t ClientMessage::writeTo(Socket &socket, int32_t offset, int32_t frameLen) {
                int32_t numBytesSent = 0;

                int32_t numBytesLeft = frameLen - offset;
                if (numBytesLeft > 0) {
                    numBytesSent = socket.send(&(*buffer)[offset], numBytesLeft);
                }

                return numBytesSent;
            }

            bool ClientMessage::isComplete() const {
                return (index >= HEADER_SIZE) && (index == getFrameLength());
            }

            std::ostream &operator<<(std::ostream &os, const ClientMessage &message) {
                os << "ClientMessage{length=" << message.getIndex()
                   << ", correlationId=" << message.getCorrelationId()
                   << ", messageType=0x" << std::hex << message.getMessageType() << std::dec
                   << ", partitionId=" << message.getPartitionId()
                   << ", isComplete=" << message.isComplete()
                   << ", isRetryable=" << message.isRetryable()
                   << ", isEvent=" << message.isFlagSet(message.LISTENER_EVENT_FLAG)
                   << "}";

                return os;
            }

            int32_t ClientMessage::calculateDataSize(const Address &param) {
                return codec::AddressCodec::calculateDataSize(param);
            }
        }
    }
}
/*
 *
 *  Created on: May 17, 2016
 *      Author: ihsan
 */


namespace hazelcast {
    namespace client {
        namespace protocol {
            ExceptionFactory::~ExceptionFactory() {
            }

            ClientExceptionFactory::ClientExceptionFactory() {
                registerException(ARRAY_INDEX_OUT_OF_BOUNDS,
                                  new ExceptionFactoryImpl<exception::ArrayIndexOutOfBoundsException>());
                registerException(ARRAY_STORE, new ExceptionFactoryImpl<exception::ArrayStoreException>());
                registerException(AUTHENTICATIONERROR, new ExceptionFactoryImpl<exception::AuthenticationException>());
                registerException(CACHE_NOT_EXISTS, new ExceptionFactoryImpl<exception::CacheNotExistsException>());
                registerException(CALLER_NOT_MEMBER, new ExceptionFactoryImpl<exception::CallerNotMemberException>());
                registerException(CANCELLATION, new ExceptionFactoryImpl<exception::CancellationException>());
                registerException(CLASS_CAST, new ExceptionFactoryImpl<exception::ClassCastException>());
                registerException(CLASS_NOT_FOUND, new ExceptionFactoryImpl<exception::ClassNotFoundException>());
                registerException(CONCURRENT_MODIFICATION,
                                  new ExceptionFactoryImpl<exception::ConcurrentModificationException>());
                registerException(CONFIG_MISMATCH, new ExceptionFactoryImpl<exception::ConfigMismatchException>());
                registerException(CONFIGURATION, new ExceptionFactoryImpl<exception::ConfigurationException>());
                registerException(DISTRIBUTED_OBJECT_DESTROYED,
                                  new ExceptionFactoryImpl<exception::DistributedObjectDestroyedException>());
                registerException(DUPLICATE_INSTANCE_NAME,
                                  new ExceptionFactoryImpl<exception::DuplicateInstanceNameException>());
                registerException(ENDOFFILE, new ExceptionFactoryImpl<exception::EOFException>());
                registerException(EXECUTION, new ExceptionFactoryImpl<exception::ExecutionException>());
                registerException(HAZELCAST, new ExceptionFactoryImpl<exception::HazelcastException>());
                registerException(HAZELCAST_INSTANCE_NOT_ACTIVE,
                                  new ExceptionFactoryImpl<exception::HazelcastInstanceNotActiveException>());
                registerException(HAZELCAST_OVERLOAD,
                                  new ExceptionFactoryImpl<exception::HazelcastOverloadException>());
                registerException(HAZELCAST_SERIALIZATION,
                                  new ExceptionFactoryImpl<exception::HazelcastSerializationException>());
                registerException(IO, new ExceptionFactoryImpl<exception::IOException>());
                registerException(ILLEGAL_ARGUMENT, new ExceptionFactoryImpl<exception::IllegalArgumentException>());
                registerException(ILLEGAL_ACCESS_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::IllegalAccessException>());
                registerException(ILLEGAL_ACCESS_ERROR, new ExceptionFactoryImpl<exception::IllegalAccessError>());
                registerException(ILLEGAL_MONITOR_STATE,
                                  new ExceptionFactoryImpl<exception::IllegalMonitorStateException>());
                registerException(ILLEGAL_STATE, new ExceptionFactoryImpl<exception::IllegalStateException>());
                registerException(ILLEGAL_THREAD_STATE,
                                  new ExceptionFactoryImpl<exception::IllegalThreadStateException>());
                registerException(INDEX_OUT_OF_BOUNDS,
                                  new ExceptionFactoryImpl<exception::IndexOutOfBoundsException>());
                registerException(INTERRUPTED, new ExceptionFactoryImpl<exception::InterruptedException>());
                registerException(INVALID_ADDRESS, new ExceptionFactoryImpl<exception::InvalidAddressException>());
                registerException(INVALID_CONFIGURATION,
                                  new ExceptionFactoryImpl<exception::InvalidConfigurationException>());
                registerException(MEMBER_LEFT, new ExceptionFactoryImpl<exception::MemberLeftException>());
                registerException(NEGATIVE_ARRAY_SIZE,
                                  new ExceptionFactoryImpl<exception::NegativeArraySizeException>());
                registerException(NO_SUCH_ELEMENT, new ExceptionFactoryImpl<exception::NoSuchElementException>());
                registerException(NOT_SERIALIZABLE, new ExceptionFactoryImpl<exception::NotSerializableException>());
                registerException(NULL_POINTER, new ExceptionFactoryImpl<exception::NullPointerException>());
                registerException(OPERATION_TIMEOUT, new ExceptionFactoryImpl<exception::OperationTimeoutException>());
                registerException(PARTITION_MIGRATING,
                                  new ExceptionFactoryImpl<exception::PartitionMigratingException>());
                registerException(QUERY, new ExceptionFactoryImpl<exception::QueryException>());
                registerException(QUERY_RESULT_SIZE_EXCEEDED,
                                  new ExceptionFactoryImpl<exception::QueryResultSizeExceededException>());
                registerException(QUORUM, new ExceptionFactoryImpl<exception::QuorumException>());
                registerException(REACHED_MAX_SIZE, new ExceptionFactoryImpl<exception::ReachedMaxSizeException>());
                registerException(REJECTED_EXECUTION,
                                  new ExceptionFactoryImpl<exception::RejectedExecutionException>());
                registerException(REMOTE_MAP_REDUCE, new ExceptionFactoryImpl<exception::RemoteMapReduceException>());
                registerException(RESPONSE_ALREADY_SENT,
                                  new ExceptionFactoryImpl<exception::ResponseAlreadySentException>());
                registerException(RETRYABLE_HAZELCAST,
                                  new ExceptionFactoryImpl<exception::RetryableHazelcastException>());
                registerException(RETRYABLE_IO, new ExceptionFactoryImpl<exception::RetryableIOException>());
                registerException(RUNTIME, new ExceptionFactoryImpl<exception::RuntimeException>());
                registerException(SECURITY, new ExceptionFactoryImpl<exception::SecurityException>());
                registerException(SOCKET, new ExceptionFactoryImpl<exception::SocketException>());
                registerException(STALE_SEQUENCE, new ExceptionFactoryImpl<exception::StaleSequenceException>());
                registerException(TARGET_DISCONNECTED,
                                  new ExceptionFactoryImpl<exception::TargetDisconnectedException>());
                registerException(TARGET_NOT_MEMBER, new ExceptionFactoryImpl<exception::TargetNotMemberException>());
                registerException(TIMEOUT, new ExceptionFactoryImpl<exception::TimeoutException>());
                registerException(TOPIC_OVERLOAD, new ExceptionFactoryImpl<exception::TopicOverloadException>());
                registerException(TOPOLOGY_CHANGED, new ExceptionFactoryImpl<exception::TopologyChangedException>());
                registerException(TRANSACTION, new ExceptionFactoryImpl<exception::TransactionException>());
                registerException(TRANSACTION_NOT_ACTIVE,
                                  new ExceptionFactoryImpl<exception::TransactionNotActiveException>());
                registerException(TRANSACTION_TIMED_OUT,
                                  new ExceptionFactoryImpl<exception::TransactionTimedOutException>());
                registerException(URI_SYNTAX, new ExceptionFactoryImpl<exception::URISyntaxException>());
                registerException(UTF_DATA_FORMAT, new ExceptionFactoryImpl<exception::UTFDataFormatException>());
                registerException(UNSUPPORTED_OPERATION,
                                  new ExceptionFactoryImpl<exception::UnsupportedOperationException>());
                registerException(WRONG_TARGET, new ExceptionFactoryImpl<exception::WrongTargetException>());
                registerException(XA, new ExceptionFactoryImpl<exception::XAException>());
                registerException(ACCESS_CONTROL, new ExceptionFactoryImpl<exception::AccessControlException>());
                registerException(LOGIN, new ExceptionFactoryImpl<exception::LoginException>());
                registerException(UNSUPPORTED_CALLBACK,
                                  new ExceptionFactoryImpl<exception::UnsupportedCallbackException>());
                registerException(NO_DATA_MEMBER,
                                  new ExceptionFactoryImpl<exception::NoDataMemberInClusterException>());
                registerException(REPLICATED_MAP_CANT_BE_CREATED,
                                  new ExceptionFactoryImpl<exception::ReplicatedMapCantBeCreatedOnLiteMemberException>());
                registerException(MAX_MESSAGE_SIZE_EXCEEDED,
                                  new ExceptionFactoryImpl<exception::MaxMessageSizeExceeded>());
                registerException(WAN_REPLICATION_QUEUE_FULL,
                                  new ExceptionFactoryImpl<exception::WANReplicationQueueFullException>());
                registerException(ASSERTION_ERROR, new ExceptionFactoryImpl<exception::AssertionError>());
                registerException(OUT_OF_MEMORY_ERROR, new ExceptionFactoryImpl<exception::OutOfMemoryError>());
                registerException(STACK_OVERFLOW_ERROR, new ExceptionFactoryImpl<exception::StackOverflowError>());
                registerException(NATIVE_OUT_OF_MEMORY_ERROR,
                                  new ExceptionFactoryImpl<exception::NativeOutOfMemoryError>());
                registerException(SERVICE_NOT_FOUND, new ExceptionFactoryImpl<exception::ServiceNotFoundException>());
                registerException(CONSISTENCY_LOST, new ExceptionFactoryImpl<exception::ConsistencyLostException>());
            }

            ClientExceptionFactory::~ClientExceptionFactory() {
                // release memory for the factories
                for (std::map<int, hazelcast::client::protocol::ExceptionFactory *>::const_iterator it =
                        errorCodeToFactory.begin(); errorCodeToFactory.end() != it; ++it) {
                    delete (it->second);
                }
            }

            std::unique_ptr<exception::IException> ClientExceptionFactory::createException(const std::string &source,
                                                                                           protocol::ClientMessage &clientMessage) const {
                codec::ErrorCodec error = codec::ErrorCodec::decode(clientMessage);
                std::map<int, hazelcast::client::protocol::ExceptionFactory *>::const_iterator it = errorCodeToFactory.find(
                        error.errorCode);
                if (errorCodeToFactory.end() == it) {
                    return std::unique_ptr<exception::IException>(
                            new exception::UndefinedErrorCodeException(source, "",
                                                                       error.errorCode,
                                                                       clientMessage.getCorrelationId(),
                                                                       error.toString()));
                }

                return it->second->createException(source, error.message, error.toString(), error.causeErrorCode);
            }

            void ClientExceptionFactory::registerException(int32_t errorCode, ExceptionFactory *factory) {
                std::map<int, hazelcast::client::protocol::ExceptionFactory *>::iterator it = errorCodeToFactory.find(
                        errorCode);
                if (errorCodeToFactory.end() != it) {
                    char msg[100];
                    util::hz_snprintf(msg, 100, "Error code %d was already registered!!!", errorCode);
                    throw exception::IllegalStateException("ClientExceptionFactory::registerException", msg);
                }

                errorCodeToFactory[errorCode] = factory;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const std::string StackTraceElement::EMPTY_STRING("");

                StackTraceElement::StackTraceElement() : fileName((std::string *) NULL) {
                }

                StackTraceElement::StackTraceElement(const std::string &className, const std::string &method,
                                                     std::unique_ptr<std::string> &file, int line) : declaringClass(
                        className),
                                                                                                     methodName(method),
                                                                                                     fileName(std::move(
                                                                                                             file)),
                                                                                                     lineNumber(line) {}


                StackTraceElement::StackTraceElement(const StackTraceElement &rhs) {
                    declaringClass = rhs.declaringClass;
                    methodName = rhs.methodName;
                    if (NULL == rhs.fileName.get()) {
                        fileName = std::unique_ptr<std::string>();
                    } else {
                        fileName = std::unique_ptr<std::string>(new std::string(*rhs.fileName));
                    }
                    lineNumber = rhs.lineNumber;
                }

                StackTraceElement &StackTraceElement::operator=(const StackTraceElement &rhs) {
                    declaringClass = rhs.declaringClass;
                    methodName = rhs.methodName;
                    if (NULL == rhs.fileName.get()) {
                        fileName = std::unique_ptr<std::string>();
                    } else {
                        fileName = std::unique_ptr<std::string>(new std::string(*rhs.fileName));
                    }
                    lineNumber = rhs.lineNumber;
                    return *this;
                }

                const std::string &StackTraceElement::getDeclaringClass() const {
                    return declaringClass;
                }

                const std::string &StackTraceElement::getMethodName() const {
                    return methodName;
                }

                const std::string &StackTraceElement::getFileName() const {
                    if (NULL == fileName.get()) {
                        return EMPTY_STRING;
                    }

                    return *fileName;
                }

                int StackTraceElement::getLineNumber() const {
                    return lineNumber;
                }

                std::ostream &operator<<(std::ostream &out, const StackTraceElement &trace) {
                    return out << trace.getFileName() << " line " << trace.getLineNumber() << " :" <<
                               trace.getDeclaringClass() << "." << trace.getMethodName();
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                Address AddressCodec::decode(ClientMessage &clientMessage) {
                    std::string host = clientMessage.getStringUtf8();
                    int32_t port = clientMessage.getInt32();
                    return Address(host, port);
                }

                void AddressCodec::encode(const Address &address, ClientMessage &clientMessage) {
                    clientMessage.set(address.getHost());
                    clientMessage.set((int32_t) address.getPort());
                }

                int AddressCodec::calculateDataSize(const Address &address) {
                    return ClientMessage::calculateDataSize(address.getHost()) + ClientMessage::INT32_SIZE;
                }
            }
        }
    }
}
/*
 * ErrorCodec.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: ihsan
 */



namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                ErrorCodec ErrorCodec::decode(ClientMessage &clientMessage) {
                    return ErrorCodec(clientMessage);
                }

                ErrorCodec::ErrorCodec(ClientMessage &clientMessage) {
                    assert(ErrorCodec::TYPE == clientMessage.getMessageType());

                    errorCode = clientMessage.getInt32();
                    className = clientMessage.getStringUtf8();
                    message = clientMessage.getNullable<std::string>();
                    stackTrace = clientMessage.getArray<StackTraceElement>();
                    causeErrorCode = clientMessage.getInt32();
                    causeClassName = clientMessage.getNullable<std::string>();
                }

                std::string ErrorCodec::toString() const {
                    std::ostringstream out;
                    out << "Error code:" << errorCode << ", Class name that generated the error:" << className <<
                        ", ";
                    if (NULL != message.get()) {
                        out << *message;
                    }
                    out << std::endl;
                    for (std::vector<StackTraceElement>::const_iterator it = stackTrace.begin();
                         it != stackTrace.end(); ++it) {
                        out << "\t" << (*it) << std::endl;
                    }

                    out << std::endl << "Cause error code:" << causeErrorCode << std::endl;
                    if (NULL != causeClassName.get()) {
                        out << "Caused by:" << *causeClassName;
                    }

                    return out.str();
                }

                ErrorCodec::ErrorCodec(const ErrorCodec &rhs) {
                    errorCode = rhs.errorCode;
                    className = rhs.className;
                    if (NULL != rhs.message.get()) {
                        message = std::unique_ptr<std::string>(new std::string(*rhs.message));
                    }
                    stackTrace = rhs.stackTrace;
                    causeErrorCode = rhs.causeErrorCode;
                    if (NULL != rhs.causeClassName.get()) {
                        causeClassName = std::unique_ptr<std::string>(new std::string(*rhs.causeClassName));
                    }
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                Member MemberCodec::decode(ClientMessage &clientMessage) {
                    Address address = AddressCodec::decode(clientMessage);
                    std::string uuid = clientMessage.get<std::string>();
                    bool liteMember = clientMessage.get<bool>();
                    int32_t attributeSize = clientMessage.get<int32_t>();
                    std::map<std::string, std::string> attributes;
                    for (int i = 0; i < attributeSize; i++) {
                        std::string key = clientMessage.get<std::string>();
                        std::string value = clientMessage.get<std::string>();
                        attributes[key] = value;
                    }

                    return Member(address, uuid, liteMember, attributes);
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                util::UUID UUIDCodec::decode(ClientMessage &clientMessage) {
                    return util::UUID(clientMessage.get<int64_t>(), clientMessage.get<int64_t>());
                }

                void UUIDCodec::encode(const util::UUID &uuid, ClientMessage &clientMessage) {
                    clientMessage.set(uuid.getMostSignificantBits());
                    clientMessage.set(uuid.getLeastSignificantBits());
                }

                int UUIDCodec::calculateDataSize(const util::UUID &uuid) {
                    return UUID_DATA_SIZE;
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                StackTraceElement StackTraceElementCodec::decode(ClientMessage &clientMessage) {
                    std::string className = clientMessage.getStringUtf8();
                    std::string methodName = clientMessage.getStringUtf8();
                    std::unique_ptr<std::string> fileName = clientMessage.getNullable<std::string>();
                    int32_t lineNumber = clientMessage.getInt32();

                    return StackTraceElement(className, methodName, fileName, lineNumber);
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                map::DataEntryView DataEntryViewCodec::decode(ClientMessage &clientMessage) {
                    serialization::pimpl::Data key = clientMessage.get<serialization::pimpl::Data>(); // key
                    serialization::pimpl::Data value = clientMessage.get<serialization::pimpl::Data>(); // value
                    int64_t cost = clientMessage.get<int64_t>(); // cost
                    int64_t creationTime = clientMessage.get<int64_t>(); // creationTime
                    int64_t expirationTime = clientMessage.get<int64_t>(); // expirationTime
                    int64_t hits = clientMessage.get<int64_t>(); // hits
                    int64_t lastAccessTime = clientMessage.get<int64_t>(); // lastAccessTime
                    int64_t lastStoredTime = clientMessage.get<int64_t>(); // lastStoredTime
                    int64_t lastUpdateTime = clientMessage.get<int64_t>(); // lastUpdateTime
                    int64_t version = clientMessage.get<int64_t>(); // version
                    int64_t evictionCriteria = clientMessage.get<int64_t>(); // evictionCriteriaNumber
                    int64_t ttl = clientMessage.get<int64_t>();  // ttl
                    return map::DataEntryView(key, value, cost, creationTime, expirationTime, hits, lastAccessTime,
                                              lastStoredTime, lastUpdateTime, version, evictionCriteria, ttl);
                }

                void DataEntryViewCodec::encode(const map::DataEntryView &view, ClientMessage &clientMessage) {
                    clientMessage.set(view.getKey());
                    clientMessage.set(view.getValue());
                    clientMessage.set((int64_t) view.getCost());
                    clientMessage.set((int64_t) view.getCreationTime());
                    clientMessage.set((int64_t) view.getExpirationTime());
                    clientMessage.set((int64_t) view.getHits());
                    clientMessage.set((int64_t) view.getLastAccessTime());
                    clientMessage.set((int64_t) view.getLastStoredTime());
                    clientMessage.set((int64_t) view.getLastUpdateTime());
                    clientMessage.set((int64_t) view.getVersion());
                    clientMessage.set((int64_t) view.getEvictionCriteriaNumber());
                    clientMessage.set((int64_t) view.getTtl());
                }

                int DataEntryViewCodec::calculateDataSize(const map::DataEntryView &view) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;;
                    return dataSize
                           + ClientMessage::calculateDataSize(view.getKey())
                           + ClientMessage::calculateDataSize(view.getValue())
                           + ClientMessage::INT64_SIZE * 10;
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {

            Principal::Principal(std::unique_ptr<std::string> &id, std::unique_ptr<std::string> &owner) : uuid(
                    std::move(id)),
                                                                                                          ownerUuid(
                                                                                                                  std::move(
                                                                                                                          owner)) {
            }

            const std::string *Principal::getUuid() const {
                return uuid.get();
            }

            const std::string *Principal::getOwnerUuid() const {
                return ownerUuid.get();
            }

            bool Principal::operator==(const Principal &rhs) const {
                if (ownerUuid.get() != NULL ? (rhs.ownerUuid.get() == NULL || *ownerUuid != *rhs.ownerUuid) :
                    ownerUuid.get() != NULL) {
                    return false;
                }

                if (uuid.get() != NULL ? (rhs.uuid.get() == NULL || *uuid != *rhs.uuid) : rhs.uuid.get() != NULL) {
                    return false;
                }

                return true;
            }

            std::ostream &operator<<(std::ostream &os, const Principal &principal) {
                os << "uuid: " << (principal.uuid.get() ? *principal.uuid : "null") << " ownerUuid: "
                   << (principal.ownerUuid ? *principal.ownerUuid : "null");
                return os;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            ClientMessageBuilder::ClientMessageBuilder(connection::Connection &connection)
                    : connection(connection) {
            }

            ClientMessageBuilder::~ClientMessageBuilder() {
            }

            bool ClientMessageBuilder::onData(util::ByteBuffer &buffer) {
                bool isCompleted = false;

                if (NULL == message.get()) {
                    if (buffer.remaining() >= ClientMessage::HEADER_SIZE) {
                        util::Bits::littleEndianToNative4(
                                ((byte *) buffer.ix()) + ClientMessage::FRAME_LENGTH_FIELD_OFFSET, &frameLen);

                        message = ClientMessage::create(frameLen);
                        offset = 0;
                    }
                }

                if (NULL != message.get()) {
                    offset += message->fillMessageFrom(buffer, offset, frameLen);

                    if (offset == frameLen) {
                        if (message->isFlagSet(ClientMessage::BEGIN_AND_END_FLAGS)) {
                            //MESSAGE IS COMPLETE HERE
                            connection.handleClientMessage(std::move(message));
                            isCompleted = true;
                        } else {
                            if (message->isFlagSet(ClientMessage::BEGIN_FLAG)) {
                                // put the message into the partial messages list
                                addToPartialMessages(message);
                            } else if (message->isFlagSet(ClientMessage::END_FLAG)) {
                                // This is the intermediate frame. Append at the previous message buffer
                                appendExistingPartialMessage(message);
                                isCompleted = true;
                            }
                        }
                    }
                }

                return isCompleted;
            }

            void ClientMessageBuilder::addToPartialMessages(std::unique_ptr<ClientMessage> &message) {
                int64_t id = message->getCorrelationId();
                partialMessages[id] = std::move(message);
            }

            bool ClientMessageBuilder::appendExistingPartialMessage(std::unique_ptr<ClientMessage> &message) {
                bool result = false;

                MessageMap::iterator foundItemIter = partialMessages.find(message->getCorrelationId());
                if (partialMessages.end() != foundItemIter) {
                    foundItemIter->second->append(message.get());
                    if (message->isFlagSet(ClientMessage::END_FLAG)) {
                        // remove from message from map
                        std::shared_ptr<ClientMessage> foundMessage(foundItemIter->second);

                        partialMessages.erase(foundItemIter, foundItemIter);

                        connection.handleClientMessage(foundMessage);

                        result = true;
                    }
                } else {
                    // Should never be here
                    assert(0);
                }

                return result;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace protocol {
            UsernamePasswordCredentials::UsernamePasswordCredentials(const std::string &principal,
                                                                     const std::string &password)
                    : principal(principal), password(password) {
            }

            const std::string &UsernamePasswordCredentials::getPrincipal() const {
                return principal;
            }

            const std::string &UsernamePasswordCredentials::getPassword() const {
                return password;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        ItemEventType::ItemEventType() {
        }

        ItemEventType::ItemEventType(Type value) : value(value) {
        }

        ItemEventType::operator int() const {
            return value;
        }

        /**
         * copy function.
         */
        void ItemEventType::operator=(int i) {
            switch (i) {
                case 1:
                    value = ADDED;
                    break;
                case 2:
                    value = REMOVED;
                    break;
            }
        }

        ItemEventBase::ItemEventBase(const std::string &name, const Member &member, const ItemEventType &eventType)
                : name(name),
                  member(member),
                  eventType(
                          eventType) {}

        Member ItemEventBase::getMember() const {
            return member;
        }

        ItemEventType ItemEventBase::getEventType() const {
            return eventType;
        }

        std::string ItemEventBase::getName() const {
            return name;
        }

        ItemEventBase::~ItemEventBase() {
        }

    }
}

namespace hazelcast {
    namespace client {
        LifecycleListener::~LifecycleListener() {
        }
    }
}

namespace hazelcast {
    namespace client {
        bool IdGenerator::init(int64_t id) {
            return impl->init(id);
        }

        int64_t IdGenerator::newId() {
            return impl->newId();
        }

        IdGenerator::IdGenerator(const std::shared_ptr<impl::IdGeneratorInterface> &impl) : impl(impl) {}

        IdGenerator::~IdGenerator() {
        }
    }
}

namespace hazelcast {
    namespace client {
        MembershipListener::~MembershipListener() {
        }

        const std::string &MembershipListener::getRegistrationId() const {
            return registrationId;
        }

        void MembershipListener::setRegistrationId(const std::string &registrationId) {
            this->registrationId = registrationId;
        }

        bool MembershipListener::shouldRequestInitialMembers() const {
            return false;
        }

        MembershipListenerDelegator::MembershipListenerDelegator(
                MembershipListener *listener) : listener(listener) {}

        void MembershipListenerDelegator::memberAdded(
                const MembershipEvent &membershipEvent) {
            listener->memberAdded(membershipEvent);
        }

        void MembershipListenerDelegator::memberRemoved(
                const MembershipEvent &membershipEvent) {
            listener->memberRemoved(membershipEvent);
        }

        void MembershipListenerDelegator::memberAttributeChanged(
                const MemberAttributeEvent &memberAttributeEvent) {
            listener->memberAttributeChanged(memberAttributeEvent);
        }

        bool MembershipListenerDelegator::shouldRequestInitialMembers() const {
            return listener->shouldRequestInitialMembers();
        }

        void MembershipListenerDelegator::setRegistrationId(const std::string &registrationId) {
            listener->setRegistrationId(registrationId);
        }

        const std::string &MembershipListenerDelegator::getRegistrationId() const {
            return listener->getRegistrationId();
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                AbstractClientInvocationService::AbstractClientInvocationService(ClientContext &client)
                        : CLEAN_RESOURCES_MILLIS(client.getClientProperties().getCleanResourcesPeriodMillis()),
                          client(client), invocationLogger(client.getLogger()),
                          connectionManager(NULL),
                          partitionService(client.getPartitionService()),
                          clientListenerService(NULL),
                          invocationTimeoutMillis(client.getClientProperties().getInteger(
                                  client.getClientProperties().getInvocationTimeoutSeconds()) * 1000),
                          invocationRetryPauseMillis(client.getClientProperties().getLong(
                                  client.getClientProperties().getInvocationRetryPauseMillis())),
                          responseThread(client.getName() + ".response-", invocationLogger, *this, client) {
                }

                bool AbstractClientInvocationService::start() {
                    connectionManager = &client.getConnectionManager();
                    clientListenerService = static_cast<listener::AbstractClientListenerService *>(&client.getClientListenerService());

                    responseThread.start();

                    int64_t cleanResourcesMillis = client.getClientProperties().getLong(CLEAN_RESOURCES_MILLIS);
                    if (cleanResourcesMillis <= 0) {
                        cleanResourcesMillis = util::IOUtil::to_value<int64_t>(
                                CLEAN_RESOURCES_MILLIS.getDefaultValue());
                    }

                    client.getClientExecutionService().scheduleWithRepetition(std::shared_ptr<util::Runnable>(
                            new CleanResourcesTask(invocations)), cleanResourcesMillis, cleanResourcesMillis);

                    return true;
                }

                void AbstractClientInvocationService::shutdown() {
                    isShutdown.store(true);

                    responseThread.shutdown();

                    typedef std::vector<std::pair<int64_t, std::shared_ptr<ClientInvocation> > > InvocationEntriesVector;
                    InvocationEntriesVector allEntries = invocations.clear();
                    std::shared_ptr<exception::HazelcastClientNotActiveException> notActiveException(
                            new exception::HazelcastClientNotActiveException(
                                    "AbstractClientInvocationService::shutdown",
                                    "Client is shutting down"));
                    for (InvocationEntriesVector::value_type &entry : allEntries) {
                        entry.second->notifyException(notActiveException);
                    }
                }

                int64_t AbstractClientInvocationService::getInvocationTimeoutMillis() const {
                    return invocationTimeoutMillis;
                }

                int64_t AbstractClientInvocationService::getInvocationRetryPauseMillis() const {
                    return invocationRetryPauseMillis;
                }

                bool AbstractClientInvocationService::isRedoOperation() {
                    return client.getClientConfig().isRedoOperation();
                }

                void AbstractClientInvocationService::handleClientMessage(
                        const std::shared_ptr<connection::Connection> &connection,
                        const std::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    responseThread.responseQueue.push(clientMessage);
                }

                std::shared_ptr<ClientInvocation> AbstractClientInvocationService::deRegisterCallId(int64_t callId) {
                    return invocations.remove(callId);
                }

                void AbstractClientInvocationService::send(std::shared_ptr<impl::ClientInvocation> invocation,
                                                           std::shared_ptr<connection::Connection> connection) {
                    if (isShutdown) {
                        throw exception::HazelcastClientNotActiveException("AbstractClientInvocationService::send",
                                                                           "Client is shut down");
                    }
                    registerInvocation(invocation);

                    const std::shared_ptr<protocol::ClientMessage> &clientMessage = invocation->getClientMessage();
                    if (!writeToConnection(*connection, clientMessage)) {
                        int64_t callId = clientMessage->getCorrelationId();
                        std::shared_ptr<ClientInvocation> clientInvocation = deRegisterCallId(callId);
                        if (clientInvocation.get() != NULL) {
                            std::ostringstream out;
                            out << "Packet not sent to ";
                            if (connection->getRemoteEndpoint().get()) {
                                out << *connection->getRemoteEndpoint();
                            } else {
                                out << "null";
                            }
                            throw exception::IOException("AbstractClientInvocationService::send", out.str());
                        } else {
                            if (invocationLogger.isFinestEnabled()) {
                                invocationLogger.finest("Invocation not found to deregister for call ID ", callId);
                            }
                            return;
                        }
                    }

                    invocation->setSendConnection(connection);
                }

                void AbstractClientInvocationService::registerInvocation(
                        const std::shared_ptr<ClientInvocation> &clientInvocation) {
                    const std::shared_ptr<protocol::ClientMessage> &clientMessage = clientInvocation->getClientMessage();
                    int64_t correlationId = clientMessage->getCorrelationId();
                    invocations.put(correlationId, clientInvocation);
                    const std::shared_ptr<
                    EventHandler < protocol::ClientMessage > > handler = clientInvocation->getEventHandler();
                    if (handler.get() != NULL) {
                        clientListenerService->addEventHandler(correlationId, handler);
                    }
                }

                bool AbstractClientInvocationService::writeToConnection(connection::Connection &connection,
                                                                        const std::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    clientMessage->addFlag(protocol::ClientMessage::BEGIN_AND_END_FLAGS);
                    return connection.write(clientMessage);
                }

                void AbstractClientInvocationService::CleanResourcesTask::run() {
                    std::vector<int64_t> invocationsToBeRemoved;
                    typedef std::vector<std::pair<int64_t, std::shared_ptr<ClientInvocation> > > INVOCATION_ENTRIES;
                    for (const INVOCATION_ENTRIES::value_type &entry : invocations.entrySet()) {
                        int64_t key = entry.first;
                        const std::shared_ptr<ClientInvocation> &invocation = entry.second;
                        std::shared_ptr<connection::Connection> connection = invocation->getSendConnection();
                        if (!connection.get()) {
                            continue;
                        }

                        if (connection->isAlive()) {
                            continue;
                        }

                        invocationsToBeRemoved.push_back(key);

                        notifyException(*invocation, connection);
                    }

                    for (int64_t invocationId : invocationsToBeRemoved) {
                        invocations.remove(invocationId);
                    }
                }

                void AbstractClientInvocationService::CleanResourcesTask::notifyException(ClientInvocation &invocation,
                                                                                          std::shared_ptr<connection::Connection> &connection) {
                    std::shared_ptr<exception::IException> ex(
                            new exception::TargetDisconnectedException("CleanResourcesTask::notifyException",
                                                                       connection->getCloseReason()));
                    invocation.notifyException(ex);
                }

                AbstractClientInvocationService::CleanResourcesTask::CleanResourcesTask(
                        util::SynchronizedMap<int64_t, ClientInvocation> &invocations) : invocations(invocations) {}

                const std::string AbstractClientInvocationService::CleanResourcesTask::getName() const {
                    return "AbstractClientInvocationService::CleanResourcesTask";
                }

                AbstractClientInvocationService::~AbstractClientInvocationService() {
                }

                AbstractClientInvocationService::ResponseThread::ResponseThread(const std::string &name,
                                                                                util::ILogger &invocationLogger,
                                                                                AbstractClientInvocationService &invocationService,
                                                                                ClientContext &clientContext)
                        : responseQueue(100000), invocationLogger(invocationLogger),
                          invocationService(invocationService), client(clientContext),
                          worker(std::shared_ptr<util::Runnable>(new util::RunnableDelegator(*this)),
                                 invocationLogger) {
                }

                void AbstractClientInvocationService::ResponseThread::run() {
                    try {
                        doRun();
                    } catch (exception::IException &t) {
                        invocationLogger.severe(t);
                    }
                }

                void AbstractClientInvocationService::ResponseThread::doRun() {
                    while (!invocationService.isShutdown) {
                        std::shared_ptr<protocol::ClientMessage> task;
                        try {
                            task = responseQueue.pop();
                        } catch (exception::InterruptedException &) {
                            continue;
                        }
                        process(task);
                    }
                }

                void AbstractClientInvocationService::ResponseThread::process(
                        const std::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    try {
                        handleClientMessage(clientMessage);
                    } catch (exception::IException &e) {
                        invocationLogger.severe("Failed to process task: ", clientMessage, " on responseThread: ",
                                                getName(), e);
                    }
                }

                void AbstractClientInvocationService::ResponseThread::handleClientMessage(
                        const std::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    int64_t correlationId = clientMessage->getCorrelationId();

                    std::shared_ptr<ClientInvocation> future = invocationService.deRegisterCallId(correlationId);
                    if (future.get() == NULL) {
                        invocationLogger.warning("No call for callId: ", correlationId, ", response: ", *clientMessage);
                        return;
                    }
                    if (protocol::codec::ErrorCodec::TYPE == clientMessage->getMessageType()) {
                        std::shared_ptr<exception::IException> exception(
                                client.getClientExceptionFactory().createException(
                                        "AbstractClientInvocationService::ResponseThread::handleClientMessage",
                                        *clientMessage));
                        future->notifyException(exception);
                    } else {
                        future->notify(clientMessage);
                    }
                }

                void AbstractClientInvocationService::ResponseThread::shutdown() {
                    do {
                        responseQueue.interrupt();
                    } while (!worker.waitMilliseconds(100));

                    worker.join();
                }

                void AbstractClientInvocationService::ResponseThread::start() {
                    worker.start();
                }

                const std::string AbstractClientInvocationService::ResponseThread::getName() const {
                    return "AbstractClientInvocationService::ResponseThread";
                }

                AbstractClientInvocationService::ResponseThread::~ResponseThread() {
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {
                    AbstractClientListenerService::AbstractClientListenerService(ClientContext &clientContext,
                                                                                 int32_t eventThreadCount,
                                                                                 int32_t eventQueueCapacity)
                            : clientContext(clientContext),
                              serializationService(clientContext.getSerializationService()),
                              logger(clientContext.getLogger()),
                              clientConnectionManager(clientContext.getConnectionManager()),
                              eventExecutor(logger, clientContext.getName() + ".event-", eventThreadCount,
                                            eventQueueCapacity),
                              registrationExecutor(logger, clientContext.getName() + ".eventRegistration-", 1) {
                        AbstractClientInvocationService &invocationService = (AbstractClientInvocationService &) clientContext.getInvocationService();
                        invocationTimeoutMillis = invocationService.getInvocationTimeoutMillis();
                        invocationRetryPauseMillis = invocationService.getInvocationRetryPauseMillis();
                    }

                    AbstractClientListenerService::~AbstractClientListenerService() {
                    }

                    std::string
                    AbstractClientListenerService::registerListener(
                            const std::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                            const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/
                        std::shared_ptr<util::Callable<std::string> > task(
                                new RegisterListenerTask("AbstractClientListenerService::registerListener",
                                                         shared_from_this(), listenerMessageCodec, handler));
                        return *registrationExecutor.submit<std::string>(task)->get();
                    }

                    bool AbstractClientListenerService::deregisterListener(const std::string &registrationId) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/

                        try {
                            std::shared_ptr<util::Future<bool> > future = registrationExecutor.submit(
                                    std::shared_ptr<util::Callable<bool> >(
                                            new DeregisterListenerTask(
                                                    "AbstractClientListenerService::deregisterListener",
                                                    shared_from_this(), registrationId)));

                            return *future->get();
                        } catch (exception::RejectedExecutionException &) {
                            //RejectedExecutionException executor(hence the client) is already shutdown
                            //listeners are cleaned up by the server side. We can ignore the exception and return true safely
                            return true;
                        }
                    }

                    void AbstractClientListenerService::connectionAdded(
                            const std::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        registrationExecutor.execute(
                                std::shared_ptr<util::Runnable>(
                                        new ConnectionAddedTask("AbstractClientListenerService::connectionAdded",
                                                                shared_from_this(), connection)));
                    }

                    void AbstractClientListenerService::connectionRemoved(
                            const std::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/

                        registrationExecutor.execute(
                                std::shared_ptr<util::Runnable>(
                                        new ConnectionRemovedTask("AbstractClientListenerService::connectionRemoved",
                                                                  shared_from_this(), connection)));
                    }

                    void AbstractClientListenerService::addEventHandler(int64_t callId,
                                                                        const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        eventHandlerMap.put(callId, handler);
                    }

                    void AbstractClientListenerService::removeEventHandler(int64_t callId) {
                        eventHandlerMap.remove(callId);
                    }

                    void AbstractClientListenerService::handleClientMessage(
                            const std::shared_ptr<protocol::ClientMessage> &clientMessage,
                            const std::shared_ptr<connection::Connection> &connection) {
                        try {
                            eventExecutor.execute(
                                    std::shared_ptr<util::StripedRunnable>(
                                            new ClientEventProcessor(clientMessage, connection, eventHandlerMap,
                                                                     logger)));
                        } catch (exception::RejectedExecutionException &e) {
                            logger.warning("Event clientMessage could not be handled. ", e);
                        }
                    }

                    void AbstractClientListenerService::shutdown() {
                        clientContext.getClientExecutionService().shutdownExecutor(eventExecutor.getThreadNamePrefix(),
                                                                                   eventExecutor, logger);
                        clientContext.getClientExecutionService().shutdownExecutor(
                                registrationExecutor.getThreadNamePrefix(), registrationExecutor, logger);
                    }

                    void AbstractClientListenerService::start() {
                        registrationExecutor.start();
                        eventExecutor.start();
                        clientConnectionManager.addConnectionListener(shared_from_this());
                    }

                    void AbstractClientListenerService::ClientEventProcessor::run() {
                        int64_t correlationId = clientMessage->getCorrelationId();
                        std::shared_ptr<EventHandler < protocol::ClientMessage> > eventHandler = eventHandlerMap.get(
                                correlationId);
                        if (eventHandler.get() == NULL) {
                            logger.warning("No eventHandler for callId: ", correlationId, ", event: ", *clientMessage);
                            return;
                        }

                        eventHandler->handle(clientMessage);
                    }

                    const std::string AbstractClientListenerService::ClientEventProcessor::getName() const {
                        return "AbstractClientListenerService::ClientEventProcessor";
                    }

                    int32_t AbstractClientListenerService::ClientEventProcessor::getKey() {
                        return clientMessage->getPartitionId();
                    }

                    AbstractClientListenerService::ClientEventProcessor::ClientEventProcessor(
                            const std::shared_ptr<protocol::ClientMessage> &clientMessage,
                            const std::shared_ptr<connection::Connection> &connection,
                            util::SynchronizedMap<int64_t, EventHandler<protocol::ClientMessage> > &eventHandlerMap,
                            util::ILogger &logger)
                            : clientMessage(clientMessage), eventHandlerMap(eventHandlerMap), logger(logger) {
                    }

                    AbstractClientListenerService::ClientEventProcessor::~ClientEventProcessor() {
                    }

                    AbstractClientListenerService::RegisterListenerTask::RegisterListenerTask(
                            const std::string &taskName,
                            const std::shared_ptr<AbstractClientListenerService> &listenerService,
                            const std::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                            const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) : taskName(
                            taskName), listenerService(listenerService), listenerMessageCodec(listenerMessageCodec),
                                                                                                      handler(handler) {}

                    std::shared_ptr<std::string> AbstractClientListenerService::RegisterListenerTask::call() {
                        return std::shared_ptr<std::string>(new std::string(
                                listenerService->registerListenerInternal(listenerMessageCodec, handler)));
                    }

                    const std::string AbstractClientListenerService::RegisterListenerTask::getName() const {
                        return taskName;
                    }

                    AbstractClientListenerService::DeregisterListenerTask::DeregisterListenerTask(
                            const std::string &taskName,
                            const std::shared_ptr<AbstractClientListenerService> &listenerService,
                            const std::string &registrationId) : taskName(taskName), listenerService(listenerService),
                                                                 registrationId(registrationId) {}

                    std::shared_ptr<bool> AbstractClientListenerService::DeregisterListenerTask::call() {
                        return std::shared_ptr<bool>(
                                new bool(listenerService->deregisterListenerInternal(registrationId)));
                    }

                    const std::string AbstractClientListenerService::DeregisterListenerTask::getName() const {
                        return taskName;
                    }

                    AbstractClientListenerService::ConnectionAddedTask::ConnectionAddedTask(const std::string &taskName,
                                                                                            const std::shared_ptr<AbstractClientListenerService> &listenerService,
                                                                                            const std::shared_ptr<connection::Connection> &connection)
                            : taskName(taskName), listenerService(listenerService), connection(connection) {}

                    const std::string AbstractClientListenerService::ConnectionAddedTask::getName() const {
                        return taskName;
                    }

                    void AbstractClientListenerService::ConnectionAddedTask::run() {
                        listenerService->connectionAddedInternal(connection);
                    }


                    AbstractClientListenerService::ConnectionRemovedTask::ConnectionRemovedTask(
                            const std::string &taskName,
                            const std::shared_ptr<AbstractClientListenerService> &listenerService,
                            const std::shared_ptr<connection::Connection> &connection) : taskName(taskName),
                                                                                         listenerService(
                                                                                                 listenerService),
                                                                                         connection(connection) {}

                    const std::string AbstractClientListenerService::ConnectionRemovedTask::getName() const {
                        return taskName;
                    }

                    void AbstractClientListenerService::ConnectionRemovedTask::run() {
                        listenerService->connectionRemovedInternal(connection);
                    }

                    std::string AbstractClientListenerService::registerListenerInternal(
                            const std::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                            const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        std::string userRegistrationId = util::UuidUtil::newUnsecureUuidString();

                        ClientRegistrationKey registrationKey(userRegistrationId, handler, listenerMessageCodec);
                        registrations.put(registrationKey, std::shared_ptr<ConnectionRegistrationsMap>(
                                new ConnectionRegistrationsMap()));
                        for (const std::shared_ptr<connection::Connection> &connection : clientConnectionManager.getActiveConnections()) {
                            try {
                                invoke(registrationKey, connection);
                            } catch (exception::IException &e) {
                                if (connection->isAlive()) {
                                    deregisterListenerInternal(userRegistrationId);
                                    throw (exception::ExceptionBuilder<exception::HazelcastException>(
                                            "AbstractClientListenerService::RegisterListenerTask::call")
                                            << "Listener can not be added " << e).build();
                                }
                            }
                        }
                        return userRegistrationId;
                    }

                    bool
                    AbstractClientListenerService::deregisterListenerInternal(const std::string &userRegistrationId) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        ClientRegistrationKey key(userRegistrationId);
                        std::shared_ptr<ConnectionRegistrationsMap> registrationMap = registrations.get(key);
                        if (registrationMap.get() == NULL) {
                            return false;
                        }
                        bool successful = true;

                        for (ConnectionRegistrationsMap::iterator it = registrationMap->begin();
                             it != registrationMap->end();) {
                            ClientEventRegistration &registration = (*it).second;
                            std::shared_ptr<connection::Connection> subscriber = registration.getSubscriber();
                            try {
                                const std::shared_ptr<ListenerMessageCodec> &listenerMessageCodec = registration.getCodec();
                                const std::string &serverRegistrationId = registration.getServerRegistrationId();
                                std::unique_ptr<protocol::ClientMessage> request = listenerMessageCodec->encodeRemoveRequest(
                                        serverRegistrationId);
                                std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(clientContext,
                                                                                                        request, "",
                                                                                                        subscriber);
                                invocation->invoke()->get();
                                removeEventHandler(registration.getCallId());

                                ConnectionRegistrationsMap::iterator oldEntry = it;
                                ++it;
                                registrationMap->erase(oldEntry);
                            } catch (exception::IException &e) {
                                ++it;

                                if (subscriber->isAlive()) {
                                    successful = false;
                                    std::ostringstream endpoint;
                                    if (subscriber->getRemoteEndpoint().get()) {
                                        endpoint << *subscriber->getRemoteEndpoint();
                                    } else {
                                        endpoint << "null";
                                    }
                                    logger.warning("AbstractClientListenerService::deregisterListenerInternal",
                                                   "Deregistration of listener with ID ", userRegistrationId,
                                                   " has failed to address ", subscriber->getRemoteEndpoint(), e);
                                }
                            }
                        }
                        if (successful) {
                            registrations.remove(key);
                        }
                        return successful;
                    }

                    void AbstractClientListenerService::connectionAddedInternal(
                            const std::shared_ptr<connection::Connection> &connection) {
                        for (const ClientRegistrationKey &registrationKey : registrations.keys()) {
                            invokeFromInternalThread(registrationKey, connection);
                        }
                    }

                    void AbstractClientListenerService::connectionRemovedInternal(
                            const std::shared_ptr<connection::Connection> &connection) {
                        typedef std::vector<std::pair<ClientRegistrationKey, std::shared_ptr<ConnectionRegistrationsMap> > > ENTRY_VECTOR;
                        for (const ENTRY_VECTOR::value_type &registrationMapEntry : registrations.entrySet()) {
                            std::shared_ptr<ConnectionRegistrationsMap> registrationMap = registrationMapEntry.second;
                            ConnectionRegistrationsMap::iterator foundRegistration = registrationMap->find(
                                    connection);
                            if (foundRegistration != registrationMap->end()) {
                                removeEventHandler(foundRegistration->second.getCallId());
                                registrationMap->erase(foundRegistration);
                                registrations.put(registrationMapEntry.first,
                                                  registrationMap);
                            }
                        }
                    }

                    void
                    AbstractClientListenerService::invokeFromInternalThread(
                            const ClientRegistrationKey &registrationKey,
                            const std::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        try {
                            invoke(registrationKey, connection);
                        } catch (exception::IException &e) {
                            logger.warning("Listener ", registrationKey, " can not be added to a new connection: ",
                                           *connection, ", reason: ", e.getMessage());
                        }
                    }

                    void
                    AbstractClientListenerService::invoke(const ClientRegistrationKey &registrationKey,
                                                          const std::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/

                        std::shared_ptr<ConnectionRegistrationsMap> registrationMap = registrations.get(
                                registrationKey);
                        if (registrationMap->find(connection) != registrationMap->end()) {
                            return;
                        }

                        const std::shared_ptr<ListenerMessageCodec> &codec = registrationKey.getCodec();
                        std::unique_ptr<protocol::ClientMessage> request = codec->encodeAddRequest(
                                registersLocalOnly());
                        std::shared_ptr<EventHandler < protocol::ClientMessage> >
                        handler = registrationKey.getHandler();
                        handler->beforeListenerRegister();

                        std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(clientContext,
                                                                                                request, "",
                                                                                                connection);
                        invocation->setEventHandler(handler);

                        std::shared_ptr<protocol::ClientMessage> clientMessage = invocation->invokeUrgent()->get();

                        std::string serverRegistrationId = codec->decodeAddResponse(*clientMessage);
                        handler->onListenerRegister();
                        int64_t correlationId = invocation->getClientMessage()->getCorrelationId();
                        ClientEventRegistration registration(serverRegistrationId, correlationId, connection, codec);

                        (*registrationMap)[connection] = registration;
                    }

                    bool AbstractClientListenerService::ConnectionPointerLessComparator::operator()(
                            const std::shared_ptr<connection::Connection> &lhs,
                            const std::shared_ptr<connection::Connection> &rhs) const {
                        if (lhs == rhs) {
                            return false;
                        }
                        if (!lhs.get()) {
                            return true;
                        }
                        if (!rhs.get()) {
                            return false;
                        }

                        return *lhs < *rhs;
                    }

                }
            }
        }

    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {
                    ClientEventRegistration::ClientEventRegistration(const std::string &serverRegistrationId,
                                                                     int64_t callId,
                                                                     const std::shared_ptr<connection::Connection> &subscriber,
                                                                     const std::shared_ptr<ListenerMessageCodec> &codec)
                            : serverRegistrationId(serverRegistrationId), callId(callId), subscriber(subscriber),
                              codec(codec) {
                    }

                    const std::string &ClientEventRegistration::getServerRegistrationId() const {
                        return serverRegistrationId;
                    }

                    int64_t ClientEventRegistration::getCallId() const {
                        return callId;
                    }

                    const std::shared_ptr<connection::Connection> &ClientEventRegistration::getSubscriber() const {
                        return subscriber;
                    }

                    const std::shared_ptr<ListenerMessageCodec> &ClientEventRegistration::getCodec() const {
                        return codec;
                    }

                    bool ClientEventRegistration::operator==(const ClientEventRegistration &rhs) const {
                        return serverRegistrationId == rhs.serverRegistrationId;
                    }

                    bool ClientEventRegistration::operator!=(const ClientEventRegistration &rhs) const {
                        return !(rhs == *this);
                    }

                    bool ClientEventRegistration::operator<(const ClientEventRegistration &rhs) const {
                        return serverRegistrationId < rhs.serverRegistrationId;
                    }

                    ClientEventRegistration::ClientEventRegistration() {}
                }
            }
        }

    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {
                    SmartClientListenerService::SmartClientListenerService(ClientContext &clientContext,
                                                                           int32_t eventThreadCount,
                                                                           int32_t eventQueueCapacity)
                            : AbstractClientListenerService(clientContext, eventThreadCount, eventQueueCapacity) {
                    }


                    void SmartClientListenerService::start() {
                        AbstractClientListenerService::start();

                        registrationExecutor.scheduleAtFixedRate(
                                std::shared_ptr<util::Runnable>(new AsyncConnectToAllMembersTask(
                                        std::static_pointer_cast<SmartClientListenerService>(shared_from_this()))),
                                1000, 1000);
                    }

                    std::string
                    SmartClientListenerService::registerListener(
                            const std::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                            const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/
                        trySyncConnectToAllMembers();

                        return AbstractClientListenerService::registerListener(listenerMessageCodec, handler);
                    }

                    void SmartClientListenerService::trySyncConnectToAllMembers() {
                        ClientClusterService &clientClusterService = clientContext.getClientClusterService();
                        int64_t startMillis = util::currentTimeMillis();

                        do {
                            Member lastFailedMember;
                            std::shared_ptr<exception::IException> lastException;

                            for (const Member &member : clientClusterService.getMemberList()) {
                                try {
                                    clientConnectionManager.getOrConnect(member.getAddress());
                                } catch (exception::IException &e) {
                                    lastFailedMember = member;
                                    lastException = e.clone();
                                }
                            }

                            if (lastException.get() == NULL) {
                                // successfully connected to all members, break loop.
                                break;
                            }

                            timeOutOrSleepBeforeNextTry(startMillis, lastFailedMember, lastException);

                        } while (clientContext.getLifecycleService().isRunning());
                    }

                    void SmartClientListenerService::timeOutOrSleepBeforeNextTry(int64_t startMillis,
                                                                                 const Member &lastFailedMember,
                                                                                 std::shared_ptr<exception::IException> &lastException) {
                        int64_t nowInMillis = util::currentTimeMillis();
                        int64_t elapsedMillis = nowInMillis - startMillis;
                        bool timedOut = elapsedMillis > invocationTimeoutMillis;

                        if (timedOut) {
                            throwOperationTimeoutException(startMillis, nowInMillis, elapsedMillis, lastFailedMember,
                                                           lastException);
                        } else {
                            sleepBeforeNextTry();
                        }

                    }

                    void
                    SmartClientListenerService::throwOperationTimeoutException(int64_t startMillis, int64_t nowInMillis,
                                                                               int64_t elapsedMillis,
                                                                               const Member &lastFailedMember,
                                                                               std::shared_ptr<exception::IException> &lastException) {
                        throw (exception::ExceptionBuilder<exception::OperationTimeoutException>(
                                "SmartClientListenerService::throwOperationTimeoutException")
                                << "Registering listeners is timed out."
                                << " Last failed member : " << lastFailedMember << ", "
                                << " Current time: " << util::StringUtil::timeToString(nowInMillis) << ", "
                                << " Start time : " << util::StringUtil::timeToString(startMillis) << ", "
                                << " Client invocation timeout : " << invocationTimeoutMillis << " ms, "
                                << " Elapsed time : " << elapsedMillis << " ms. " << *lastException).build();

                    }

                    void SmartClientListenerService::sleepBeforeNextTry() {
                        // TODO: change with interruptible sleep
                        util::sleepmillis(invocationRetryPauseMillis);
                    }

                    bool SmartClientListenerService::registersLocalOnly() const {
                        return true;
                    }

                    void SmartClientListenerService::asyncConnectToAllMembersInternal() {
                        std::vector<Member> memberList = clientContext.getClientClusterService().getMemberList();
                        for (const Member &member : memberList) {
                            try {
                                if (!clientContext.getLifecycleService().isRunning()) {
                                    return;
                                }
                                clientContext.getConnectionManager().getOrTriggerConnect(
                                        member.getAddress());
                            } catch (exception::IOException &) {
                                return;
                            }
                        }

                    }

                    SmartClientListenerService::AsyncConnectToAllMembersTask::AsyncConnectToAllMembersTask(
                            const std::shared_ptr<SmartClientListenerService> &listenerService) : listenerService(
                            listenerService) {}

                    void SmartClientListenerService::AsyncConnectToAllMembersTask::run() {
                        listenerService->asyncConnectToAllMembersInternal();
                    }

                    const std::string SmartClientListenerService::AsyncConnectToAllMembersTask::getName() const {
                        return "SmartClientListenerService::AsyncConnectToAllMembersTask";
                    }
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {
                    ClientRegistrationKey::ClientRegistrationKey(const std::string &userRegistrationId,
                                                                 const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler,
                                                                 const std::shared_ptr<ListenerMessageCodec> &codec)
                            : userRegistrationId(userRegistrationId), handler(handler), codec(codec) {
                    }

                    ClientRegistrationKey::ClientRegistrationKey(const std::string &userRegistrationId)
                            : userRegistrationId(userRegistrationId) {
                    }

                    const std::string &ClientRegistrationKey::getUserRegistrationId() const {
                        return userRegistrationId;
                    }

                    const std::shared_ptr<EventHandler<protocol::ClientMessage> > &
                    ClientRegistrationKey::getHandler() const {
                        return handler;
                    }

                    const std::shared_ptr<ListenerMessageCodec> &ClientRegistrationKey::getCodec() const {
                        return codec;
                    }

                    bool ClientRegistrationKey::operator==(const ClientRegistrationKey &rhs) const {
                        return userRegistrationId == rhs.userRegistrationId;
                    }

                    bool ClientRegistrationKey::operator!=(const ClientRegistrationKey &rhs) const {
                        return !(rhs == *this);
                    }

                    bool ClientRegistrationKey::operator<(const ClientRegistrationKey &rhs) const {
                        return userRegistrationId < rhs.userRegistrationId;
                    }

                    std::ostream &operator<<(std::ostream &os, const ClientRegistrationKey &key) {
                        os << "ClientRegistrationKey{ userRegistrationId='" << key.userRegistrationId + '\'' + '}';
                        return os;
                    }

                    ClientRegistrationKey::ClientRegistrationKey() {}
                }
            }
        }

    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {

                    NonSmartClientListenerService::NonSmartClientListenerService(ClientContext &clientContext,
                                                                                 int32_t eventThreadCount,
                                                                                 int32_t eventQueueCapacity)
                            : AbstractClientListenerService(clientContext, eventThreadCount, eventQueueCapacity) {

                    }

                    bool NonSmartClientListenerService::registersLocalOnly() const {
                        return false;
                    }
                }
            }
        }

    }
}


namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                NonSmartClientInvocationService::NonSmartClientInvocationService(ClientContext &client)
                        : AbstractClientInvocationService(client) {

                }

                void NonSmartClientInvocationService::invokeOnConnection(
                        std::shared_ptr<impl::ClientInvocation> invocation,
                        std::shared_ptr<connection::Connection> connection) {
                    assert(connection.get() != NULL);
                    send(invocation, connection);
                }

                void NonSmartClientInvocationService::invokeOnPartitionOwner(
                        std::shared_ptr<impl::ClientInvocation> invocation, int partitionId) {
                    invocation->getClientMessage()->setPartitionId(partitionId);
                    send(invocation, getOwnerConnection());
                }

                void NonSmartClientInvocationService::invokeOnRandomTarget(
                        std::shared_ptr<impl::ClientInvocation> invocation) {
                    send(invocation, getOwnerConnection());
                }

                void
                NonSmartClientInvocationService::invokeOnTarget(std::shared_ptr<impl::ClientInvocation> invocation,
                                                                const std::shared_ptr<Address> &target) {
                    send(invocation, getOwnerConnection());
                }

                std::shared_ptr<connection::Connection> NonSmartClientInvocationService::getOwnerConnection() {
                    std::shared_ptr<Address> ownerConnectionAddress = connectionManager->getOwnerConnectionAddress();
                    if (ownerConnectionAddress.get() == NULL) {
                        throw exception::IOException("NonSmartClientInvocationService::getOwnerConnection",
                                                     "Owner connection address is not available.");
                    }
                    std::shared_ptr<connection::Connection> ownerConnection = connectionManager->getActiveConnection(
                            *ownerConnectionAddress);
                    if (ownerConnection.get() == NULL) {
                        throw exception::IOException("NonSmartClientInvocationService::getOwnerConnection",
                                                     "Owner connection is not available.");
                    }
                    return ownerConnection;
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                DefaultAddressProvider::DefaultAddressProvider(config::ClientNetworkConfig &networkConfig,
                                                               bool noOtherAddressProviderExist) : networkConfig(
                        networkConfig), noOtherAddressProviderExist(noOtherAddressProviderExist) {
                }

                std::vector<Address> DefaultAddressProvider::loadAddresses() {
                    std::vector<Address> addresses = networkConfig.getAddresses();
                    if (addresses.empty() && noOtherAddressProviderExist) {
                        addresses.push_back(Address("127.0.0.1", 5701));
                    }

                    // TODO Implement AddressHelper to add alternative ports for the same address

                    return addresses;
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                impl::ClientClusterServiceImpl::ClientClusterServiceImpl(hazelcast::client::spi::ClientContext &client)
                        : client(client) {
                }

                std::string ClientClusterServiceImpl::addMembershipListenerWithoutInit(
                        const std::shared_ptr<MembershipListener> &listener) {
                    std::string id = util::UuidUtil::newUnsecureUuidString();
                    listeners.put(id, listener);
                    listener->setRegistrationId(id);
                    return id;
                }

                std::shared_ptr<Member> ClientClusterServiceImpl::getMember(const Address &address) {
                    std::map<Address, std::shared_ptr<Member> > currentMembers = members.get();
                    const std::map<hazelcast::client::Address, std::shared_ptr<hazelcast::client::Member> >::iterator &it = currentMembers.find(
                            address);
                    if (it == currentMembers.end()) {
                        return std::shared_ptr<Member>();
                    }
                    return it->second;
                }

                std::shared_ptr<Member> ClientClusterServiceImpl::getMember(const std::string &uuid) {
                    std::vector<Member> memberList = getMemberList();
                    for (const Member &member : memberList) {
                        if (uuid == member.getUuid()) {
                            return std::shared_ptr<Member>(new Member(member));
                        }
                    }
                    return std::shared_ptr<Member>();
                }

                std::vector<Member> ClientClusterServiceImpl::getMemberList() {
                    typedef std::map<Address, std::shared_ptr<Member> > MemberMap;
                    MemberMap memberMap = members.get();
                    std::vector<Member> memberList;
                    for (const MemberMap::value_type &entry : memberMap) {
                        memberList.push_back(*entry.second);
                    }
                    return memberList;
                }

                void ClientClusterServiceImpl::initMembershipListener(MembershipListener &listener) {
                    if (listener.shouldRequestInitialMembers()) {
                        Cluster &cluster = client.getCluster();
                        std::vector<Member> memberCollection = getMemberList();
                        InitialMembershipEvent event(cluster, std::set<Member>(memberCollection.begin(),
                                                                               memberCollection.end()));
                        ((InitialMembershipListener &) listener).init(event);
                    }
                }

                void ClientClusterServiceImpl::start() {
                    clientMembershipListener.reset(new ClientMembershipListener(client));

                    ClientConfig &config = client.getClientConfig();
                    const std::set<std::shared_ptr<MembershipListener> > &membershipListeners = config.getManagedMembershipListeners();

                    for (const std::shared_ptr<MembershipListener> &listener : membershipListeners) {
                        addMembershipListenerWithoutInit(listener);
                    }
                }

                void ClientClusterServiceImpl::handleMembershipEvent(const MembershipEvent &event) {
                    util::LockGuard guard(initialMembershipListenerMutex);
                    const Member &member = event.getMember();
                    std::map<Address, std::shared_ptr<Member> > newMap = members.get();
                    if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                        newMap[member.getAddress()] = std::shared_ptr<Member>(new Member(member));
                    } else {
                        newMap.erase(member.getAddress());
                    }
                    members = newMap;
                    fireMembershipEvent(event);
                }

                void ClientClusterServiceImpl::fireMembershipEvent(const MembershipEvent &event) {
                    for (const std::shared_ptr<MembershipListener> &listener : listeners.values()) {
                        if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                            listener->memberAdded(event);
                        } else {
                            listener->memberRemoved(event);
                        }
                    }
                }

                void ClientClusterServiceImpl::fireMemberAttributeEvent(const MemberAttributeEvent &event) {
                    for (const std::shared_ptr<MembershipListener> &listener : listeners.values()) {
                        listener->memberAttributeChanged(event);
                    }
                }

                void ClientClusterServiceImpl::handleInitialMembershipEvent(const InitialMembershipEvent &event) {
                    util::LockGuard guard(initialMembershipListenerMutex);
                    const std::vector<Member> &initialMembers = event.getMembers();
                    std::map<Address, std::shared_ptr<Member> > newMap;
                    for (const Member &initialMember : initialMembers) {
                        newMap[initialMember.getAddress()] = std::shared_ptr<Member>(
                                new Member(initialMember));
                    }
                    members.set(newMap);
                    fireInitialMembershipEvent(event);

                }

                void ClientClusterServiceImpl::fireInitialMembershipEvent(const InitialMembershipEvent &event) {
                    for (const std::shared_ptr<MembershipListener> &listener : listeners.values()) {
                        if (listener->shouldRequestInitialMembers()) {
                            ((InitialMembershipListener *) listener.get())->init(event);
                        }
                    }
                }

                void ClientClusterServiceImpl::shutdown() {
                }

                void ClientClusterServiceImpl::listenMembershipEvents(
                        const std::shared_ptr<connection::Connection> &ownerConnection) {
                    clientMembershipListener->listenMembershipEvents(clientMembershipListener, ownerConnection);
                }

                std::string
                ClientClusterServiceImpl::addMembershipListener(const std::shared_ptr<MembershipListener> &listener) {
                    if (listener.get() == NULL) {
                        throw exception::NullPointerException("ClientClusterServiceImpl::addMembershipListener",
                                                              "listener can't be null");
                    }

                    util::LockGuard guard(initialMembershipListenerMutex);
                    std::string id = addMembershipListenerWithoutInit(listener);
                    initMembershipListener(*listener);
                    return id;
                }

                bool ClientClusterServiceImpl::removeMembershipListener(const std::string &registrationId) {
                    return listeners.remove(registrationId).get() != NULL;
                }

                std::vector<Member>
                ClientClusterServiceImpl::getMembers(const cluster::memberselector::MemberSelector &selector) {
                    std::vector<Member> result;
                    for (const Member &member : getMemberList()) {
                        if (selector.select(member)) {
                            result.push_back(member);
                        }
                    }

                    return result;
                }

                int ClientClusterServiceImpl::getSize() {
                    return (int) getMemberList().size();
                }

                Client ClientClusterServiceImpl::getLocalClient() const {
                    connection::ClientConnectionManagerImpl &cm = client.getConnectionManager();
                    std::shared_ptr<connection::Connection> connection = cm.getOwnerConnection();
                    std::shared_ptr<Address> inetSocketAddress =
                            connection.get() != NULL ? std::shared_ptr<Address>(connection->getLocalSocketAddress())
                                                     : std::shared_ptr<Address>();
                    const std::shared_ptr<protocol::Principal> principal = cm.getPrincipal();
                    std::shared_ptr<std::string> uuid =
                            principal.get() != NULL ? std::make_shared<std::string>(*principal->getUuid())
                                                    : std::shared_ptr<std::string>();
                    return Client(uuid, inetSocketAddress, client.getName());
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                SmartClientInvocationService::SmartClientInvocationService(ClientContext &client)
                        : AbstractClientInvocationService(client),
                          loadBalancer(*client.getClientConfig().getLoadBalancer()) {}

                void
                SmartClientInvocationService::invokeOnConnection(std::shared_ptr<impl::ClientInvocation> invocation,
                                                                 std::shared_ptr<connection::Connection> connection) {
                    send(invocation, connection);
                }

                void SmartClientInvocationService::invokeOnPartitionOwner(
                        std::shared_ptr<impl::ClientInvocation> invocation, int partitionId) {
                    std::shared_ptr<Address> owner = partitionService.getPartitionOwner(partitionId);
                    if (owner.get() == NULL) {
                        throw (exception::ExceptionBuilder<exception::IOException>(
                                "SmartClientInvocationService::invokeOnPartitionOwner")
                                << "Partition does not have an owner. partitionId: " << partitionId).build();
                    }
                    if (!isMember(*owner)) {
                        throw (exception::ExceptionBuilder<exception::TargetNotMemberException>(
                                "SmartClientInvocationService::invokeOnPartitionOwner") << "Partition owner '" << *owner
                                                                                        << "' is not a member.").build();
                    }
                    invocation->getClientMessage()->setPartitionId(partitionId);
                    std::shared_ptr<connection::Connection> connection = getOrTriggerConnect(owner);
                    send(invocation, connection);
                }

                void SmartClientInvocationService::invokeOnRandomTarget(
                        std::shared_ptr<impl::ClientInvocation> invocation) {
                    std::shared_ptr<Address> randomAddress = getRandomAddress();
                    if (randomAddress.get() == NULL) {
                        throw (exception::ExceptionBuilder<exception::IOException>(
                                "SmartClientInvocationService::invokeOnRandomTarget")
                                << "No address found to invoke").build();
                    }
                    std::shared_ptr<connection::Connection> connection = getOrTriggerConnect(randomAddress);
                    send(invocation, connection);
                }

                void SmartClientInvocationService::invokeOnTarget(std::shared_ptr<impl::ClientInvocation> invocation,
                                                                  const std::shared_ptr<Address> &target) {
                    if (!isMember(*target)) {
                        throw (exception::ExceptionBuilder<exception::TargetNotMemberException>(
                                "SmartClientInvocationService::invokeOnTarget")
                                << "Target '" << *target << "' is not a member.").build();
                    }
                    std::shared_ptr<connection::Connection> connection = getOrTriggerConnect(target);
                    invokeOnConnection(invocation, connection);

                }

                bool SmartClientInvocationService::isMember(const Address &target) const {
                    std::shared_ptr<Member> member = client.getClientClusterService().getMember(target);
                    return member.get() != NULL;
                }

                std::shared_ptr<connection::Connection>
                SmartClientInvocationService::getOrTriggerConnect(const std::shared_ptr<Address> &target) const {
                    std::shared_ptr<connection::Connection> connection = connectionManager->getOrTriggerConnect(
                            *target);
                    if (connection.get() == NULL) {
                        throw (exception::ExceptionBuilder<exception::IOException>(
                                "SmartClientInvocationService::getOrTriggerConnect")
                                << "No available connection to address " << target).build();
                    }
                    return connection;
                }

                std::shared_ptr<Address> SmartClientInvocationService::getRandomAddress() {
                    // TODO: Change loadBalancer to return pointer as in Java so that it is possible to denote the
                    // case where no address can be provided
                    std::shared_ptr<Address> address;
                    try {
                        Member member = loadBalancer.next();
                        address = std::shared_ptr<Address>(new Address(member.getAddress()));
                    } catch (exception::IllegalStateException &) {
                        // do nothing, there is no available server
                    }
                    return address;
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                int ClientMembershipListener::INITIAL_MEMBERS_TIMEOUT_SECONDS = 5;

                ClientMembershipListener::ClientMembershipListener(ClientContext &client)
                        : client(client), logger(client.getLogger()),
                          clusterService(static_cast<ClientClusterServiceImpl &>(client.getClientClusterService())),
                          partitionService((ClientPartitionServiceImpl &) client.getPartitionService()),
                          connectionManager(client.getConnectionManager()) {}

                void ClientMembershipListener::handleMemberEventV10(const Member &member, const int32_t &eventType) {
                    switch (eventType) {
                        case MembershipEvent::MEMBER_ADDED:
                            memberAdded(member);
                            break;
                        case MembershipEvent::MEMBER_REMOVED:
                            memberRemoved(member);
                            break;
                        default:
                            logger.warning("Unknown event type: ", eventType);
                    }
                    partitionService.refreshPartitions();
                }

                void ClientMembershipListener::handleMemberListEventV10(const std::vector<Member> &initialMembers) {
                    std::map<std::string, Member> prevMembers;
                    if (!members.empty()) {
                        for (const Member &member : members) {
                            prevMembers[member.getUuid()] = member;
                        }
                        members.clear();
                    }

                    for (const Member &initialMember : initialMembers) {
                        members.insert(initialMember);
                    }

                    if (prevMembers.empty()) {
                        //this means this is the first time client connected to server
                        logger.info(membersString());
                        clusterService.handleInitialMembershipEvent(
                                InitialMembershipEvent(client.getCluster(), members));
                        initialListFetchedLatch.get()->countDown();
                        return;
                    }

                    std::vector<MembershipEvent> events = detectMembershipEvents(prevMembers);
                    logger.info(membersString());
                    fireMembershipEvent(events);
                    initialListFetchedLatch.get()->countDown();
                }

                void
                ClientMembershipListener::handleMemberAttributeChangeEventV10(const std::string &uuid,
                                                                              const std::string &key,
                                                                              const int32_t &operationType,
                                                                              std::unique_ptr<std::string> &value) {
                    std::vector<Member> members = clusterService.getMemberList();
                    for (Member &target : members) {
                        if (target.getUuid() == uuid) {
                            Member::MemberAttributeOperationType type = (Member::MemberAttributeOperationType) operationType;
                            target.updateAttribute(type, key, value);
                            MemberAttributeEvent memberAttributeEvent(client.getCluster(), target,
                                                                      (MemberAttributeEvent::MemberAttributeOperationType) type,
                                                                      key, value.get() ? (*value) : "");
                            clusterService.fireMemberAttributeEvent(memberAttributeEvent);
                            break;
                        }
                    }

                }

                void ClientMembershipListener::memberAdded(const Member &member) {
                    members.insert(member);
                    logger.info(membersString());
                    MembershipEvent event(client.getCluster(), member, MembershipEvent::MEMBER_ADDED,
                                          std::vector<Member>(members.begin(), members.end()));
                    clusterService.handleMembershipEvent(event);
                }

                std::string ClientMembershipListener::membersString() const {
                    std::stringstream out;
                    out << std::endl << std::endl << "Members [" << members.size() << "]  {";

                    for (const Member &member : members) {
                        out << std::endl << "\t" << member;
                    }
                    out << std::endl << "}" << std::endl;

                    return out.str();
                }

                void ClientMembershipListener::memberRemoved(const Member &member) {
                    members.erase(member);
                    logger.info(membersString());
                    std::shared_ptr<connection::Connection> connection = connectionManager.getActiveConnection(
                            member.getAddress());
                    if (connection.get() != NULL) {
                        connection->close("", newTargetDisconnectedExceptionCausedByMemberLeftEvent(connection));
                    }
                    MembershipEvent event(client.getCluster(), member, MembershipEvent::MEMBER_REMOVED,
                                          std::vector<Member>(members.begin(), members.end()));
                    clusterService.handleMembershipEvent(event);
                }

                std::shared_ptr<exception::IException>
                ClientMembershipListener::newTargetDisconnectedExceptionCausedByMemberLeftEvent(
                        const std::shared_ptr<connection::Connection> &connection) {
                    return (exception::ExceptionBuilder<exception::TargetDisconnectedException>(
                            "ClientMembershipListener::newTargetDisconnectedExceptionCausedByMemberLeftEvent")
                            << "The client has closed the connection to this member, after receiving a member left event from the cluster. "
                            << *connection).buildShared();
                }

                std::vector<MembershipEvent>
                ClientMembershipListener::detectMembershipEvents(std::map<std::string, Member> &prevMembers) {
                    std::vector<MembershipEvent> events;

                    const std::set<Member> &eventMembers = members;

                    std::vector<Member> newMembers;
                    for (const Member &member : members) {
                        std::map<std::string, Member>::iterator formerEntry = prevMembers.find(
                                member.getUuid());
                        if (formerEntry != prevMembers.end()) {
                            prevMembers.erase(formerEntry);
                        } else {
                            newMembers.push_back(member);
                        }
                    }

                    // removal events should be added before added events
                    typedef const std::map<std::string, Member> MemberMap;
                    for (const MemberMap::value_type &member : prevMembers) {
                        events.push_back(MembershipEvent(client.getCluster(), member.second,
                                                         MembershipEvent::MEMBER_REMOVED,
                                                         std::vector<Member>(eventMembers.begin(),
                                                                             eventMembers.end())));
                        const Address &address = member.second.getAddress();
                        if (clusterService.getMember(address).get() == NULL) {
                            std::shared_ptr<connection::Connection> connection = connectionManager.getActiveConnection(
                                    address);
                            if (connection.get() != NULL) {
                                connection->close("",
                                                  newTargetDisconnectedExceptionCausedByMemberLeftEvent(
                                                          connection));
                            }
                        }
                    }
                    for (const Member &member : newMembers) {
                        events.push_back(
                                MembershipEvent(client.getCluster(), member, MembershipEvent::MEMBER_ADDED,
                                                std::vector<Member>(eventMembers.begin(),
                                                                    eventMembers.end())));
                    }

                    return events;
                }

                void ClientMembershipListener::fireMembershipEvent(std::vector<MembershipEvent> &events) {
                    for (const MembershipEvent &event : events) {
                        clusterService.handleMembershipEvent(event);
                    }
                }

                void
                ClientMembershipListener::listenMembershipEvents(
                        const std::shared_ptr<ClientMembershipListener> &listener,
                        const std::shared_ptr<connection::Connection> &ownerConnection) {
                    listener->initialListFetchedLatch = std::shared_ptr<util::CountDownLatch>(
                            new util::CountDownLatch(1));
                    std::unique_ptr<protocol::ClientMessage> clientMessage = protocol::codec::ClientAddMembershipListenerCodec::encodeRequest(
                            false);
                    std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(listener->client,
                                                                                            clientMessage, "",
                                                                                            ownerConnection);
                    invocation->setEventHandler(listener);
                    invocation->invokeUrgent()->get();
                    listener->waitInitialMemberListFetched();
                }

                void ClientMembershipListener::waitInitialMemberListFetched() {
                    bool success = initialListFetchedLatch.get()->await(INITIAL_MEMBERS_TIMEOUT_SECONDS);
                    if (!success) {
                        logger.warning("Error while getting initial member list from cluster!");
                    }
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                const int ClientExecutionServiceImpl::SHUTDOWN_CHECK_INTERVAL_SECONDS = 30;

                ClientExecutionServiceImpl::ClientExecutionServiceImpl(const std::string &name,
                                                                       const ClientProperties &clientProperties,
                                                                       int32_t poolSize, util::ILogger &logger)
                        : logger(logger) {

                    int internalPoolSize = clientProperties.getInteger(clientProperties.getInternalExecutorPoolSize());
                    if (internalPoolSize <= 0) {
                        internalPoolSize = util::IOUtil::to_value<int>(
                                ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT);
                    }

                    int32_t executorPoolSize = poolSize;
                    if (executorPoolSize <= 0) {
                        executorPoolSize = util::RuntimeAvailableProcessors::get();
                    }
                    if (executorPoolSize <= 0) {
                        executorPoolSize = 4; // hard coded thread pool count in case we could not get the processor count
                    }

                    internalExecutor.reset(
                            new util::impl::SimpleExecutorService(logger, name + ".internal-", internalPoolSize,
                                                                  INT32_MAX));

                    userExecutor.reset(
                            new util::impl::SimpleExecutorService(logger, name + ".user-", executorPoolSize,
                                                                  INT32_MAX));
                }

                void ClientExecutionServiceImpl::execute(const std::shared_ptr<util::Runnable> &command) {
                    internalExecutor->execute(command);
                }

                void ClientExecutionServiceImpl::start() {
                    userExecutor->start();
                    internalExecutor->start();
                }

                void ClientExecutionServiceImpl::shutdown() {
                    shutdownExecutor("user", *userExecutor, logger);
                    shutdownExecutor("internal", *internalExecutor, logger);
                }

                void
                ClientExecutionServiceImpl::shutdownExecutor(const std::string &name, util::ExecutorService &executor,
                                                             util::ILogger &logger) {
                    try {
                        int64_t startTimeMilliseconds = util::currentTimeMillis();
                        bool success = false;
                        // Wait indefinitely until the threads gracefully shutdown an log the problem periodically.
                        while (!success) {
                            int64_t waitTimeMillis = 100;
                            auto intervalStartTimeMillis = util::currentTimeMillis();
                            while (!success && util::currentTimeMillis() - intervalStartTimeMillis <
                                               1000 * SHUTDOWN_CHECK_INTERVAL_SECONDS) {
                                executor.shutdown();
                                auto &executorService = static_cast<util::impl::SimpleExecutorService &>(executor);
                                success = executorService.awaitTerminationMilliseconds(waitTimeMillis);
                            }

                            if (!success) {
                                logger.warning(name, " executor awaitTermination could not be completed in ",
                                               (util::currentTimeMillis() - startTimeMilliseconds), " msecs.");
                            }
                        }
                    } catch (exception::InterruptedException &e) {
                        logger.warning(name, " executor await termination is interrupted. ", e);
                    }
                }

                void
                ClientExecutionServiceImpl::scheduleWithRepetition(const std::shared_ptr<util::Runnable> &command,
                                                                   int64_t initialDelayInMillis,
                                                                   int64_t periodInMillis) {
                    internalExecutor->scheduleAtFixedRate(command, initialDelayInMillis, periodInMillis);
                }

                void ClientExecutionServiceImpl::schedule(const std::shared_ptr<util::Runnable> &command,
                                                          int64_t initialDelayInMillis) {
                    internalExecutor->schedule(command, initialDelayInMillis);
                }

                const std::shared_ptr<util::ExecutorService> ClientExecutionServiceImpl::getUserExecutor() const {
                    return userExecutor;
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName,
                                                   int partitionId) :
                        logger(clientContext.getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService().shared_from_this()),
                        clientMessage(std::shared_ptr<protocol::ClientMessage>(std::move(clientMessage))),
                        callIdSequence(clientContext.getCallIdSequence()),
                        partitionId(partitionId),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        invokeCount(0),
                        clientInvocationFuture(
                                new ClientInvocationFuture(clientContext.getClientExecutionService().shared_from_this(),
                                                           clientContext.getLogger(), this->clientMessage,
                                                           clientContext.getCallIdSequence())) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName,
                                                   const std::shared_ptr<connection::Connection> &connection) :
                        logger(clientContext.getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService().shared_from_this()),
                        clientMessage(std::shared_ptr<protocol::ClientMessage>(std::move(clientMessage))),
                        callIdSequence(clientContext.getCallIdSequence()),
                        partitionId(UNASSIGNED_PARTITION),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        connection(connection),
                        invokeCount(0),
                        clientInvocationFuture(
                                new ClientInvocationFuture(clientContext.getClientExecutionService().shared_from_this(),
                                                           clientContext.getLogger(), std::move(clientMessage),
                                                           clientContext.getCallIdSequence())) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName) :
                        logger(clientContext.getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService().shared_from_this()),
                        clientMessage(std::shared_ptr<protocol::ClientMessage>(std::move(clientMessage))),
                        callIdSequence(clientContext.getCallIdSequence()),
                        partitionId(UNASSIGNED_PARTITION),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        invokeCount(0),
                        clientInvocationFuture(
                                new ClientInvocationFuture(clientContext.getClientExecutionService().shared_from_this(),
                                                           clientContext.getLogger(), this->clientMessage,
                                                           clientContext.getCallIdSequence())) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName, const Address &address) :
                        logger(clientContext.getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService().shared_from_this()),
                        clientMessage(std::shared_ptr<protocol::ClientMessage>(std::move(clientMessage))),
                        callIdSequence(clientContext.getCallIdSequence()),
                        address(new Address(address)),
                        partitionId(UNASSIGNED_PARTITION),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        invokeCount(0),
                        clientInvocationFuture(
                                new ClientInvocationFuture(clientContext.getClientExecutionService().shared_from_this(),
                                                           clientContext.getLogger(), this->clientMessage,
                                                           clientContext.getCallIdSequence())) {
                }

                ClientInvocation::~ClientInvocation() {
                }

                std::shared_ptr<ClientInvocationFuture> ClientInvocation::invoke() {
                    assert (clientMessage.get() != NULL);
                    clientMessage.get()->setCorrelationId(callIdSequence->next());
                    invokeOnSelection(shared_from_this());
                    return clientInvocationFuture;
                }

                std::shared_ptr<ClientInvocationFuture> ClientInvocation::invokeUrgent() {
                    assert (clientMessage.get() != NULL);
                    clientMessage.get()->setCorrelationId(callIdSequence->forceNext());
                    invokeOnSelection(shared_from_this());
                    return clientInvocationFuture;
                }

                void ClientInvocation::invokeOnSelection(const std::shared_ptr<ClientInvocation> &invocation) {
                    invocation->invokeCount++;
                    try {
                        if (invocation->isBindToSingleConnection()) {
                            invocation->invocationService.invokeOnConnection(invocation, invocation->connection);
                        } else if (invocation->partitionId != UNASSIGNED_PARTITION) {
                            invocation->invocationService.invokeOnPartitionOwner(invocation, invocation->partitionId);
                        } else if (invocation->address.get() != NULL) {
                            invocation->invocationService.invokeOnTarget(invocation, invocation->address);
                        } else {
                            invocation->invocationService.invokeOnRandomTarget(invocation);
                        }
                    } catch (exception::HazelcastOverloadException &) {
                        throw;
                    } catch (exception::IException &e) {
                        invocation->notifyException(std::shared_ptr<exception::IException>(e.clone()));
                    }
                }

                bool ClientInvocation::isBindToSingleConnection() const {
                    return connection.get() != NULL;
                }

                void ClientInvocation::run() {
                    retry();
                }

                void ClientInvocation::retry() {
                    // retry modifies the client message and should not reuse the client message.
                    // It could be the case that it is in write queue of the connection.
                    clientMessage = copyMessage();
                    // first we force a new invocation slot because we are going to return our old invocation slot immediately after
                    // It is important that we first 'force' taking a new slot; otherwise it could be that a sneaky invocation gets
                    // through that takes our slot!
                    clientMessage.get()->setCorrelationId(callIdSequence->forceNext());
                    //we release the old slot
                    callIdSequence->complete();

                    try {
                        invokeOnSelection(shared_from_this());
                    } catch (exception::IException &e) {
                        clientInvocationFuture->complete(std::shared_ptr<exception::IException>(e.clone()));
                    }
                }

                void ClientInvocation::notifyException(const std::shared_ptr<exception::IException> &exception) {
                    if (!lifecycleService.isRunning()) {
                        std::shared_ptr<exception::IException> notActiveException(
                                new exception::HazelcastClientNotActiveException(exception->getSource(),
                                                                                 "Client is shutting down", exception));

                        clientInvocationFuture->complete(notActiveException);
                        return;
                    }

                    if (isNotAllowedToRetryOnSelection(*exception)) {
                        clientInvocationFuture->complete(exception);
                        return;
                    }

                    bool retry = isRetrySafeException(*exception)
                                 || invocationService.isRedoOperation()
                                 || (exception->getErrorCode() == protocol::TARGET_DISCONNECTED &&
                                     clientMessage.get()->isRetryable());

                    if (!retry) {
                        clientInvocationFuture->complete(exception);
                        return;
                    }

                    int64_t timePassed = util::currentTimeMillis() - startTimeMillis;
                    if (timePassed > invocationService.getInvocationTimeoutMillis()) {
                        if (logger.isFinestEnabled()) {
                            std::ostringstream out;
                            out << "Exception will not be retried because invocation timed out. " << exception->what();
                            logger.finest(out.str());
                        }

                        clientInvocationFuture->complete(newOperationTimeoutException(*exception));
                        return;
                    }

                    try {
                        execute();
                    } catch (exception::RejectedExecutionException &) {
                        clientInvocationFuture->complete(exception);
                    }

                }

                bool ClientInvocation::isNotAllowedToRetryOnSelection(exception::IException &exception) {
                    if (isBindToSingleConnection() && exception.getErrorCode() == protocol::IO) {
                        return true;
                    }

                    if (address.get() != NULL && exception.getErrorCode() == protocol::TARGET_NOT_MEMBER &&
                        clientClusterService.getMember(*address).get() == NULL) {
                        //when invocation send over address
                        //if exception is target not member and
                        //address is not available in member list , don't retry
                        return true;
                    }
                    return false;
                }

                bool ClientInvocation::isRetrySafeException(exception::IException &exception) {
                    int32_t errorCode = exception.getErrorCode();
                    if (errorCode == protocol::IO || errorCode == protocol::HAZELCAST_INSTANCE_NOT_ACTIVE ||
                        exception.isRetryable()) {
                        return true;
                    }

                    return false;
                }

                std::shared_ptr<exception::OperationTimeoutException>
                ClientInvocation::newOperationTimeoutException(exception::IException &exception) {
                    int64_t nowInMillis = util::currentTimeMillis();

                    return (exception::ExceptionBuilder<exception::OperationTimeoutException>(
                            "ClientInvocation::newOperationTimeoutException") << *this
                                                                              << " timed out because exception occurred after client invocation timeout "
                                                                              << "Current time :"
                                                                              << invocationService.getInvocationTimeoutMillis()
                                                                              << util::StringUtil::timeToString(
                                                                                      nowInMillis) << ". "
                                                                              << "Start time: "
                                                                              << util::StringUtil::timeToString(
                                                                                      startTimeMillis)
                                                                              << ". Total elapsed time: "
                                                                              << (nowInMillis - startTimeMillis)
                                                                              << " ms. ").buildShared();
                }

                std::ostream &operator<<(std::ostream &os, const ClientInvocation &invocation) {
                    std::ostringstream target;
                    if (invocation.isBindToSingleConnection()) {
                        target << "connection " << *invocation.connection;
                    } else if (invocation.partitionId != -1) {
                        target << "partition " << invocation.partitionId;
                    } else if (invocation.address.get() != NULL) {
                        target << "address " << *invocation.address;
                    } else {
                        target << "random";
                    }
                    ClientInvocation &nonConstInvocation = const_cast<ClientInvocation &>(invocation);
                    os << "ClientInvocation{" << "clientMessage = " << *nonConstInvocation.clientMessage.get()
                       << ", objectName = "
                       << invocation.objectName << ", target = " << target.str() << ", sendConnection = ";
                    std::shared_ptr<connection::Connection> sendConnection = nonConstInvocation.sendConnection.get();
                    if (sendConnection.get()) {
                        os << *sendConnection;
                    } else {
                        os << "null";
                    }
                    os << '}';

                    return os;
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                                           const std::string &objectName,
                                                                           int partitionId) {
                    std::shared_ptr<ClientInvocation> invocation = std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName, partitionId));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                                           const std::string &objectName,
                                                                           const std::shared_ptr<connection::Connection> &connection) {
                    std::shared_ptr<ClientInvocation> invocation = std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName, connection));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }


                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                                           const std::string &objectName,
                                                                           const Address &address) {
                    std::shared_ptr<ClientInvocation> invocation = std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName, address));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                                           const std::string &objectName) {
                    std::shared_ptr<ClientInvocation> invocation = std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }

                std::shared_ptr<connection::Connection> ClientInvocation::getSendConnection() {
                    return sendConnection;
                }

                std::shared_ptr<connection::Connection> ClientInvocation::getSendConnectionOrWait() {
                    while (sendConnection.get().get() == NULL && !clientInvocationFuture->isDone()) {
                        // TODO: Make sleep interruptible
                        util::sleepmillis(retryPauseMillis);
                    }
                    return sendConnection;
                }

                void
                ClientInvocation::setSendConnection(const std::shared_ptr<connection::Connection> &sendConnection) {
                    ClientInvocation::sendConnection = sendConnection;
                }

                void ClientInvocation::notify(const std::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    if (clientMessage.get() == NULL) {
                        throw exception::IllegalArgumentException("response can't be null");
                    }
                    clientInvocationFuture->complete(clientMessage);
                }

                const std::shared_ptr<protocol::ClientMessage> ClientInvocation::getClientMessage() {
                    return clientMessage.get();
                }

                const std::shared_ptr<EventHandler<protocol::ClientMessage> > &
                ClientInvocation::getEventHandler() const {
                    return eventHandler;
                }

                void ClientInvocation::setEventHandler(
                        const std::shared_ptr<EventHandler<protocol::ClientMessage> > &eventHandler) {
                    ClientInvocation::eventHandler = eventHandler;
                }

                void ClientInvocation::execute() {
                    if (invokeCount < MAX_FAST_INVOCATION_COUNT) {
                        // fast retry for the first few invocations
                        executionService->execute(std::shared_ptr<util::Runnable>(shared_from_this()));
                    } else {
                        // progressive retry delay
                        int64_t delayMillis = util::min<int64_t>(1 << (invokeCount - MAX_FAST_INVOCATION_COUNT),
                                                                 retryPauseMillis);
                        executionService->schedule(shared_from_this(), delayMillis);
                    }
                }

                const std::string ClientInvocation::getName() const {
                    return "ClientInvocation";
                }

                std::shared_ptr<protocol::ClientMessage> ClientInvocation::copyMessage() {
                    return std::shared_ptr<protocol::ClientMessage>(new protocol::ClientMessage(*clientMessage.get()));
                }

                std::shared_ptr<util::Executor> ClientInvocation::getUserExecutor() {
                    return executionService->getUserExecutor();
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace sequence {

                    std::unique_ptr<CallIdSequence> CallIdFactory::newCallIdSequence(bool isBackPressureEnabled,
                                                                                     int32_t maxAllowedConcurrentInvocations,
                                                                                     int64_t backoffTimeoutMs) {
                        if (!isBackPressureEnabled) {
                            return std::unique_ptr<CallIdSequence>(new CallIdSequenceWithoutBackpressure());
                        } else if (backoffTimeoutMs <= 0) {
                            return std::unique_ptr<CallIdSequence>(
                                    new FailFastCallIdSequence(maxAllowedConcurrentInvocations));
                        } else {
                            return std::unique_ptr<CallIdSequence>(
                                    new CallIdSequenceWithBackpressure(maxAllowedConcurrentInvocations,
                                                                       backoffTimeoutMs));
                        }
                    }
                }
            }
        }

    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace sequence {

                    CallIdSequenceWithoutBackpressure::CallIdSequenceWithoutBackpressure() : head(0) {}

                    CallIdSequenceWithoutBackpressure::~CallIdSequenceWithoutBackpressure() {
                    }

                    int32_t CallIdSequenceWithoutBackpressure::getMaxConcurrentInvocations() const {
                        return INT32_MAX;
                    }

                    int64_t CallIdSequenceWithoutBackpressure::next() {
                        return forceNext();
                    }

                    int64_t CallIdSequenceWithoutBackpressure::forceNext() {
                        return ++head;
                    }

                    void CallIdSequenceWithoutBackpressure::complete() {
                        // no-op
                    }

                    int64_t CallIdSequenceWithoutBackpressure::getLastCallId() {
                        return head;
                    }
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace sequence {
                    AbstractCallIdSequence::AbstractCallIdSequence(int32_t maxConcurrentInvocations) : longs(
                            3 * util::Bits::CACHE_LINE_LENGTH / util::Bits::LONG_SIZE_IN_BYTES) {
                        std::ostringstream out;
                        out << "maxConcurrentInvocations should be a positive number. maxConcurrentInvocations="
                            << maxConcurrentInvocations;
                        util::Preconditions::checkPositive(maxConcurrentInvocations, out.str());

                        this->maxConcurrentInvocations = maxConcurrentInvocations;
                    }

                    AbstractCallIdSequence::~AbstractCallIdSequence() {
                    }

                    int32_t AbstractCallIdSequence::getMaxConcurrentInvocations() const {
                        return maxConcurrentInvocations;
                    }

                    int64_t AbstractCallIdSequence::next() {
                        if (!hasSpace()) {
                            handleNoSpaceLeft();
                        }
                        return forceNext();
                    }

                    int64_t AbstractCallIdSequence::forceNext() {
                        return longs.incrementAndGet(INDEX_HEAD);
                    }

                    void AbstractCallIdSequence::complete() {
                        int64_t newTail = longs.incrementAndGet(INDEX_TAIL);
                        assert(newTail <= longs.get(INDEX_HEAD));
                    }

                    int64_t AbstractCallIdSequence::getLastCallId() {
                        return longs.get(INDEX_HEAD);
                    }

                    bool AbstractCallIdSequence::hasSpace() {
                        return longs.get(INDEX_HEAD) - longs.get(INDEX_TAIL) < maxConcurrentInvocations;
                    }

                    int64_t AbstractCallIdSequence::getTail() {
                        return longs.get(INDEX_TAIL);
                    }
                }
            }
        }

    }
}


namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace sequence {
                    const std::unique_ptr<util::concurrent::IdleStrategy> CallIdSequenceWithBackpressure::IDLER(
                            new util::concurrent::BackoffIdleStrategy(
                                    0, 0, std::chrono::duration_cast<std::chrono::nanoseconds>(
                                            std::chrono::microseconds(1000)).count(),
                                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                                            std::chrono::microseconds(MAX_DELAY_MS * 1000)).count()));

                    CallIdSequenceWithBackpressure::CallIdSequenceWithBackpressure(int32_t maxConcurrentInvocations,
                                                                                   int64_t backoffTimeoutMs)
                            : AbstractCallIdSequence(maxConcurrentInvocations) {
                        std::ostringstream out;
                        out << "backoffTimeoutMs should be a positive number. backoffTimeoutMs=" << backoffTimeoutMs;
                        util::Preconditions::checkPositive(backoffTimeoutMs, out.str());

                        backoffTimeoutNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                std::chrono::milliseconds(backoffTimeoutMs)).count();
                    }

                    void CallIdSequenceWithBackpressure::handleNoSpaceLeft() {
                        auto start = std::chrono::system_clock::now();
                        for (int64_t idleCount = 0;; idleCount++) {
                            int64_t elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                    std::chrono::system_clock::now() - start).count();
                            if (elapsedNanos > backoffTimeoutNanos) {
                                throw (exception::ExceptionBuilder<exception::HazelcastOverloadException>(
                                        "CallIdSequenceWithBackpressure::handleNoSpaceLeft")
                                        << "Timed out trying to acquire another call ID."
                                        << " maxConcurrentInvocations = " << getMaxConcurrentInvocations()
                                        << ", backoffTimeout = "
                                        << std::chrono::microseconds(backoffTimeoutNanos / 1000).count()
                                        << " msecs, elapsed:"
                                        << std::chrono::microseconds(elapsedNanos / 1000).count() << " msecs").build();
                            }
                            IDLER->idle(idleCount);
                            if (hasSpace()) {
                                return;
                            }

                        }
                    }
                }
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace sequence {

                    FailFastCallIdSequence::FailFastCallIdSequence(int32_t maxConcurrentInvocations)
                            : AbstractCallIdSequence(maxConcurrentInvocations) {}

                    void FailFastCallIdSequence::handleNoSpaceLeft() {
                        throw (exception::ExceptionBuilder<exception::HazelcastOverloadException>(
                                "FailFastCallIdSequence::handleNoSpaceLeft")
                                << "Maximum invocation count is reached. maxConcurrentInvocations = "
                                << getMaxConcurrentInvocations()).build();

                    }
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                ClientPartitionServiceImpl::RefreshTaskCallback::RefreshTaskCallback(
                        ClientPartitionServiceImpl &partitionService) : partitionService(partitionService) {}

                void ClientPartitionServiceImpl::RefreshTaskCallback::onResponse(
                        const std::shared_ptr<protocol::ClientMessage> &responseMessage) {
                    if (!responseMessage.get()) {
                        return;
                    }
                    protocol::codec::ClientGetPartitionsCodec::ResponseParameters response =
                            protocol::codec::ClientGetPartitionsCodec::ResponseParameters::decode(*responseMessage);
                    partitionService.processPartitionResponse(response.partitions, response.partitionStateVersion,
                                                              response.partitionStateVersionExist);
                }

                void ClientPartitionServiceImpl::RefreshTaskCallback::onFailure(
                        const std::shared_ptr<exception::IException> &t) {
                    if (partitionService.client.getLifecycleService().isRunning()) {
                        partitionService.logger.warning("Error while fetching cluster partition table! Cause:", *t);
                    }
                }

                ClientPartitionServiceImpl::ClientPartitionServiceImpl(ClientContext &client,
                                                                       hazelcast::client::spi::impl::ClientExecutionServiceImpl &executionService)
                        : client(client), logger(client.getLogger()),
                          clientExecutionService(executionService),
                          refreshTaskCallback(new RefreshTaskCallback(*this)), partitionCount(0),
                          lastPartitionStateVersion(0) {
                }

                bool ClientPartitionServiceImpl::processPartitionResponse(
                        const std::vector<std::pair<Address, std::vector<int32_t> > > &partitions,
                        int32_t partitionStateVersion, bool partitionStateVersionExist) {
                    {
                        util::LockGuard guard(lock);
                        if (!partitionStateVersionExist || partitionStateVersion > lastPartitionStateVersion) {
                            typedef std::vector<std::pair<Address, std::vector<int32_t> > > PARTITION_VECTOR;
                            for (const PARTITION_VECTOR::value_type &entry : partitions) {
                                const Address &address = entry.first;
                                for (const std::vector<int32_t>::value_type &partition : entry.second) {
                                    this->partitions.put(partition, std::shared_ptr<Address>(
                                            new Address(address)));
                                }
                            }
                            partitionCount = this->partitions.size();
                            lastPartitionStateVersion = partitionStateVersion;
                            if (logger.isFinestEnabled()) {
                                logger.finest("Processed partition response. partitionStateVersion : ",
                                              (partitionStateVersionExist ? util::IOUtil::to_string<int32_t>(
                                                      partitionStateVersion) : "NotAvailable"), ", partitionCount :",
                                              (int) partitionCount);
                            }
                        }
                    }
                    return partitionCount > 0;
                }

                void ClientPartitionServiceImpl::start() {
                    //scheduling left in place to support server versions before 3.9.
                    clientExecutionService.scheduleWithRepetition(
                            std::shared_ptr<util::Runnable>(new RefreshTask(client, *this)), INITIAL_DELAY, PERIOD);
                }

                void ClientPartitionServiceImpl::listenPartitionTable(
                        const std::shared_ptr<connection::Connection> &ownerConnection) {
                    //when we connect to cluster back we need to reset partition state version
                    lastPartitionStateVersion = -1;
                    if (ownerConnection->getConnectedServerVersion() >=
                        client::impl::BuildInfo::calculateVersion("3.9")) {
                        //Servers after 3.9 supports listeners
                        std::unique_ptr<protocol::ClientMessage> clientMessage =
                                protocol::codec::ClientAddPartitionListenerCodec::encodeRequest();
                        std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(client, clientMessage,
                                                                                                "", ownerConnection);
                        invocation->setEventHandler(shared_from_this());
                        invocation->invokeUrgent()->get();
                    }
                }

                void ClientPartitionServiceImpl::refreshPartitions() {
                    try {
                        // use internal execution service for all partition refresh process (do not use the user executor thread)
                        clientExecutionService.execute(
                                std::shared_ptr<util::Runnable>(new RefreshTask(client, *this)));
                    } catch (exception::RejectedExecutionException &) {
                        // ignore
                    }
                }

                void ClientPartitionServiceImpl::handlePartitionsEventV15(
                        const std::vector<std::pair<Address, std::vector<int32_t> > > &collection,
                        const int32_t &partitionStateVersion) {
                    processPartitionResponse(collection, partitionStateVersion, true);
                }

                void ClientPartitionServiceImpl::beforeListenerRegister() {
                }

                void ClientPartitionServiceImpl::onListenerRegister() {
                }

                std::shared_ptr<Address> ClientPartitionServiceImpl::getPartitionOwner(int partitionId) {
                    waitForPartitionsFetchedOnce();
                    return partitions.get(partitionId);
                }

                int ClientPartitionServiceImpl::getPartitionId(const serialization::pimpl::Data &key) {
                    int pc = getPartitionCount();
                    if (pc <= 0) {
                        return 0;
                    }
                    int hash = key.getPartitionHash();
                    return util::HashUtil::hashToIndex(hash, pc);
                }

                int ClientPartitionServiceImpl::getPartitionCount() {
                    waitForPartitionsFetchedOnce();
                    return partitionCount;
                }

                std::shared_ptr<client::impl::Partition> ClientPartitionServiceImpl::getPartition(int partitionId) {
                    return std::shared_ptr<client::impl::Partition>(new PartitionImpl(partitionId, client, *this));
                }

                void ClientPartitionServiceImpl::waitForPartitionsFetchedOnce() {
                    while (partitionCount == 0 && client.getConnectionManager().isAlive()) {
                        if (isClusterFormedByOnlyLiteMembers()) {
                            throw exception::NoDataMemberInClusterException(
                                    "ClientPartitionServiceImpl::waitForPartitionsFetchedOnce",
                                    "Partitions can't be assigned since all nodes in the cluster are lite members");
                        }
                        std::unique_ptr<protocol::ClientMessage> requestMessage = protocol::codec::ClientGetPartitionsCodec::encodeRequest();
                        std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(client,
                                                                                                requestMessage, "");
                        std::shared_ptr<ClientInvocationFuture> future = invocation->invokeUrgent();
                        try {
                            std::shared_ptr<protocol::ClientMessage> responseMessage = future->get();
                            protocol::codec::ClientGetPartitionsCodec::ResponseParameters response =
                                    protocol::codec::ClientGetPartitionsCodec::ResponseParameters::decode(
                                            *responseMessage);
                            processPartitionResponse(response.partitions,
                                                     response.partitionStateVersion,
                                                     response.partitionStateVersionExist);
                        } catch (exception::IException &e) {
                            if (client.getLifecycleService().isRunning()) {
                                logger.warning("Error while fetching cluster partition table!", e);
                            }
                        }
                    }
                }

                bool ClientPartitionServiceImpl::isClusterFormedByOnlyLiteMembers() {
                    ClientClusterService &clusterService = client.getClientClusterService();
                    for (const std::vector<Member>::value_type &member : clusterService.getMemberList()) {
                        if (!member.isLiteMember()) {
                            return false;
                        }
                    }
                    return true;
                }

                void ClientPartitionServiceImpl::stop() {
                    partitions.clear();
                }

                void ClientPartitionServiceImpl::RefreshTask::run() {
                    if (!client.getLifecycleService().isRunning()) {
                        return;
                    }

                    try {
                        connection::ClientConnectionManagerImpl &connectionManager = client.getConnectionManager();
                        std::shared_ptr<connection::Connection> connection = connectionManager.getOwnerConnection();
                        if (!connection.get()) {
                            return;
                        }
                        std::unique_ptr<protocol::ClientMessage> requestMessage = protocol::codec::ClientGetPartitionsCodec::encodeRequest();
                        std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(client,
                                                                                                requestMessage, "");
                        std::shared_ptr<ClientInvocationFuture> future = invocation->invokeUrgent();
                        future->andThen(partitionService.refreshTaskCallback);
                    } catch (exception::IException &e) {
                        if (client.getLifecycleService().isRunning()) {
                            partitionService.logger.warning(
                                    std::string("Error while fetching cluster partition table! ") + e.what());
                        }
                    }
                }

                const std::string ClientPartitionServiceImpl::RefreshTask::getName() const {
                    return "ClientPartitionServiceImpl::RefreshTask";
                }

                ClientPartitionServiceImpl::RefreshTask::RefreshTask(ClientContext &client,
                                                                     ClientPartitionServiceImpl &partitionService)
                        : client(client), partitionService(partitionService) {}


                int ClientPartitionServiceImpl::PartitionImpl::getPartitionId() const {
                    return partitionId;
                }

                std::shared_ptr<Member> ClientPartitionServiceImpl::PartitionImpl::getOwner() const {
                    std::shared_ptr<Address> owner = partitionService.getPartitionOwner(partitionId);
                    if (owner.get()) {
                        return client.getClientClusterService().getMember(*owner);
                    }
                    return std::shared_ptr<Member>();
                }

                ClientPartitionServiceImpl::PartitionImpl::PartitionImpl(int partitionId, ClientContext &client,
                                                                         ClientPartitionServiceImpl &partitionService)
                        : partitionId(partitionId), client(client), partitionService(partitionService) {}
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                ClientContext &impl::ClientTransactionManagerServiceImpl::getClient() const {
                    return client;
                }

                ClientTransactionManagerServiceImpl::ClientTransactionManagerServiceImpl(ClientContext &client,
                                                                                         hazelcast::client::LoadBalancer &loadBalancer)
                        : client(client), loadBalancer(loadBalancer) {}

                std::shared_ptr<connection::Connection> ClientTransactionManagerServiceImpl::connect() {
                    AbstractClientInvocationService &invocationService = (AbstractClientInvocationService &) client.getInvocationService();
                    int64_t startTimeMillis = util::currentTimeMillis();
                    int64_t invocationTimeoutMillis = invocationService.getInvocationTimeoutMillis();
                    ClientConfig &clientConfig = client.getClientConfig();
                    bool smartRouting = clientConfig.getNetworkConfig().isSmartRouting();

                    while (client.getLifecycleService().isRunning()) {
                        try {
                            if (smartRouting) {
                                return tryConnectSmart();
                            } else {
                                return tryConnectUnisocket();
                            }
                        } catch (exception::HazelcastClientOfflineException &) {
                            throw;
                        } catch (exception::IException &e) {
                            if (util::currentTimeMillis() - startTimeMillis > invocationTimeoutMillis) {
                                throw newOperationTimeoutException(e, invocationTimeoutMillis, startTimeMillis);
                            }
                        }
                        util::sleepmillis(invocationService.getInvocationRetryPauseMillis());
                    }
                    throw exception::HazelcastClientNotActiveException("ClientTransactionManagerServiceImpl::connect",
                                                                       "Client is shutdown");
                }

                std::shared_ptr<connection::Connection> ClientTransactionManagerServiceImpl::tryConnectSmart() {
                    try {
                        Address address = getRandomAddress();
                        return client.getConnectionManager().getOrConnect(address);
                    } catch (exception::IllegalStateException &) {
                        // loadBalancer.next may throw IllegalStateException when no available member
                        throwException(true);
                    }
                    return std::shared_ptr<connection::Connection>();
                }

                Address ClientTransactionManagerServiceImpl::getRandomAddress() {
                    Member member = loadBalancer.next();
                    return member.getAddress();
                }

                std::shared_ptr<connection::Connection> ClientTransactionManagerServiceImpl::tryConnectUnisocket() {
                    std::shared_ptr<connection::Connection> connection = client.getConnectionManager().getOwnerConnection();

                    if (connection.get()) {
                        return connection;
                    }
                    return throwException(false);
                }

                std::shared_ptr<connection::Connection>
                ClientTransactionManagerServiceImpl::throwException(bool smartRouting) {
                    ClientConfig clientConfig = client.getClientConfig();
                    const config::ClientConnectionStrategyConfig &connectionStrategyConfig = clientConfig.getConnectionStrategyConfig();
                    config::ClientConnectionStrategyConfig::ReconnectMode reconnectMode = connectionStrategyConfig.getReconnectMode();
                    if (reconnectMode == config::ClientConnectionStrategyConfig::ASYNC) {
                        throw exception::HazelcastClientOfflineException(
                                "ClientTransactionManagerServiceImpl::throwException", "Hazelcast client is offline");
                    }
                    if (smartRouting) {
                        std::vector<Member> members = client.getCluster().getMembers();
                        std::ostringstream msg;
                        if (members.empty()) {
                            msg
                                    << "No address was return by the LoadBalancer since there are no members in the cluster";
                        } else {
                            msg << "No address was return by the LoadBalancer. "
                                << "But the cluster contains the following members:{";
                            for (std::vector<Member>::const_iterator it = members.begin();;) {
                                msg << (*it);

                                ++it;

                                if (it != members.end()) {
                                    msg << " , ";
                                } else {
                                    break;
                                }
                            }
                            msg << "}";
                        }
                        throw exception::IllegalStateException("ClientTransactionManagerServiceImpl::throwException",
                                                               msg.str());
                    }
                    throw exception::IllegalStateException("ClientTransactionManagerServiceImpl::throwException",
                                                           "No active connection is found");
                }

                exception::OperationTimeoutException
                ClientTransactionManagerServiceImpl::newOperationTimeoutException(exception::IException &throwable,
                                                                                  int64_t invocationTimeoutMillis,
                                                                                  int64_t startTimeMillis) {
                    std::ostringstream sb;
                    sb
                            << "Creating transaction context timed out because exception occurred after client invocation timeout "
                            << invocationTimeoutMillis << " ms. " << "Current time: "
                            << util::StringUtil::timeToString(util::currentTimeMillis()) << ". " << "Start time: "
                            << util::StringUtil::timeToString(startTimeMillis) << ". Total elapsed time: "
                            << (util::currentTimeMillis() - startTimeMillis) << " ms. ";
                    return exception::OperationTimeoutException(
                            "ClientTransactionManagerServiceImpl::newOperationTimeoutException", sb.str(),
                            std::shared_ptr<exception::IException>(throwable.clone()));

                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                ClientInvocationFuture::ClientInvocationFuture(
                        const std::shared_ptr<Executor> &defaultExecutor, ILogger &logger,
                        const std::shared_ptr<protocol::ClientMessage> &request,
                        const std::shared_ptr<sequence::CallIdSequence> &callIdSequence)
                        : AbstractInvocationFuture<protocol::ClientMessage>(defaultExecutor, logger), request(request),
                          callIdSequence(callIdSequence) {}

                std::string ClientInvocationFuture::invocationToString() const {
                    auto existingInvocation = getInvocation();
                    if (!existingInvocation) {
                        return std::string();
                    }
                    std::ostringstream out;
                    out << existingInvocation;
                    return out.str();
                }

                void ClientInvocationFuture::andThen(
                        const std::shared_ptr<ExecutionCallback<protocol::ClientMessage> > &callback,
                        const std::shared_ptr<Executor> &executor) {
                    AbstractInvocationFuture<protocol::ClientMessage>::andThen(
                            std::shared_ptr<client::ExecutionCallback<protocol::ClientMessage> >(
                                    new InternalDelegatingExecutionCallback(callback, callIdSequence)), executor);
                }

                void ClientInvocationFuture::andThen(
                        const std::shared_ptr<ExecutionCallback<protocol::ClientMessage> > &callback) {
                    AbstractInvocationFuture<protocol::ClientMessage>::andThen(callback);
                }

                void ClientInvocationFuture::onComplete() {
                    callIdSequence->complete();
                }

                std::shared_ptr<protocol::ClientMessage> ClientInvocationFuture::resolveAndThrowIfException(
                        const std::shared_ptr<AbstractInvocationFuture<protocol::ClientMessage>::BaseState> &response) {
                    if (response->getType() == BaseState::Exception) {
                        std::shared_ptr<ExceptionState> exceptionState = std::static_pointer_cast<ExceptionState>(
                                response);
                        std::shared_ptr<IException> exception = exceptionState->getException();
                        int32_t errorCode = exception->getErrorCode();
                        if (errorCode == ExecutionException::ERROR_CODE || errorCode == protocol::CANCELLATION ||
                            errorCode == protocol::INTERRUPTED) {
                            exception->raise();
                        }

                        throw ExecutionException("ClientInvocationFuture::resolveAndThrowIfException",
                                                 "ExecutionException occured.", exception);
                    }
                    std::shared_ptr<AbstractInvocationFuture<protocol::ClientMessage>::ValueState> valueState =
                            std::static_pointer_cast<AbstractInvocationFuture<protocol::ClientMessage>::ValueState>(
                                    response);

                    return valueState->getValue();
                }

                const std::shared_ptr<ClientInvocation> ClientInvocationFuture::getInvocation() const {
                    return invocation.lock();
                }

                void ClientInvocationFuture::setInvocation(
                        const std::weak_ptr<spi::impl::ClientInvocation> &invocation) {
                    this->invocation = invocation;
                }

                ClientInvocationFuture::InternalDelegatingExecutionCallback::InternalDelegatingExecutionCallback(
                        const std::shared_ptr<client::ExecutionCallback<protocol::ClientMessage> > &callback,
                        const std::shared_ptr<sequence::CallIdSequence> &callIdSequence) : callback(callback),
                                                                                           callIdSequence(
                                                                                                   callIdSequence) {
                    this->callIdSequence->forceNext();
                }

                void ClientInvocationFuture::InternalDelegatingExecutionCallback::onResponse(
                        const std::shared_ptr<protocol::ClientMessage> &message) {
                    try {
                        callback->onResponse(message);
                        callIdSequence->complete();
                    } catch (...) {
                        callIdSequence->complete();
                    }
                }

                void ClientInvocationFuture::InternalDelegatingExecutionCallback::onFailure(
                        const std::shared_ptr<exception::IException> &e) {
                    try {
                        callback->onFailure(e);
                        callIdSequence->complete();
                    } catch (...) {
                        callIdSequence->complete();
                    }
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                AwsAddressProvider::AwsAddressProvider(config::ClientAwsConfig &awsConfig, int awsMemberPort,
                                                       util::ILogger &logger) : awsMemberPort(
                        util::IOUtil::to_string<int>(awsMemberPort)), logger(logger), awsClient(awsConfig, logger) {
                }

                std::vector<Address> AwsAddressProvider::loadAddresses() {
                    updateLookupTable();
                    std::map<std::string, std::string> lookupTable = getLookupTable();
                    std::vector<Address> addresses;

                    typedef std::map<std::string, std::string> LookupTable;
                    for (const LookupTable::value_type &privateAddress : lookupTable) {
                        std::vector<Address> possibleAddresses = util::AddressHelper::getSocketAddresses(
                                privateAddress.first + ":" + awsMemberPort, logger);
                        addresses.insert(addresses.begin(), possibleAddresses.begin(),
                                         possibleAddresses.end());
                    }
                    return addresses;
                }

                void AwsAddressProvider::updateLookupTable() {
                    try {
                        privateToPublic = awsClient.getAddresses();
                    } catch (exception::IException &e) {
                        logger.warning("Aws addresses failed to load: ", e.getMessage());
                    }
                }

                std::map<std::string, std::string> AwsAddressProvider::getLookupTable() {
                    return privateToPublic;
                }

                AwsAddressProvider::~AwsAddressProvider() {
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                Address DefaultAddressTranslator::translate(const Address &address) {
                    return address;
                }

                void DefaultAddressTranslator::refresh() {
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            ProxyManager::ProxyManager(ClientContext &context) : client(context) {
            }

            void ProxyManager::init() {
                spi::impl::AbstractClientInvocationService &invocationService =
                        (spi::impl::AbstractClientInvocationService &) client.getInvocationService();

                invocationTimeoutMillis = invocationService.getInvocationTimeoutMillis();
                invocationRetryPauseMillis = invocationService.getInvocationRetryPauseMillis();
            }

            void ProxyManager::destroy() {
                for (const std::shared_ptr<util::Future<ClientProxy> > &future : proxies.values()) {
                    future->get()->onShutdown();
                }
                proxies.clear();
            }

            std::shared_ptr<ClientProxy> ProxyManager::getOrCreateProxy(
                    const std::string &service, const std::string &id, ClientProxyFactory &factory) {
                DefaultObjectNamespace ns(service, id);
                std::shared_ptr<util::Future<ClientProxy> > proxyFuture = proxies.get(ns);
                if (proxyFuture.get() != NULL) {
                    return proxyFuture->get();
                }

                proxyFuture.reset(new util::Future<ClientProxy>(client.getLogger()));
                std::shared_ptr<util::Future<ClientProxy> > current = proxies.putIfAbsent(ns, proxyFuture);
                if (current.get()) {
                    return current->get();
                }

                try {
                    std::shared_ptr<ClientProxy> clientProxy = factory.create(id);
                    initializeWithRetry(clientProxy);
                    proxyFuture->set_value(clientProxy);
                    return clientProxy;
                } catch (exception::IException &e) {
                    proxies.remove(ns);
                    proxyFuture->complete(e);
                    throw;
                }
            }

            void ProxyManager::initializeWithRetry(const std::shared_ptr<ClientProxy> &clientProxy) {
                int64_t startMillis = util::currentTimeMillis();
                while (util::currentTimeMillis() < startMillis + invocationTimeoutMillis) {
                    try {
                        initialize(clientProxy);
                        return;
                    } catch (exception::IException &e) {
                        bool retryable = isRetryable(e);

                        if (!retryable) {
                            try {
                                e.raise();
                            } catch (exception::ExecutionException &) {
                                retryable = isRetryable(*e.getCause());
                            }
                        }

                        if (retryable) {
                            sleepForProxyInitRetry();
                        } else {
                            e.raise();
                        }
                    }
                }
                int64_t elapsedTime = util::currentTimeMillis() - startMillis;
                throw (exception::ExceptionBuilder<exception::OperationTimeoutException>(
                        "ProxyManager::initializeWithRetry") << "Initializing  " << clientProxy->getServiceName() << ":"
                                                             << clientProxy->getName() << " is timed out after "
                                                             << elapsedTime << " ms. Configured invocation timeout is "
                                                             << invocationTimeoutMillis << " ms").build();
            }

            void ProxyManager::initialize(const std::shared_ptr<ClientProxy> &clientProxy) {
                std::shared_ptr<Address> initializationTarget = findNextAddressToSendCreateRequest();
                if (initializationTarget.get() == NULL) {
                    throw exception::IOException("ProxyManager::initialize",
                                                 "Not able to find a member to create proxy on!");
                }
                std::unique_ptr<protocol::ClientMessage> clientMessage = protocol::codec::ClientCreateProxyCodec::encodeRequest(
                        clientProxy->getName(),
                        clientProxy->getServiceName(), *initializationTarget);
                spi::impl::ClientInvocation::create(client, clientMessage, clientProxy->getServiceName(),
                                                    *initializationTarget)->invoke()->get();
                clientProxy->onInitialize();
            }

            std::shared_ptr<Address> ProxyManager::findNextAddressToSendCreateRequest() {
                int clusterSize = client.getClientClusterService().getSize();
                if (clusterSize == 0) {
                    throw exception::HazelcastClientOfflineException("ProxyManager::findNextAddressToSendCreateRequest",
                                                                     "Client connecting to cluster");
                }
                std::shared_ptr<Member> liteMember;

                LoadBalancer *loadBalancer = client.getClientConfig().getLoadBalancer();
                for (int i = 0; i < clusterSize; i++) {
                    std::shared_ptr<Member> member;
                    try {
                        member = std::shared_ptr<Member>(new Member(loadBalancer->next()));
                    } catch (exception::IllegalStateException &) {
                        // skip
                    }
                    if (member.get() != NULL && !member->isLiteMember()) {
                        return std::shared_ptr<Address>(new Address(member->getAddress()));
                    } else if (liteMember.get() == NULL) {
                        liteMember = member;
                    }
                }

                return liteMember.get() != NULL ? std::shared_ptr<Address>(new Address(liteMember->getAddress()))
                                                : std::shared_ptr<Address>();
            }

            bool ProxyManager::isRetryable(exception::IException &exception) {
                return spi::impl::ClientInvocation::isRetrySafeException(exception);
            }

            void ProxyManager::sleepForProxyInitRetry() {
                // TODO: change to interruptible sleep
                util::sleepmillis(invocationRetryPauseMillis);
            }

            void ProxyManager::destroyProxy(ClientProxy &proxy) {
                DefaultObjectNamespace objectNamespace(proxy.getServiceName(), proxy.getName());
                std::shared_ptr<util::Future<ClientProxy> > registeredProxyFuture = proxies.remove(
                        objectNamespace);
                std::shared_ptr<ClientProxy> registeredProxy;
                if (registeredProxyFuture.get()) {
                    registeredProxy = registeredProxyFuture->get();
                }

                try {
                    if (registeredProxy.get() != NULL) {
                        try {
                            registeredProxy->destroyLocally();
                            registeredProxy->destroyRemotely();
                        } catch (exception::IException &e) {
                            registeredProxy->destroyRemotely();
                            e.raise();
                        }
                    }
                    if (&proxy != registeredProxy.get()) {
                        // The given proxy is stale and was already destroyed, but the caller
                        // may have allocated local resources in the context of this stale proxy
                        // instance after it was destroyed, so we have to cleanup it locally one
                        // more time to make sure there are no leaking local resources.
                        proxy.destroyLocally();
                    }
                } catch (exception::IException &e) {
                    if (&proxy != registeredProxy.get()) {
                        // The given proxy is stale and was already destroyed, but the caller
                        // may have allocated local resources in the context of this stale proxy
                        // instance after it was destroyed, so we have to cleanup it locally one
                        // more time to make sure there are no leaking local resources.
                        proxy.destroyLocally();
                    }
                    e.raise();
                }

            }

        }
    }
}


namespace hazelcast {
    namespace client {
        namespace spi {

            ClientContext::ClientContext(const client::HazelcastClient &hazelcastClient) : hazelcastClient(
                    *hazelcastClient.clientImpl) {
            }

            ClientContext::ClientContext(client::impl::HazelcastClientInstanceImpl &hazelcastClient)
                    : hazelcastClient(hazelcastClient) {
            }

            serialization::pimpl::SerializationService &ClientContext::getSerializationService() {
                return hazelcastClient.serializationService;
            }

            ClientClusterService &ClientContext::getClientClusterService() {
                return hazelcastClient.clusterService;
            }

            ClientInvocationService &ClientContext::getInvocationService() {
                return *hazelcastClient.invocationService;
            }

            ClientConfig &ClientContext::getClientConfig() {
                return hazelcastClient.clientConfig;
            }

            ClientPartitionService &ClientContext::getPartitionService() {
                return *hazelcastClient.partitionService;
            }

            LifecycleService &ClientContext::getLifecycleService() {
                return hazelcastClient.lifecycleService;
            }

            ClientListenerService &ClientContext::getClientListenerService() {
                return *hazelcastClient.listenerService;
            }

            connection::ClientConnectionManagerImpl &ClientContext::getConnectionManager() {
                return *hazelcastClient.connectionManager;
            }

            internal::nearcache::NearCacheManager &ClientContext::getNearCacheManager() {
                return *hazelcastClient.nearCacheManager;
            }

            ClientProperties &ClientContext::getClientProperties() {
                return hazelcastClient.clientProperties;
            }

            Cluster &ClientContext::getCluster() {
                return hazelcastClient.cluster;
            }

            std::shared_ptr<impl::sequence::CallIdSequence> &ClientContext::getCallIdSequence() const {
                return hazelcastClient.callIdSequence;
            }

            const protocol::ClientExceptionFactory &ClientContext::getClientExceptionFactory() const {
                return hazelcastClient.getExceptionFactory();
            }

            const std::string &ClientContext::getName() const {
                return hazelcastClient.getName();
            }

            impl::ClientExecutionServiceImpl &ClientContext::getClientExecutionService() const {
                return *hazelcastClient.executionService;
            }

            void ClientContext::onClusterConnect(const std::shared_ptr<connection::Connection> &ownerConnection) {
                hazelcastClient.onClusterConnect(ownerConnection);
            }

            const std::shared_ptr<client::impl::ClientLockReferenceIdGenerator> &
            ClientContext::getLockReferenceIdGenerator() {
                return hazelcastClient.getLockReferenceIdGenerator();
            }

            std::shared_ptr<client::impl::HazelcastClientInstanceImpl>
            ClientContext::getHazelcastClientImplementation() {
                return hazelcastClient.shared_from_this();
            }

            spi::ProxyManager &ClientContext::getProxyManager() {
                return hazelcastClient.getProxyManager();
            }

            util::ILogger &ClientContext::getLogger() {
                return *hazelcastClient.logger;
            }

            client::impl::statistics::Statistics &ClientContext::getClientstatistics() {
                return *hazelcastClient.statistics;
            }
        }

    }
}


namespace hazelcast {
    namespace client {
        namespace spi {

            LifecycleService::LifecycleService(ClientContext &clientContext,
                                               const std::set<LifecycleListener *> &lifecycleListeners,
                                               LoadBalancer *const loadBalancer, Cluster &cluster) : clientContext(
                    clientContext), loadBalancer(loadBalancer), cluster(cluster), shutdownCompletedLatch(1) {
                listeners.insert(lifecycleListeners.begin(), lifecycleListeners.end());
            }

            bool LifecycleService::start() {
                bool expected = false;
                if (!active.compare_exchange_strong(expected, true)) {
                    return false;
                }

                fireLifecycleEvent(LifecycleEvent::STARTED);

                clientContext.getClientExecutionService().start();

                ((spi::impl::listener::AbstractClientListenerService &) clientContext.getClientListenerService()).start();

                if (!((impl::AbstractClientInvocationService &) clientContext.getInvocationService()).start()) {
                    return false;
                }

                ((spi::impl::ClientClusterServiceImpl &) clientContext.getClientClusterService()).start();

                if (!clientContext.getConnectionManager().start()) {
                    return false;
                }

                loadBalancer->init(cluster);

                ((spi::impl::ClientPartitionServiceImpl &) clientContext.getPartitionService()).start();

                clientContext.getClientstatistics().start();

                return true;
            }

            void LifecycleService::shutdown() {
                bool expected = true;
                if (!active.compare_exchange_strong(expected, false)) {
                    shutdownCompletedLatch.await();
                    return;
                }
                fireLifecycleEvent(LifecycleEvent::SHUTTING_DOWN);
                clientContext.getProxyManager().destroy();
                clientContext.getConnectionManager().shutdown();
                ((spi::impl::ClientClusterServiceImpl &) clientContext.getClientClusterService()).shutdown();
                ((spi::impl::AbstractClientInvocationService &) clientContext.getInvocationService()).shutdown();
                ((spi::impl::listener::AbstractClientListenerService &) clientContext.getClientListenerService()).shutdown();
                clientContext.getNearCacheManager().destroyAllNearCaches();
                ((spi::impl::ClientPartitionServiceImpl &) clientContext.getPartitionService()).stop();
                clientContext.getClientExecutionService().shutdown();
                fireLifecycleEvent(LifecycleEvent::SHUTDOWN);
                clientContext.getSerializationService().dispose();
                shutdownCompletedLatch.countDown();
            }

            void LifecycleService::addLifecycleListener(LifecycleListener *lifecycleListener) {
                util::LockGuard lg(listenerLock);
                listeners.insert(lifecycleListener);
            }

            bool LifecycleService::removeLifecycleListener(LifecycleListener *lifecycleListener) {
                util::LockGuard lg(listenerLock);
                return listeners.erase(lifecycleListener) == 1;
            }

            void LifecycleService::fireLifecycleEvent(const LifecycleEvent &lifecycleEvent) {
                util::LockGuard lg(listenerLock);
                util::ILogger &logger = clientContext.getLogger();
                switch (lifecycleEvent.getState()) {
                    case LifecycleEvent::STARTING : {
                        // convert the date string from "2016-04-20" to 20160420
                        std::string date(HAZELCAST_STRINGIZE(HAZELCAST_GIT_COMMIT_DATE));
                        util::gitDateToHazelcastLogDate(date);
                        std::string commitId(HAZELCAST_STRINGIZE(HAZELCAST_GIT_COMMIT_ID));
                        commitId.erase(std::remove(commitId.begin(), commitId.end(), '"'), commitId.end());
                        char msg[100];
                        util::hz_snprintf(msg, 100, "(%s:%s) LifecycleService::LifecycleEvent STARTING", date.c_str(),
                                          commitId.c_str());
                        logger.info(msg);
                        break;
                    }
                    case LifecycleEvent::STARTED :
                        logger.info("LifecycleService::LifecycleEvent STARTED");
                        break;
                    case LifecycleEvent::SHUTTING_DOWN :
                        logger.info("LifecycleService::LifecycleEvent SHUTTING_DOWN");
                        break;
                    case LifecycleEvent::SHUTDOWN :
                        logger.info("LifecycleService::LifecycleEvent SHUTDOWN");
                        break;
                    case LifecycleEvent::CLIENT_CONNECTED :
                        logger.info("LifecycleService::LifecycleEvent CLIENT_CONNECTED");
                        break;
                    case LifecycleEvent::CLIENT_DISCONNECTED :
                        logger.info("LifecycleService::LifecycleEvent CLIENT_DISCONNECTED");
                        break;
                }

                for (std::set<LifecycleListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    (*it)->stateChanged(lifecycleEvent);
                }

            }

            bool LifecycleService::isRunning() {
                return active;
            }

            LifecycleService::~LifecycleService() {
                if (active) {
                    shutdown();
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace spi {
            DefaultObjectNamespace::DefaultObjectNamespace(const std::string &service, const std::string &object)
                    : serviceName(service), objectName(object) {

            }

            const std::string &DefaultObjectNamespace::getServiceName() const {
                return serviceName;
            }

            const std::string &DefaultObjectNamespace::getObjectName() const {
                return objectName;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace spi {
            ClientProxy::ClientProxy(const std::string &name, const std::string &serviceName, ClientContext &context)
                    : name(name), serviceName(serviceName), context(context) {}

            ClientProxy::~ClientProxy() {}

            const std::string &ClientProxy::getName() const {
                return name;
            }

            const std::string &ClientProxy::getServiceName() const {
                return serviceName;
            }

            ClientContext &ClientProxy::getContext() {
                return context;
            }

            void ClientProxy::onDestroy() {
            }

            void ClientProxy::destroy() {
                getContext().getProxyManager().destroyProxy(*this);
            }

            void ClientProxy::destroyLocally() {
                if (preDestroy()) {
                    try {
                        onDestroy();
                        postDestroy();
                    } catch (exception::IException &e) {
                        postDestroy();
                        e.raise();
                    }
                }
            }

            bool ClientProxy::preDestroy() {
                return true;
            }

            void ClientProxy::postDestroy() {
            }

            void ClientProxy::onInitialize() {
            }

            void ClientProxy::onShutdown() {
            }

            serialization::pimpl::SerializationService &ClientProxy::getSerializationService() {
                return context.getSerializationService();
            }

            void ClientProxy::destroyRemotely() {
                std::unique_ptr<protocol::ClientMessage> clientMessage = protocol::codec::ClientDestroyProxyCodec::encodeRequest(
                        getName(), getServiceName());
                spi::impl::ClientInvocation::create(getContext(), clientMessage, getName())->invoke()->get();
            }

            ClientProxy::EventHandlerDelegator::EventHandlerDelegator(client::impl::BaseEventHandler *handler)
                    : handler(
                    handler) {}

            void ClientProxy::EventHandlerDelegator::handle(const std::shared_ptr<protocol::ClientMessage> &event) {
                handler->handle(event);
            }

            void ClientProxy::EventHandlerDelegator::beforeListenerRegister() {
                handler->beforeListenerRegister();
            }

            void ClientProxy::EventHandlerDelegator::onListenerRegister() {
                handler->onListenerRegister();
            }

            std::string ClientProxy::registerListener(const std::shared_ptr<spi::impl::ListenerMessageCodec> &codec,
                                                      client::impl::BaseEventHandler *handler) {
                handler->setLogger(&getContext().getLogger());
                return getContext().getClientListenerService().registerListener(codec,
                                                                                std::shared_ptr<spi::EventHandler<protocol::ClientMessage> >(
                                                                                        new EventHandlerDelegator(
                                                                                                handler)));
            }

            std::string
            ClientProxy::registerListener(const std::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                                          const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                std::static_pointer_cast<client::impl::BaseEventHandler>(handler)->setLogger(&getContext().getLogger());
                return getContext().getClientListenerService().registerListener(listenerMessageCodec, handler);
            }

            bool ClientProxy::deregisterListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace map {
            namespace impl {
                namespace nearcache {
                    bool TrueMarkerImpl::tryMark(const serialization::pimpl::Data &key) {
                        return true;
                    }

                    bool TrueMarkerImpl::tryUnmark(const serialization::pimpl::Data &key) {
                        return true;
                    }

                    bool TrueMarkerImpl::tryRemove(const serialization::pimpl::Data &key) {
                        return true;
                    }

                    void TrueMarkerImpl::forceUnmark(const serialization::pimpl::Data &key) {
                    }

                    void TrueMarkerImpl::init() {
                    }

                    const std::unique_ptr<KeyStateMarker> KeyStateMarker::TRUE_MARKER =
                            std::unique_ptr<KeyStateMarker>(new TrueMarkerImpl());
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace map {
            DataEntryView::DataEntryView(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                         int64_t cost,
                                         int64_t creationTime, int64_t expirationTime, int64_t hits,
                                         int64_t lastAccessTime,
                                         int64_t lastStoredTime, int64_t lastUpdateTime, int64_t version,
                                         int64_t evictionCriteriaNumber,
                                         int64_t ttl) : key(key), value(value), cost(cost), creationTime(creationTime),
                                                        expirationTime(expirationTime), hits(hits),
                                                        lastAccessTime(lastAccessTime),
                                                        lastStoredTime(lastStoredTime), lastUpdateTime(lastUpdateTime),
                                                        version(version),
                                                        evictionCriteriaNumber(evictionCriteriaNumber), ttl(ttl) {}


            const serialization::pimpl::Data &DataEntryView::getKey() const {
                return key;
            }

            const serialization::pimpl::Data &DataEntryView::getValue() const {
                return value;
            }

            int64_t DataEntryView::getCost() const {
                return cost;
            }

            int64_t DataEntryView::getCreationTime() const {
                return creationTime;
            }

            int64_t DataEntryView::getExpirationTime() const {
                return expirationTime;
            }

            int64_t DataEntryView::getHits() const {
                return hits;
            }

            int64_t DataEntryView::getLastAccessTime() const {
                return lastAccessTime;
            }

            int64_t DataEntryView::getLastStoredTime() const {
                return lastStoredTime;
            }

            int64_t DataEntryView::getLastUpdateTime() const {
                return lastUpdateTime;
            }

            int64_t DataEntryView::getVersion() const {
                return version;
            }

            int64_t DataEntryView::getEvictionCriteriaNumber() const {
                return evictionCriteriaNumber;
            }

            int64_t DataEntryView::getTtl() const {
                return ttl;
            }

        }
    }
}


namespace hazelcast {
    namespace client {

        int64_t IAtomicLong::addAndGet(int64_t delta) {
            return impl->addAndGet(delta);
        }

        bool IAtomicLong::compareAndSet(int64_t expect, int64_t update) {
            return impl->compareAndSet(expect, update);
        }

        int64_t IAtomicLong::decrementAndGet() {
            return impl->decrementAndGet();
        }

        int64_t IAtomicLong::get() {
            return impl->get();
        }

        int64_t IAtomicLong::getAndAdd(int64_t delta) {
            return impl->getAndAdd(delta);
        }

        int64_t IAtomicLong::getAndSet(int64_t newValue) {
            return impl->getAndSet(newValue);
        }

        int64_t IAtomicLong::incrementAndGet() {
            return impl->incrementAndGet();
        }

        int64_t IAtomicLong::getAndIncrement() {
            return impl->getAndIncrement();
        }

        void IAtomicLong::set(int64_t newValue) {
            impl->set(newValue);
        }

        IAtomicLong::IAtomicLong(const std::shared_ptr<impl::AtomicLongInterface> &impl) : impl(impl) {}

        const std::string &IAtomicLong::getServiceName() const {
            return impl->getServiceName();
        }

        const std::string &IAtomicLong::getName() const {
            return impl->getName();
        }

        void IAtomicLong::destroy() {
            impl->destroy();
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::addAndGetAsync(int64_t delta) {
            return impl->addAndGetAsync(delta);
        }

        std::shared_ptr<ICompletableFuture<bool> > IAtomicLong::compareAndSetAsync(int64_t expect, int64_t update) {
            return impl->compareAndSetAsync(expect, update);
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::decrementAndGetAsync() {
            return impl->decrementAndGetAsync();
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::getAsync() {
            return impl->getAsync();
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::getAndAddAsync(int64_t delta) {
            return impl->getAndAddAsync(delta);
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::getAndSetAsync(int64_t newValue) {
            return impl->getAndSetAsync(newValue);
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::incrementAndGetAsync() {
            return impl->incrementAndGetAsync();
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::getAndIncrementAsync() {
            return impl->getAndIncrementAsync();
        }

        std::shared_ptr<ICompletableFuture<void> > IAtomicLong::setAsync(int64_t newValue) {
            return impl->setAsync(newValue);
        }
    }
}


namespace hazelcast {
    namespace client {
        const std::string IExecutorService::SERVICE_NAME = "hz:impl:executorService";

        IExecutorService::IExecutorService(const std::string &name, spi::ClientContext *context) : ProxyImpl(
                SERVICE_NAME, name, context), consecutiveSubmits(0), lastSubmitTime(0) {
        }

        std::vector<Member>
        IExecutorService::selectMembers(const cluster::memberselector::MemberSelector &memberSelector) {
            std::vector<Member> selected;
            std::vector<Member> members = getContext().getClientClusterService().getMemberList();
            for (const Member &member : members) {
                if (memberSelector.select(member)) {
                    selected.push_back(member);
                }
            }
            if (selected.empty()) {
                throw (exception::ExceptionBuilder<exception::RejectedExecutionException>(
                        "IExecutorService::selectMembers") << "No member selected with memberSelector["
                                                           << memberSelector << "]").build();
            }
            return selected;
        }

        std::shared_ptr<spi::impl::ClientInvocationFuture>
        IExecutorService::invokeOnTarget(std::unique_ptr<protocol::ClientMessage> &request, const Address &target) {
            try {
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), target);
                return clientInvocation->invoke();
            } catch (exception::IException &e) {
                util::ExceptionUtil::rethrow(e);
            }
            return std::shared_ptr<spi::impl::ClientInvocationFuture>();
        }

        std::shared_ptr<spi::impl::ClientInvocationFuture>
        IExecutorService::invokeOnPartitionOwner(std::unique_ptr<protocol::ClientMessage> &request, int partitionId) {
            try {
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), partitionId);
                return clientInvocation->invoke();
            } catch (exception::IException &e) {
                util::ExceptionUtil::rethrow(e);
            }
            return std::shared_ptr<spi::impl::ClientInvocationFuture>();
        }

        bool IExecutorService::isSyncComputation(bool preventSync) {
            int64_t now = util::currentTimeMillis();

            int64_t last = lastSubmitTime;
            lastSubmitTime = now;

            if (last + MIN_TIME_RESOLUTION_OF_CONSECUTIVE_SUBMITS < now) {
                consecutiveSubmits = 0;
                return false;
            }

            return !preventSync && (consecutiveSubmits++ % MAX_CONSECUTIVE_SUBMITS == 0);
        }

        Address IExecutorService::getMemberAddress(const Member &member) {
            std::shared_ptr<Member> m = getContext().getClientClusterService().getMember(member.getUuid());
            if (m.get() == NULL) {
                throw (exception::ExceptionBuilder<exception::HazelcastException>(
                        "IExecutorService::getMemberAddress(Member)") << member << " is not available!").build();
            }
            return m->getAddress();
        }

        int IExecutorService::randomPartitionId() {
            spi::ClientPartitionService &partitionService = getContext().getPartitionService();
            return rand() % partitionService.getPartitionCount();
        }

        void IExecutorService::shutdown() {
            std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ExecutorServiceShutdownCodec::encodeRequest(
                    getName());
            invoke(request);
        }

        bool IExecutorService::isShutdown() {
            std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ExecutorServiceIsShutdownCodec::encodeRequest(
                    getName());
            return invokeAndGetResult<bool, protocol::codec::ExecutorServiceIsShutdownCodec::ResponseParameters>(
                    request);
        }

        bool IExecutorService::isTerminated() {
            return isShutdown();
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace atomiclong {
            namespace impl {
                AtomicLongProxyFactory::AtomicLongProxyFactory(spi::ClientContext *clientContext) : clientContext(
                        clientContext) {}

                std::shared_ptr<spi::ClientProxy> AtomicLongProxyFactory::create(const std::string &id) {
                    return std::shared_ptr<spi::ClientProxy>(new proxy::ClientAtomicLongProxy(id, clientContext));
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        ILock::ILock(const std::string &instanceName, spi::ClientContext *context)
                : proxy::ProxyImpl("hz:impl:lockService", instanceName, context),
                  key(toData<std::string>(instanceName)) {
            partitionId = getPartitionId(key);

            // TODO: remove this line once the client instance getDistributedObject works as expected in Java for this proxy type
            referenceIdGenerator = context->getLockReferenceIdGenerator();
        }

        void ILock::lock() {
            lock(-1);
        }

        void ILock::lock(long leaseTimeInMillis) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockLockCodec::encodeRequest(getName(), leaseTimeInMillis,
                                                                  util::getCurrentThreadId(),
                                                                  referenceIdGenerator->getNextReferenceId());

            invokeOnPartition(request, partitionId);
        }

        void ILock::unlock() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockUnlockCodec::encodeRequest(getName(), util::getCurrentThreadId(),
                                                                    referenceIdGenerator->getNextReferenceId());

            invokeOnPartition(request, partitionId);
        }

        void ILock::forceUnlock() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockForceUnlockCodec::encodeRequest(getName(),
                                                                         referenceIdGenerator->getNextReferenceId());

            invokeOnPartition(request, partitionId);
        }

        bool ILock::isLocked() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockIsLockedCodec::encodeRequest(getName());

            return invokeAndGetResult<bool, protocol::codec::LockIsLockedCodec::ResponseParameters>(request,
                                                                                                    partitionId);
        }

        bool ILock::isLockedByCurrentThread() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockIsLockedByCurrentThreadCodec::encodeRequest(getName(),
                                                                                     util::getCurrentThreadId());

            return invokeAndGetResult<bool, protocol::codec::LockIsLockedByCurrentThreadCodec::ResponseParameters>(
                    request, partitionId);
        }

        int ILock::getLockCount() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockGetLockCountCodec::encodeRequest(getName());

            return invokeAndGetResult<int, protocol::codec::LockGetLockCountCodec::ResponseParameters>(request,
                                                                                                       partitionId);
        }

        long ILock::getRemainingLeaseTime() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockGetRemainingLeaseTimeCodec::encodeRequest(getName());

            return invokeAndGetResult<long, protocol::codec::LockGetRemainingLeaseTimeCodec::ResponseParameters>(
                    request, partitionId);
        }

        bool ILock::tryLock() {
            return tryLock(0);
        }

        bool ILock::tryLock(long timeInMillis) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockTryLockCodec::encodeRequest(getName(), util::getCurrentThreadId(), LONG_MAX,
                                                                     timeInMillis,
                                                                     referenceIdGenerator->getNextReferenceId());

            return invokeAndGetResult<bool, protocol::codec::LockTryLockCodec::ResponseParameters>(request,
                                                                                                   partitionId);
        }

        void ILock::onInitialize() {
            ProxyImpl::onInitialize();

            referenceIdGenerator = getContext().getLockReferenceIdGenerator();
        }
    }
}

namespace hazelcast {
    namespace client {
        FlakeIdGenerator::FlakeIdGenerator(const std::shared_ptr<impl::IdGeneratorInterface> &impl) : IdGenerator(
                impl) {}

        int64_t FlakeIdGenerator::newId() {
            return IdGenerator::newId();
        }

        bool FlakeIdGenerator::init(int64_t id) {
            return IdGenerator::init(id);
        }
    }
}


namespace hazelcast {
    namespace client {

        SerializationConfig::SerializationConfig() : version(0) {
        }

        int SerializationConfig::getPortableVersion() const {
            return version;
        }

        SerializationConfig &SerializationConfig::setPortableVersion(int version) {
            this->version = version;
            return *this;
        }

        std::vector<std::shared_ptr<serialization::SerializerBase> > const &
        SerializationConfig::getSerializers() const {
            return serializers;
        }

        SerializationConfig &
        SerializationConfig::registerSerializer(std::shared_ptr<serialization::SerializerBase> serializer) {
            serializers.push_back(serializer);
            return *this;
        }

        SerializationConfig &
        SerializationConfig::registerSerializer(std::shared_ptr<serialization::StreamSerializer> serializer) {
            serializers.push_back(serializer);
            return *this;
        }

        SerializationConfig &SerializationConfig::addDataSerializableFactory(int32_t factoryId,
                                                                             std::shared_ptr<serialization::DataSerializableFactory> dataSerializableFactory) {
            dataSerializableFactories[factoryId] = dataSerializableFactory;
            return *this;
        }

        SerializationConfig &SerializationConfig::addPortableFactory(int32_t factoryId,
                                                                     std::shared_ptr<serialization::PortableFactory> portableFactory) {
            portableFactories[factoryId] = portableFactory;
            return *this;
        }

        const std::map<int32_t, std::shared_ptr<serialization::DataSerializableFactory> > &
        SerializationConfig::getDataSerializableFactories() const {
            return dataSerializableFactories;
        }

        const std::map<int32_t, std::shared_ptr<serialization::PortableFactory> > &
        SerializationConfig::getPortableFactories() const {
            return portableFactories;
        }

        SerializationConfig &
        SerializationConfig::setGlobalSerializer(const std::shared_ptr<serialization::StreamSerializer> &serializer) {
            globalSerializer = serializer;
            return *this;
        }

        const std::shared_ptr<serialization::StreamSerializer> &SerializationConfig::getGlobalSerializer() const {
            return globalSerializer;
        }
    }
}


// Includes for parameters classes

namespace hazelcast {
    namespace client {

        ISemaphore::ISemaphore(const std::string &name, spi::ClientContext *context)
                : proxy::ProxyImpl("hz:impl:semaphoreService", name, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&name);
            partitionId = getPartitionId(keyData);
        }

        bool ISemaphore::init(int permits) {
            checkNegative(permits);
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreInitCodec::encodeRequest(getName(), permits);

            return invokeAndGetResult<bool, protocol::codec::SemaphoreInitCodec::ResponseParameters>(request,
                                                                                                     partitionId);
        }

        void ISemaphore::acquire() {
            acquire(1);
        }

        void ISemaphore::acquire(int permits) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreAcquireCodec::encodeRequest(getName(), permits);

            invokeOnPartition(request, partitionId);
        }

        int ISemaphore::availablePermits() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreAvailablePermitsCodec::encodeRequest(getName());

            return invokeAndGetResult<int, protocol::codec::SemaphoreAvailablePermitsCodec::ResponseParameters>(request,
                                                                                                                partitionId);
        }

        int ISemaphore::drainPermits() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreDrainPermitsCodec::encodeRequest(getName());

            return invokeAndGetResult<int, protocol::codec::SemaphoreDrainPermitsCodec::ResponseParameters>(request,
                                                                                                            partitionId);
        }

        void ISemaphore::reducePermits(int reduction) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreReducePermitsCodec::encodeRequest(getName(), reduction);

            invokeOnPartition(request, partitionId);
        }

        void ISemaphore::release() {
            release(1);
        }

        void ISemaphore::release(int permits) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreReleaseCodec::encodeRequest(getName(), permits);

            invokeOnPartition(request, partitionId);
        }

        bool ISemaphore::tryAcquire() {
            return tryAcquire(int(1));
        }

        bool ISemaphore::tryAcquire(int permits) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreTryAcquireCodec::encodeRequest(getName(), permits, 0);

            return invokeAndGetResult<bool, protocol::codec::SemaphoreTryAcquireCodec::ResponseParameters>(request,
                                                                                                           partitionId);
        }

        bool ISemaphore::tryAcquire(long timeoutInMillis) {
            return tryAcquire(1, timeoutInMillis);
        }

        bool ISemaphore::tryAcquire(int permits, long timeoutInMillis) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreTryAcquireCodec::encodeRequest(getName(), permits, timeoutInMillis);

            return invokeAndGetResult<bool, protocol::codec::SemaphoreTryAcquireCodec::ResponseParameters>(request,
                                                                                                           partitionId);
        }

        void ISemaphore::increasePermits(int32_t increase) {
            checkNegative(increase);
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreIncreasePermitsCodec::encodeRequest(getName(), increase);
            invokeOnPartition(request, partitionId);
        }

        void ISemaphore::checkNegative(int32_t permits) const {
            if (permits < 0) {
                throw exception::IllegalArgumentException("ISemaphore::checkNegative", "Permits cannot be negative!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        InitialMembershipListener::~InitialMembershipListener() {
        }

        bool InitialMembershipListener::shouldRequestInitialMembers() const {
            return true;
        }

        void InitialMembershipListenerDelegator::init(
                const InitialMembershipEvent &event) {
            listener->init(event);
        }

        void InitialMembershipListenerDelegator::memberRemoved(
                const MembershipEvent &membershipEvent) {
            listener->memberRemoved(membershipEvent);
        }

        void InitialMembershipListenerDelegator::memberAdded(
                const MembershipEvent &membershipEvent) {
            listener->memberAdded(membershipEvent);
        }

        void InitialMembershipListenerDelegator::memberAttributeChanged(
                const MemberAttributeEvent &memberAttributeEvent) {
            listener->memberAttributeChanged(memberAttributeEvent);
        }

        InitialMembershipListenerDelegator::InitialMembershipListenerDelegator(
                InitialMembershipListener *listener) : listener(listener) {}

        bool InitialMembershipListenerDelegator::shouldRequestInitialMembers() const {
            return listener->shouldRequestInitialMembers();
        }

        const std::string &InitialMembershipListenerDelegator::getRegistrationId() const {
            return listener->getRegistrationId();
        }

        void InitialMembershipListenerDelegator::setRegistrationId(const std::string &registrationId) {
            listener->setRegistrationId(registrationId);
        }

    }
}

namespace hazelcast {
    namespace client {
        namespace executor {
            namespace impl {
                ExecutorServiceProxyFactory::ExecutorServiceProxyFactory(spi::ClientContext *clientContext)
                        : clientContext(
                        clientContext) {}

                std::shared_ptr<spi::ClientProxy> ExecutorServiceProxyFactory::create(const std::string &id) {
                    return std::shared_ptr<spi::ClientProxy>(
                            new IExecutorService(id, clientContext));
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace impl {
                Filter::Filter() {
                }

                /**
                 *
                 * Add a new filter with the given name and value to the query.
                 *
                 * @param name Filter name
                 * @param value Filter value
                 *
                 */
                void Filter::addFilter(const std::string &name, const std::string &value) {
                    std::stringstream out;
                    unsigned long index = filters.size() + 1;
                    out << "Filter." << index << ".Name";
                    filters[out.str()] = name;
                    out.str("");
                    out.clear();
                    out << "Filter." << index << ".Value.1";
                    filters[out.str()] = value;
                }

                const std::map<std::string, std::string> &Filter::getFilters() {
                    return filters;
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace impl {
                AwsAddressTranslator::AwsAddressTranslator(config::ClientAwsConfig &awsConfig, util::ILogger &logger)
                        : logger(logger) {
                    if (awsConfig.isEnabled() && !awsConfig.isInsideAws()) {
                        awsClient = std::unique_ptr<AWSClient>(new AWSClient(awsConfig, logger));
                    }
                }

                Address AwsAddressTranslator::translate(const Address &address) {
                    // if no translation is needed just return the address as it is
                    if (NULL == awsClient.get()) {
                        return address;
                    }

                    Address translatedAddress = address;

                    if (findFromCache(address, translatedAddress)) {
                        return translatedAddress;
                    }

                    refresh();

                    if (findFromCache(address, translatedAddress)) {
                        return translatedAddress;
                    }

                    std::stringstream out;
                    out << "No translation is found for private ip:" << address;
                    throw exception::IOException("AwsAddressTranslator::translate", out.str());
                }

                void AwsAddressTranslator::refresh() {
                    try {
                        privateToPublic = std::shared_ptr<std::map<std::string, std::string> >(
                                new std::map<std::string, std::string>(awsClient->getAddresses()));
                    } catch (exception::IException &e) {
                        logger.warning(std::string("AWS addresses failed to load: ") + e.what());
                    }
                }

                bool AwsAddressTranslator::findFromCache(const Address &address, Address &translatedAddress) {
                    std::shared_ptr<std::map<std::string, std::string> > mapping = privateToPublic;
                    if (mapping.get() == NULL) {
                        return false;
                    }

                    std::map<std::string, std::string>::const_iterator publicAddressIt = mapping->find(
                            address.getHost());
                    if (publicAddressIt != mapping->end()) {
                        const std::string &publicIp = (*publicAddressIt).second;
                        if (!publicIp.empty()) {
                            translatedAddress = Address((*publicAddressIt).second, address.getPort());
                            return true;
                        }
                    }

                    return false;
                }
            }
        }
    }
}


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace impl {
                const std::string DescribeInstances::QUERY_PREFIX = "/?";
                const std::string DescribeInstances::IAM_ROLE_ENDPOINT = "169.254.169.254";
                const std::string DescribeInstances::IAM_ROLE_QUERY = "/latest/meta-data/iam/security-credentials/";
                const std::string DescribeInstances::IAM_TASK_ROLE_ENDPOINT = "169.254.170.2";

                DescribeInstances::DescribeInstances(config::ClientAwsConfig &awsConfig, const std::string &endpoint,
                                                     util::ILogger &logger) : awsConfig(awsConfig), endpoint(endpoint),
                                                                              logger(logger) {
                    checkKeysFromIamRoles();

                    std::string timeStamp = getFormattedTimestamp();
                    rs = std::unique_ptr<security::EC2RequestSigner>(
                            new security::EC2RequestSigner(awsConfig, timeStamp, endpoint));
                    attributes["Action"] = "DescribeInstances";
                    attributes["Version"] = impl::Constants::DOC_VERSION;
                    attributes["X-Amz-Algorithm"] = impl::Constants::SIGNATURE_METHOD_V4;
                    attributes["X-Amz-Credential"] = rs->createFormattedCredential();
                    attributes["X-Amz-Date"] = timeStamp;
                    attributes["X-Amz-SignedHeaders"] = "host";
                    attributes["X-Amz-Expires"] = "30";
                    addFilters();
                }

                DescribeInstances::~DescribeInstances() {
                }

                std::map<std::string, std::string> DescribeInstances::execute() {
                    std::string signature = rs->sign(attributes);
                    attributes["X-Amz-Signature"] = signature;

                    std::istream &stream = callService();
                    return utility::CloudUtility::unmarshalTheResponse(stream, logger);
                }

                std::string DescribeInstances::getFormattedTimestamp() {
                    using namespace boost::posix_time;
                    ptime now = second_clock::universal_time();

                    std::ostringstream out;
                    std::locale timeLocale(out.getloc(), new time_facet(impl::Constants::DATE_FORMAT));
                    out.imbue(timeLocale);
                    out << now;
                    return out.str();
                }

                std::istream &DescribeInstances::callService() {
                    std::string query = rs->getCanonicalizedQueryString(attributes);
                    httpsClient = std::unique_ptr<util::SyncHttpsClient>(
                            new util::SyncHttpsClient(endpoint.c_str(), QUERY_PREFIX + query));
                    return httpsClient->openConnection();
                }

                void DescribeInstances::checkKeysFromIamRoles() {
                    if (awsConfig.getAccessKey().empty() || !awsConfig.getIamRole().empty()) {
                        tryGetDefaultIamRole();
                        if (!awsConfig.getIamRole().empty()) {
                            getKeysFromIamRole();
                        } else {
                            getKeysFromIamTaskRole();
                        }
                    }
                }

                void DescribeInstances::tryGetDefaultIamRole() {
                    // if none of the below are true
                    if (!(awsConfig.getIamRole().empty() || awsConfig.getIamRole() == "DEFAULT")) {
                        // stop here. No point looking up the default role.
                        return;
                    }
                    try {
                        util::SyncHttpClient httpClient(IAM_ROLE_ENDPOINT, IAM_ROLE_QUERY);
                        std::string roleName;
                        std::istream &responseStream = httpClient.openConnection();
                        responseStream >> roleName;
                        awsConfig.setIamRole(roleName);
                    } catch (exception::IOException &e) {
                        throw exception::InvalidConfigurationException("tryGetDefaultIamRole",
                                                                       std::string("Invalid Aws Configuration. ") +
                                                                       e.what());
                    }
                }

                void DescribeInstances::getKeysFromIamTaskRole() {
                    // before giving up, attempt to discover whether we're running in an ECS Container,
                    // in which case, AWS_CONTAINER_CREDENTIALS_RELATIVE_URI will exist as an env var.
                    const char *uri = getenv(Constants::ECS_CREDENTIALS_ENV_VAR_NAME);
                    if (!uri) {
                        throw exception::IllegalArgumentException("getKeysFromIamTaskRole",
                                                                  "Could not acquire credentials! Did not find declared AWS access key or IAM Role, and could not discover IAM Task Role or default role.");
                    }

                    util::SyncHttpClient httpClient(IAM_TASK_ROLE_ENDPOINT, uri);

                    try {
                        std::istream &istream = httpClient.openConnection();
                        parseAndStoreRoleCreds(istream);
                    } catch (exception::IException &e) {
                        std::stringstream out;
                        out << "Unable to retrieve credentials from IAM Task Role. URI: " << uri << ". \n " << e.what();
                        throw exception::InvalidConfigurationException("getKeysFromIamTaskRole", out.str());
                    }
                }

                void DescribeInstances::getKeysFromIamRole() {
                    std::string query = "/latest/meta-data/iam/security-credentials/" + awsConfig.getIamRole();

                    util::SyncHttpClient httpClient(IAM_ROLE_ENDPOINT, query);

                    try {
                        std::istream &istream = httpClient.openConnection();
                        parseAndStoreRoleCreds(istream);
                    } catch (exception::IException &e) {
                        std::stringstream out;
                        out << "Unable to retrieve credentials from IAM Task Role. URI: " << query << ". \n "
                            << e.what();
                        throw exception::InvalidConfigurationException("getKeysFromIamRole", out.str());
                    }
                }

                void DescribeInstances::parseAndStoreRoleCreds(std::istream &in) {
                    utility::CloudUtility::unmarshalJsonResponse(in, awsConfig, attributes);
                }

                /**
                 * Add available filters to narrow down the scope of the query
                 */
                void DescribeInstances::addFilters() {
                    Filter filter;
                    if (!awsConfig.getTagKey().empty()) {
                        if (!awsConfig.getTagValue().empty()) {
                            filter.addFilter(std::string("tag:") + awsConfig.getTagKey(), awsConfig.getTagValue());
                        } else {
                            filter.addFilter("tag-key", awsConfig.getTagKey());
                        }
                    } else if (!awsConfig.getTagValue().empty()) {
                        filter.addFilter("tag-value", awsConfig.getTagValue());
                    }

                    if (!awsConfig.getSecurityGroupName().empty()) {
                        filter.addFilter("instance.group-name", awsConfig.getSecurityGroupName());
                    }

                    filter.addFilter("instance-state-name", "running");
                    const std::map<std::string, std::string> &filters = filter.getFilters();
                    attributes.insert(filters.begin(), filters.end());
                }
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace aws {
            namespace impl {
                const char *Constants::DATE_FORMAT = "%Y%m%dT%H%M%SZ";
                const char *Constants::DOC_VERSION = "2016-11-15";
                const char *Constants::SIGNATURE_METHOD_V4 = "AWS4-HMAC-SHA256";
                const char *Constants::GET = "GET";
                const char *Constants::ECS_CREDENTIALS_ENV_VAR_NAME = "AWS_CONTAINER_CREDENTIALS_RELATIVE_URI";
            }
        }
    }
}


#ifdef HZ_BUILD_WITH_SSL
#endif


namespace hazelcast {
    namespace client {
        namespace aws {
            namespace security {
                std::string EC2RequestSigner::NEW_LINE = "\n";
                size_t EC2RequestSigner::DATE_LENGTH = 8;

                EC2RequestSigner::EC2RequestSigner(const config::ClientAwsConfig &awsConfig,
                                                   const std::string &timestamp,
                                                   const std::string &endpoint) : awsConfig(awsConfig),
                                                                                  timestamp(timestamp),
                                                                                  endpoint(endpoint) {
                }

                EC2RequestSigner::~EC2RequestSigner() {
                }

                std::string EC2RequestSigner::sign(const std::map<std::string, std::string> &attributes) {
                    std::string canonicalRequest = getCanonicalizedRequest(attributes);
                    std::string stringToSign = createStringToSign(canonicalRequest);
                    std::vector<unsigned char> signingKey = deriveSigningKey();

                    return createSignature(stringToSign, signingKey);
                }

                std::string EC2RequestSigner::createFormattedCredential() const {
                    std::stringstream out;
                    out << awsConfig.getAccessKey() << '/' << timestamp.substr(0, DATE_LENGTH) << '/'
                        << awsConfig.getRegion() << '/' << "ec2/aws4_request";
                    return out.str();
                }

                std::string EC2RequestSigner::getCanonicalizedQueryString(
                        const std::map<std::string, std::string> &attributes) const {
                    std::vector<std::string> components = getListOfEntries(attributes);
                    std::sort(components.begin(), components.end());
                    return getCanonicalizedQueryString(components);
                }

                /* Task 1 */
                std::string EC2RequestSigner::getCanonicalizedRequest(
                        const std::map<std::string, std::string> &attributes) const {
                    std::ostringstream out;
                    out << impl::Constants::GET << NEW_LINE
                        << '/' << NEW_LINE
                        << getCanonicalizedQueryString(attributes) << NEW_LINE
                        << getCanonicalHeaders() << NEW_LINE
                        << "host" << NEW_LINE
                        << sha256Hashhex("");
                    return out.str();
                }

                std::string EC2RequestSigner::getCanonicalHeaders() const {
                    std::ostringstream out;
                    out << "host:" << endpoint << NEW_LINE;
                    return out.str();
                }

                std::string EC2RequestSigner::getCanonicalizedQueryString(const std::vector<std::string> &list) const {
                    std::ostringstream result;
                    std::vector<std::string>::const_iterator it = list.begin();
                    result << (*it);
                    ++it;
                    for (; it != list.end(); ++it) {
                        result << "&" << *it;
                    }
                    return result.str();
                }

                std::vector<std::string> EC2RequestSigner::getListOfEntries(
                        const std::map<std::string, std::string> &entries) const {
                    std::vector<std::string> components;
                    for (std::map<std::string, std::string>::const_iterator it = entries.begin();
                         it != entries.end(); ++it) {
                        addComponents(components, entries, it->first);
                    }
                    return components;
                }

                void EC2RequestSigner::addComponents(std::vector<std::string> &components,
                                                     const std::map<std::string, std::string> &attributes,
                                                     const std::string &key) const {
                    std::ostringstream out;
                    out << utility::AwsURLEncoder::urlEncode(key) << '=' << utility::AwsURLEncoder::urlEncode(
                            (const_cast<std::map<std::string, std::string> &>(attributes))[key]);
                    components.push_back(out.str());
                }

                /* Task 2 */
                std::string EC2RequestSigner::createStringToSign(const std::string &canonicalRequest) const {
                    std::ostringstream out;
                    out << impl::Constants::SIGNATURE_METHOD_V4 << NEW_LINE
                        << timestamp << NEW_LINE
                        << getCredentialScope() << NEW_LINE
                        << sha256Hashhex(canonicalRequest);
                    return out.str();
                }

                std::string EC2RequestSigner::getCredentialScope() const {
                    // datestamp/region/service/API_TERMINATOR
                    // dateStamp
                    std::ostringstream out;
                    out << timestamp.substr(0, DATE_LENGTH) << "/" << awsConfig.getRegion() << "/ec2/aws4_request";
                    return out.str();
                }

                /* Task 3 */
                std::vector<unsigned char> EC2RequestSigner::deriveSigningKey() const {
                    const std::string &signKey = awsConfig.getSecretKey();
                    std::string dateStamp = timestamp.substr(0, DATE_LENGTH);
                    // this is derived from
                    // http://docs.aws.amazon.com/general/latest/gr/signature-v4-examples.html#signature-v4-examples-python

                    unsigned char kDate[32];
                    std::string key = std::string("AWS4") + signKey;
                    int kDateLen = hmacSHA256Bytes(key, dateStamp, kDate);

                    unsigned char kRegion[32];
                    int kRegionLen = hmacSHA256Bytes(kDate, kDateLen, awsConfig.getRegion(), kRegion);

                    unsigned char kService[32];
                    int kServiceLen = hmacSHA256Bytes(kRegion, kRegionLen, "ec2", kService);

                    std::vector<unsigned char> mSigning(32);
                    hmacSHA256Bytes(kService, kServiceLen, "aws4_request", &mSigning[0]);

                    return mSigning;
                }

                std::string EC2RequestSigner::createSignature(const std::string &stringToSign,
                                                              const std::vector<unsigned char> &signingKey) const {
                    return hmacSHA256Hex(signingKey, stringToSign);
                }

                std::string EC2RequestSigner::hmacSHA256Hex(const std::vector<unsigned char> &key,
                                                            const std::string &msg) const {
                    unsigned char hash[32];

                    unsigned int len = hmacSHA256Bytes(key, msg, hash);

                    return convertToHexString(hash, len);
                }

                std::string EC2RequestSigner::convertToHexString(const unsigned char *buffer, unsigned int len) const {
                    std::stringstream ss;
                    ss << std::hex << std::setfill('0');
                    for (unsigned int i = 0; i < len; i++) {
                        ss << std::hex << std::setw(2) << (unsigned int) buffer[i];
                    }

                    return (ss.str());
                }

                unsigned int EC2RequestSigner::hmacSHA256Bytes(const void *key, int keyLen, const std::string &msg,
                                                               unsigned char *hash) const {
                    return hmacSHA256Bytes(key, keyLen, (unsigned char *) &msg[0], msg.length(),
                                           hash);
                }

                unsigned int EC2RequestSigner::hmacSHA256Bytes(const std::string &key, const std::string &msg,
                                                               unsigned char *hash) const {
                    return hmacSHA256Bytes(&key[0], (int) key.length(), (unsigned char *) &msg[0], msg.length(),
                                           hash);
                }

                unsigned int EC2RequestSigner::hmacSHA256Bytes(const std::vector<unsigned char> &key,
                                                               const std::string &msg,
                                                               unsigned char *hash) const {
                    return hmacSHA256Bytes(&key[0], (int) key.size(), (unsigned char *) &msg[0], msg.length(),
                                           hash);
                }

                unsigned int EC2RequestSigner::hmacSHA256Bytes(const void *keyBuffer, int keyLen,
                                                               const unsigned char *data,
                                                               size_t dataLen,
                                                               unsigned char *hash) const {
#ifdef HZ_BUILD_WITH_SSL

#if OPENSSL_VERSION_NUMBER > 0x10100000L
                    HMAC_CTX *hmac = HMAC_CTX_new();
#else
                    HMAC_CTX *hmac = new HMAC_CTX;
                    HMAC_CTX_init(hmac);
#endif

                    HMAC_Init_ex(hmac, keyBuffer, keyLen, EVP_sha256(), NULL);
                    HMAC_Update(hmac, data, dataLen);
                    unsigned int len = 32;
                    HMAC_Final(hmac, hash, &len);

#if OPENSSL_VERSION_NUMBER > 0x10100000L
                    HMAC_CTX_free(hmac);
#else
                    HMAC_CTX_cleanup(hmac);
                    delete hmac;
#endif

                    return len;
#else
                    util::Preconditions::checkSSL("EC2RequestSigner::hmacSHA256Bytes");
                    return 0;
#endif
                }

                std::string EC2RequestSigner::sha256Hashhex(const std::string &in) const {
#ifdef HZ_BUILD_WITH_SSL
#ifdef OPENSSL_FIPS
                    unsigned int hashLen = 0;
                        unsigned char hash[EVP_MAX_MD_SIZE];
                        EVP_MD_CTX ctx;
                        EVP_MD_CTX_init(&ctx);
                        EVP_DigestInit_ex(&ctx, EVP_sha256(), NULL);
                        EVP_DigestUpdate(&ctx, in.c_str(), in.size());
                        EVP_DigestFinal_ex(&ctx, hash, &hashLen);
                        EVP_MD_CTX_cleanup(&ctx);
                        return convertToHexString(hash, hashLen);
#else
                    unsigned char hash[SHA256_DIGEST_LENGTH];
                    SHA256_CTX sha256;
                    SHA256_Init(&sha256);
                    SHA256_Update(&sha256, in.c_str(), in.size());
                    SHA256_Final(hash, &sha256);

                    return convertToHexString(hash, SHA256_DIGEST_LENGTH);
#endif // OPENSSL_FIPS
#else
                    util::Preconditions::checkSSL("EC2RequestSigner::hmacSHA256Bytes");
                    return "";
#endif // HZ_BUILD_WITH_SSL
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace aws {
            AWSClient::AWSClient(config::ClientAwsConfig &awsConfig, util::ILogger &logger) : awsConfig(awsConfig),
                                                                                              logger(logger) {
                this->endpoint = awsConfig.getHostHeader();
                if (!awsConfig.getRegion().empty() && awsConfig.getRegion().length() > 0) {
                    if (awsConfig.getHostHeader().find("ec2.") != 0) {
                        throw exception::InvalidConfigurationException("AWSClient::AWSClient",
                                                                       "HostHeader should start with \"ec2.\" prefix");
                    }
                    boost::replace_all(this->endpoint, "ec2.", std::string("ec2.") + awsConfig.getRegion() + ".");
                }
            }

            std::map<std::string, std::string> AWSClient::getAddresses() {
                return impl::DescribeInstances(awsConfig, endpoint, logger).execute();
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace aws {
            namespace utility {
                std::string AwsURLEncoder::urlEncode(const std::string &value) {
                    std::string result = escapeEncode(value);
                    boost::replace_all(result, "+", "%20");
                    return result;
                }

                std::string AwsURLEncoder::escapeEncode(const std::string &value) {
                    std::ostringstream escaped;
                    escaped.fill('0');
                    escaped << std::hex;

                    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
                        std::string::value_type c = (*i);

                        // Keep alphanumeric and other accepted characters intact
                        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                            escaped << c;
                            continue;
                        }

                        // Any other characters are percent-encoded
                        escaped << std::uppercase;
                        escaped << '%' << std::setw(2) << int((unsigned char) c);
                        escaped << std::nouppercase;
                    }

                    return escaped.str();
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace aws {
            namespace utility {
                std::map<std::string, std::string> CloudUtility::unmarshalTheResponse(std::istream &stream,
                                                                                      util::ILogger &logger) {
                    std::map<std::string, std::string> privatePublicPairs;

                    pt::ptree tree;
                    try {
                        pt::read_xml(stream, tree);
                    } catch (pt::xml_parser_error &e) {
                        logger.warning(
                                std::string("The parsed xml stream has errors: ") + e.what());
                        return privatePublicPairs;
                    }

                    // Use get_child to find the node containing the reservation set, and iterate over
                    // its children.
                    for (pt::ptree::value_type &item : tree.get_child("DescribeInstancesResponse.reservationSet")) {
                        for (pt::ptree::value_type &instanceItem : item.second.get_child("instancesSet")) {
                            auto privateIp = instanceItem.second.get_optional<std::string>("privateIpAddress");
                            auto publicIp = instanceItem.second.get_optional<std::string>("ipAddress");

                            auto prIp = privateIp.value_or("");
                            auto pubIp = publicIp.value_or("");

                            if (privateIp) {
                                privatePublicPairs[prIp] = pubIp;
                                if (logger.isFinestEnabled()) {
                                    auto instName = instanceItem.second.get_optional<std::string>(
                                            "tagset.item.value").value_or("");

                                    logger.finest(
                                            std::string("Accepting EC2 instance [") + instName +
                                            "][" + prIp + "]");
                                }
                            }
                        }
                    }
                    return privatePublicPairs;
                }

                void CloudUtility::unmarshalJsonResponse(std::istream &stream, config::ClientAwsConfig &awsConfig,
                                                         std::map<std::string, std::string> &attributes) {
                    pt::ptree json;
                    pt::read_json(stream, json);
                    awsConfig.setAccessKey(json.get_optional<std::string>("AccessKeyId").get_value_or(""));
                    awsConfig.setSecretKey(json.get_optional<std::string>("SecretAccessKey").get_value_or(""));
                    attributes["X-Amz-Security-Token"] = json.get_optional<std::string>("Token").get_value_or("");
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        const std::vector<Member> &InitialMembershipEvent::getMembers() const {
            return members;
        }

        Cluster &InitialMembershipEvent::getCluster() {
            return cluster;
        }

        InitialMembershipEvent::InitialMembershipEvent(Cluster &cluster, const std::set<Member> &members) : cluster(
                cluster) {
            for (const Member &member : members) {
                this->members.push_back(Member(member));
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            InstanceOfPredicate::InstanceOfPredicate(const char *javaClassName) : className(javaClassName) {
            }

            int InstanceOfPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int InstanceOfPredicate::getClassId() const {
                return impl::predicates::INSTANCEOF_PREDICATE;
            }

            void InstanceOfPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&className);
            }

            void InstanceOfPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("InstanceOfPredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            ILikePredicate::ILikePredicate(const std::string &attribute, const std::string &expression) : attributeName(
                    attribute), expressionString(expression) {
            }

            int ILikePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int ILikePredicate::getClassId() const {
                return impl::predicates::ILIKE_PREDICATE;
            }

            void ILikePredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&attributeName);
                out.writeUTF(&expressionString);
            }

            void ILikePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("ILikePredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            int TruePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int TruePredicate::getClassId() const {
                return impl::predicates::TRUE_PREDICATE;
            }

            void TruePredicate::writeData(serialization::ObjectDataOutput &out) const {
            }

            void TruePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("TruePredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            OrPredicate::~OrPredicate() {
                for (std::vector<Predicate *>::const_iterator it = predicates.begin();
                     it != predicates.end(); ++it) {
                    delete *it;
                }
            }

            OrPredicate &OrPredicate::add(std::unique_ptr<Predicate> &predicate) {
                return add(std::move(predicate));
            }

            OrPredicate &OrPredicate::add(std::unique_ptr<Predicate> &&predicate) {
                predicates.push_back(predicate.release());
                return *this;
            }

            int OrPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int OrPredicate::getClassId() const {
                return impl::predicates::OR_PREDICATE;
            }

            void OrPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeInt((int) predicates.size());
                for (std::vector<Predicate *>::const_iterator it = predicates.begin();
                     it != predicates.end(); ++it) {
                    out.writeObject<serialization::IdentifiedDataSerializable>(*it);
                }
            }

            void OrPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("OrPredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            NotPredicate::NotPredicate(std::unique_ptr<Predicate> &predicate)
                    : NotPredicate::NotPredicate(std::move(predicate)) {
            }

            NotPredicate::NotPredicate(std::unique_ptr<Predicate> &&predicate)
                    : internalPredicate(std::move(predicate)) {
            }

            int NotPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int NotPredicate::getClassId() const {
                return impl::predicates::NOT_PREDICATE;
            }

            void NotPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeObject<serialization::IdentifiedDataSerializable>(internalPredicate.get());
            }

            void NotPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("NotPredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            const char *QueryConstants::getKeyAttributeName() {
                return "__key";
            }

            const char *QueryConstants::getValueAttributeName() {
                return "this";
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            SqlPredicate::SqlPredicate(const std::string &sqlString) : sql(sqlString) {
            }

            int SqlPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int SqlPredicate::getClassId() const {
                return impl::predicates::SQL_PREDICATE;
            }

            void SqlPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&sql);
            }

            void SqlPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("SqlPredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            int FalsePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int FalsePredicate::getClassId() const {
                return impl::predicates::FALSE_PREDICATE;
            }

            void FalsePredicate::writeData(serialization::ObjectDataOutput &out) const {
            }

            void FalsePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("FalsePredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            RegexPredicate::RegexPredicate(const char *attribute, const char *regex) : attributeName(attribute),
                                                                                       regularExpression(regex) {
            }

            int RegexPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int RegexPredicate::getClassId() const {
                return impl::predicates::REGEX_PREDICATE;
            }

            void RegexPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&attributeName);
                out.writeUTF(&regularExpression);
            }

            void RegexPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("RegexPredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            AndPredicate::~AndPredicate() {
                for (std::vector<Predicate *>::const_iterator it = predicates.begin();
                     it != predicates.end(); ++it) {
                    delete *it;
                }
            }

            AndPredicate &AndPredicate::add(std::unique_ptr<Predicate> &predicate) {
                return add(std::move(predicate));
            }

            AndPredicate &AndPredicate::add(std::unique_ptr<Predicate> &&predicate) {
                predicates.push_back(predicate.release());
                return *this;
            }

            int AndPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int AndPredicate::getClassId() const {
                return impl::predicates::AND_PREDICATE;
            }

            void AndPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeInt((int) predicates.size());
                for (std::vector<Predicate *>::const_iterator it = predicates.begin();
                     it != predicates.end(); ++it) {
                    out.writeObject<serialization::IdentifiedDataSerializable>(*it);
                }
            }

            void AndPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("AndPredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            LikePredicate::LikePredicate(const std::string &attribute, const std::string &expression) : attributeName(
                    attribute), expressionString(expression) {
            }

            int LikePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int LikePredicate::getClassId() const {
                return impl::predicates::LIKE_PREDICATE;
            }

            void LikePredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&attributeName);
                out.writeUTF(&expressionString);
            }

            void LikePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("LikePredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {

        const std::string ClientProperties::PROP_HEARTBEAT_TIMEOUT = "hazelcast_client_heartbeat_timeout";
        const std::string ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT = "60";
        const std::string ClientProperties::PROP_HEARTBEAT_INTERVAL = "hazelcast_client_heartbeat_interval";
        const std::string ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT = "10";
        const std::string ClientProperties::PROP_REQUEST_RETRY_COUNT = "hazelcast_client_request_retry_count";
        const std::string ClientProperties::PROP_REQUEST_RETRY_COUNT_DEFAULT = "20";
        const std::string ClientProperties::PROP_REQUEST_RETRY_WAIT_TIME = "hazelcast_client_request_retry_wait_time";
        const std::string ClientProperties::PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT = "1";

        const std::string ClientProperties::PROP_AWS_MEMBER_PORT = "hz-port";
        const std::string ClientProperties::PROP_AWS_MEMBER_PORT_DEFAULT = "5701";

        const std::string ClientProperties::CLEAN_RESOURCES_PERIOD_MILLIS = "hazelcast.client.internal.clean.resources.millis";
        const std::string ClientProperties::CLEAN_RESOURCES_PERIOD_MILLIS_DEFAULT = "100";

        const std::string ClientProperties::INVOCATION_RETRY_PAUSE_MILLIS = "hazelcast.client.invocation.retry.pause.millis";
        const std::string ClientProperties::INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT = "1000";

        const std::string ClientProperties::INVOCATION_TIMEOUT_SECONDS = "hazelcast.client.invocation.timeout.seconds";
        const std::string ClientProperties::INVOCATION_TIMEOUT_SECONDS_DEFAULT = "120";

        const std::string ClientProperties::EVENT_THREAD_COUNT = "hazelcast.client.event.thread.count";
        const std::string ClientProperties::EVENT_THREAD_COUNT_DEFAULT = "5";

        const std::string ClientProperties::EVENT_QUEUE_CAPACITY = "hazelcast.client.event.queue.capacity";
        const std::string ClientProperties::EVENT_QUEUE_CAPACITY_DEFAULT = "1000000";

        const std::string ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE = "hazelcast.client.internal.executor.pool.size";
        const std::string ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT = "3";

        const std::string ClientProperties::SHUFFLE_MEMBER_LIST = "hazelcast.client.shuffle.member.list";
        const std::string ClientProperties::SHUFFLE_MEMBER_LIST_DEFAULT = "true";

        const std::string ClientProperties::MAX_CONCURRENT_INVOCATIONS = "hazelcast.client.max.concurrent.invocations";
        const std::string ClientProperties::MAX_CONCURRENT_INVOCATIONS_DEFAULT = util::IOUtil::to_string<int32_t>(
                INT32_MAX);

        const std::string ClientProperties::BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS = "hazelcast.client.invocation.backoff.timeout.millis";
        const std::string ClientProperties::BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT = "-1";

        const std::string ClientProperties::STATISTICS_ENABLED = "hazelcast.client.statistics.enabled";
        const std::string ClientProperties::STATISTICS_ENABLED_DEFAULT = "false";

        const std::string ClientProperties::STATISTICS_PERIOD_SECONDS = "hazelcast.client.statistics.period.seconds";
        const std::string ClientProperties::STATISTICS_PERIOD_SECONDS_DEFAULT = "3";

        ClientProperty::ClientProperty(const std::string &name, const std::string &defaultValue)
                : name(name), defaultValue(defaultValue) {
        }

        const std::string &ClientProperty::getName() const {
            return name;
        }

        const std::string &ClientProperty::getDefaultValue() const {
            return defaultValue;
        }

        const char *ClientProperty::getSystemProperty() const {
            return ::getenv(name.c_str());
        }

        ClientProperties::ClientProperties(const std::map<std::string, std::string> &properties)
                : heartbeatTimeout(PROP_HEARTBEAT_TIMEOUT, PROP_HEARTBEAT_TIMEOUT_DEFAULT),
                  heartbeatInterval(PROP_HEARTBEAT_INTERVAL, PROP_HEARTBEAT_INTERVAL_DEFAULT),
                  retryCount(PROP_REQUEST_RETRY_COUNT, PROP_REQUEST_RETRY_COUNT_DEFAULT),
                  retryWaitTime(PROP_REQUEST_RETRY_WAIT_TIME, PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT),
                  awsMemberPort(PROP_AWS_MEMBER_PORT, PROP_AWS_MEMBER_PORT_DEFAULT),
                  cleanResourcesPeriod(CLEAN_RESOURCES_PERIOD_MILLIS,
                                       CLEAN_RESOURCES_PERIOD_MILLIS_DEFAULT),
                  invocationRetryPauseMillis(INVOCATION_RETRY_PAUSE_MILLIS,
                                             INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT),
                  invocationTimeoutSeconds(INVOCATION_TIMEOUT_SECONDS,
                                           INVOCATION_TIMEOUT_SECONDS_DEFAULT),
                  eventThreadCount(EVENT_THREAD_COUNT, EVENT_THREAD_COUNT_DEFAULT),
                  eventQueueCapacity(EVENT_QUEUE_CAPACITY, EVENT_QUEUE_CAPACITY_DEFAULT),
                  internalExecutorPoolSize(INTERNAL_EXECUTOR_POOL_SIZE,
                                           INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT),
                  shuffleMemberList(SHUFFLE_MEMBER_LIST, SHUFFLE_MEMBER_LIST_DEFAULT),
                  maxConcurrentInvocations(MAX_CONCURRENT_INVOCATIONS,
                                           MAX_CONCURRENT_INVOCATIONS_DEFAULT),
                  backpressureBackoffTimeoutMillis(BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS,
                                                   BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT),
                  statisticsEnabled(STATISTICS_ENABLED, STATISTICS_ENABLED_DEFAULT),
                  statisticsPeriodSeconds(STATISTICS_PERIOD_SECONDS, STATISTICS_PERIOD_SECONDS_DEFAULT),
                  propertiesMap(properties) {
        }

        const ClientProperty &ClientProperties::getHeartbeatTimeout() const {
            return heartbeatTimeout;
        }

        const ClientProperty &ClientProperties::getHeartbeatInterval() const {
            return heartbeatInterval;
        }

        const ClientProperty &ClientProperties::getAwsMemberPort() const {
            return awsMemberPort;
        }

        const ClientProperty &ClientProperties::getCleanResourcesPeriodMillis() const {
            return cleanResourcesPeriod;
        }

        const ClientProperty &ClientProperties::getInvocationRetryPauseMillis() const {
            return invocationRetryPauseMillis;
        }

        const ClientProperty &ClientProperties::getInvocationTimeoutSeconds() const {
            return invocationTimeoutSeconds;
        }

        const ClientProperty &ClientProperties::getEventThreadCount() const {
            return eventThreadCount;
        }

        const ClientProperty &ClientProperties::getEventQueueCapacity() const {
            return eventQueueCapacity;
        }

        const ClientProperty &ClientProperties::getInternalExecutorPoolSize() const {
            return internalExecutorPoolSize;
        }

        const ClientProperty &ClientProperties::getShuffleMemberList() const {
            return shuffleMemberList;
        }

        const ClientProperty &ClientProperties::getMaxConcurrentInvocations() const {
            return maxConcurrentInvocations;
        }

        const ClientProperty &ClientProperties::getBackpressureBackoffTimeoutMillis() const {
            return backpressureBackoffTimeoutMillis;
        }

        const ClientProperty &ClientProperties::getStatisticsEnabled() const {
            return statisticsEnabled;
        }

        const ClientProperty &ClientProperties::getStatisticsPeriodSeconds() const {
            return statisticsPeriodSeconds;
        }

        std::string ClientProperties::getString(const ClientProperty &property) const {
            std::map<std::string, std::string>::const_iterator valueIt = propertiesMap.find(property.getName());
            if (valueIt != propertiesMap.end()) {
                return valueIt->second;
            }

            const char *value = property.getSystemProperty();
            if (value != NULL) {
                return value;
            }

            return property.getDefaultValue();
        }

        bool ClientProperties::getBoolean(const ClientProperty &property) const {
            return util::IOUtil::to_value<bool>(getString(property));
        }

        int32_t ClientProperties::getInteger(const ClientProperty &property) const {
            return util::IOUtil::to_value<int32_t>(getString(property));
        }

        int64_t ClientProperties::getLong(const ClientProperty &property) const {
            return util::IOUtil::to_value<int64_t>(getString(property));
        }
    }
}




namespace hazelcast {
    namespace client {

        GroupConfig::GroupConfig() : name("dev"), password("dev-pass") {

        }

        GroupConfig::GroupConfig(const std::string &name, const std::string &password)
                : name(name), password(password) {
        }

        std::string GroupConfig::getName() const {
            return name;
        }

        GroupConfig &GroupConfig::setName(const std::string &name) {
            this->name = name;
            return (*this);
        }

        GroupConfig &GroupConfig::setPassword(const std::string &password) {
            this->password = password;
            return (*this);
        }

        std::string GroupConfig::getPassword() const {
            return password;
        }

    }
}


namespace hazelcast {
    namespace client {
        LifecycleEvent::LifecycleEvent(LifeCycleState state)
                : state(state) {

        }

        LifecycleEvent::LifeCycleState LifecycleEvent::getState() const {
            return state;
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace exception {
            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, const std::string &details,
                                   int32_t errorNo, int32_t causeCode, bool isRuntime, bool retryable) : src(source),
                                                                                                         msg(message),
                                                                                                         details(details),
                                                                                                         errorCode(
                                                                                                                 errorNo),
                                                                                                         causeErrorCode(
                                                                                                                 causeCode),
                                                                                                         runtimeException(
                                                                                                                 isRuntime),
                                                                                                         retryable(
                                                                                                                 retryable) {
                std::ostringstream out;
                out << exceptionName << " {" << message << ". Details:" << details << " Error code:" << errorNo
                    << ", Cause error code:" << causeCode << "} at " + source;
                report = out.str();
            }

            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, int32_t errorNo,
                                   int32_t causeCode, bool isRuntime, bool retryable) : src(source), msg(message),
                                                                                        errorCode(errorNo),
                                                                                        causeErrorCode(causeCode),
                                                                                        runtimeException(isRuntime),
                                                                                        retryable(retryable) {
                std::ostringstream out;
                out << exceptionName << " {" << message << " Error code:" << errorNo << ", Cause error code:"
                    << causeCode << "} at " + source;
                report = out.str();
            }

            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, int32_t errorNo, bool isRuntime, bool retryable) : src(
                    source), msg(message), errorCode(errorNo), runtimeException(isRuntime), retryable(retryable) {
                std::ostringstream out;
                out << exceptionName << " {" << message << " Error code:" << errorNo << "} at " + source;
                report = out.str();
            }

            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, int32_t errorNo,
                                   const std::shared_ptr<IException> &cause, bool isRuntime, bool retryable) : src(
                    source), msg(message),
                                                                                                               errorCode(
                                                                                                                       errorNo),
                                                                                                               cause(cause),
                                                                                                               runtimeException(
                                                                                                                       isRuntime),
                                                                                                               retryable(
                                                                                                                       retryable) {
                std::ostringstream out;
                out << exceptionName << " {" << message << " Error code:" << errorNo << ", Caused by:" << *cause
                    << "} at " + source;
                report = out.str();

            }

            IException::~IException() throw() {
            }

            char const *IException::what() const throw() {
                return report.c_str();
            }

            const std::string &IException::getSource() const {
                return src;
            }

            const std::string &IException::getMessage() const {
                return msg;
            }

            void IException::raise() const {
                throw *this;
            }

            std::ostream &operator<<(std::ostream &os, const IException &exception) {
                os << exception.what();
                return os;
            }

            const std::shared_ptr<IException> &IException::getCause() const {
                return cause;
            }

            std::unique_ptr<IException> IException::clone() const {
                return std::unique_ptr<IException>(new IException(*this));
            }

            const std::string &IException::getDetails() const {
                return details;
            }

            int32_t IException::getErrorCode() const {
                return errorCode;
            }

            int32_t IException::getCauseErrorCode() const {
                return causeErrorCode;
            }

            bool IException::isRuntimeException() const {
                return runtimeException;
            }

            bool IException::isRetryable() const {
                return retryable;
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace exception {
            UndefinedErrorCodeException::UndefinedErrorCodeException(const std::string &source,
                                                                     const std::string &message,
                                                                     int32_t errorCode, int64_t correlationId,
                                                                     std::string details)
                    : IException("UndefinedErrorCodeException", source, message, protocol::UNDEFINED, true),
                      error(errorCode),
                      messageCallId(correlationId),
                      detailedErrorMessage(details) {
            }

            int32_t UndefinedErrorCodeException::getUndefinedErrorCode() const {
                return error;
            }

            int64_t UndefinedErrorCodeException::getMessageCallId() const {
                return messageCallId;
            }

            const std::string &UndefinedErrorCodeException::getDetailedErrorMessage() const {
                return detailedErrorMessage;
            }

            UndefinedErrorCodeException::~UndefinedErrorCodeException() throw() {
            }

            std::unique_ptr<IException> UndefinedErrorCodeException::clone() const {
                return std::unique_ptr<IException>(new UndefinedErrorCodeException(*this));
            }

            void UndefinedErrorCodeException::raise() const {
                throw *this;
            }

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     const std::string &details, int32_t causeCode)
                    : IException("RetryableHazelcastException", source, message, details, protocol::RETRYABLE_HAZELCAST,
                                 causeCode, true, true), HazelcastException(source, message, details, causeCode) {
            }

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message)
                    : IException("RetryableHazelcastException", source, message, protocol::RETRYABLE_HAZELCAST, true,
                                 true),
                      HazelcastException(source, message) {
            }

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     int32_t causeCode) : IException(
                    "RetryableHazelcastException", source, message, protocol::RETRYABLE_HAZELCAST, causeCode, true,
                    true),
                                                                                          HazelcastException(source,
                                                                                                             message,
                                                                                                             causeCode) {}

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     const std::shared_ptr<IException> &cause)
                    : IException("RetryableHazelcastException", source, message, protocol::RETRYABLE_HAZELCAST, cause,
                                 true, true),
                      HazelcastException(source, message, cause) {}

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message,
                                                     const std::string &details, int32_t causeCode)
                    : IException("MemberLeftException", source, message, details, protocol::MEMBER_LEFT, causeCode,
                                 true),
                      ExecutionException(source, message, details, causeCode),
                      RetryableHazelcastException(source, message, details, causeCode) {
            }

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message,
                                                     int32_t causeCode) : IException("MemberLeftException", source,
                                                                                     message, protocol::MEMBER_LEFT,
                                                                                     causeCode, true, true),
                                                                          ExecutionException(source, message,
                                                                                             causeCode),
                                                                          RetryableHazelcastException(source, message,
                                                                                                      causeCode) {
            }

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message)
                    : IException("MemberLeftException", source, message, protocol::MEMBER_LEFT, true, true),
                      ExecutionException(source, message), RetryableHazelcastException(source, message) {
            }

            void MemberLeftException::raise() const {
                throw *this;
            }

            std::unique_ptr<IException> MemberLeftException::clone() const {
                return std::unique_ptr<IException>(new MemberLeftException(*this));
            }

        }
    }
}


namespace hazelcast {
    namespace client {

        ClientConfig::ClientConfig()
                : loadBalancer(NULL), redoOperation(false), socketInterceptor(NULL), credentials(NULL),
                  executorPoolSize(-1) {
        }

        ClientConfig &ClientConfig::addAddress(const Address &address) {
            networkConfig.addAddress(address);
            return (*this);
        }

        ClientConfig &ClientConfig::addAddresses(const std::vector<Address> &addresses) {
            networkConfig.addAddresses(addresses);
            return (*this);
        }


        std::set<Address> ClientConfig::getAddresses() {
            std::set<Address> result;
            for (const Address &address : networkConfig.getAddresses()) {
                result.insert(address);
            }
            return result;
        }

        ClientConfig &ClientConfig::setGroupConfig(const GroupConfig &groupConfig) {
            this->groupConfig = groupConfig;
            return *this;
        }


        GroupConfig &ClientConfig::getGroupConfig() {
            return groupConfig;
        }


        ClientConfig &ClientConfig::setConnectionAttemptLimit(int connectionAttemptLimit) {
            networkConfig.setConnectionAttemptLimit(connectionAttemptLimit);
            return *this;
        }

        int ClientConfig::getConnectionAttemptLimit() const {
            return networkConfig.getConnectionAttemptLimit();
        }

        ClientConfig &ClientConfig::setConnectionTimeout(int connectionTimeoutInMillis) {
            this->networkConfig.setConnectionTimeout(connectionTimeoutInMillis);
            return *this;
        }

        int ClientConfig::getConnectionTimeout() const {
            return (int) this->networkConfig.getConnectionTimeout();
        }

        ClientConfig &ClientConfig::setAttemptPeriod(int attemptPeriodInMillis) {
            networkConfig.setConnectionAttemptPeriod(attemptPeriodInMillis);
            return *this;
        }

        int ClientConfig::getAttemptPeriod() const {
            return networkConfig.getConnectionAttemptPeriod();
        }

        ClientConfig &ClientConfig::setRedoOperation(bool redoOperation) {
            this->redoOperation = redoOperation;
            return *this;
        }

        bool ClientConfig::isRedoOperation() const {
            return redoOperation;
        }

        LoadBalancer *const ClientConfig::getLoadBalancer() {
            if (loadBalancer == NULL)
                return &defaultLoadBalancer;
            return loadBalancer;
        }

        ClientConfig &ClientConfig::setLoadBalancer(LoadBalancer *loadBalancer) {
            this->loadBalancer = loadBalancer;
            return *this;
        }

        ClientConfig &ClientConfig::setLogLevel(LogLevel loggerLevel) {
            this->loggerConfig.setLogLevel((LoggerLevel::Level) loggerLevel);
            return *this;
        }

        config::LoggerConfig &ClientConfig::getLoggerConfig() {
            return loggerConfig;
        }

        ClientConfig &ClientConfig::addListener(LifecycleListener *listener) {
            lifecycleListeners.insert(listener);
            return *this;
        }

        ClientConfig &ClientConfig::addListener(MembershipListener *listener) {
            if (listener == NULL) {
                throw exception::NullPointerException("ClientConfig::addListener(MembershipListener *)",
                                                      "listener can't be null");
            }

            membershipListeners.insert(listener);
            managedMembershipListeners.insert(
                    std::shared_ptr<MembershipListener>(new MembershipListenerDelegator(listener)));
            return *this;
        }

        ClientConfig &ClientConfig::addListener(InitialMembershipListener *listener) {
            if (listener == NULL) {
                throw exception::NullPointerException("ClientConfig::addListener(InitialMembershipListener *)",
                                                      "listener can't be null");
            }

            membershipListeners.insert(listener);
            managedMembershipListeners.insert(
                    std::shared_ptr<MembershipListener>(new InitialMembershipListenerDelegator(listener)));
            return *this;
        }

        ClientConfig &ClientConfig::addListener(const std::shared_ptr<MembershipListener> &listener) {
            membershipListeners.insert(listener.get());
            managedMembershipListeners.insert(listener);
            return *this;
        }

        ClientConfig &ClientConfig::addListener(const std::shared_ptr<InitialMembershipListener> &listener) {
            membershipListeners.insert(listener.get());
            managedMembershipListeners.insert(listener);
            return *this;
        }

        const std::set<LifecycleListener *> &ClientConfig::getLifecycleListeners() const {
            return lifecycleListeners;
        }

        const std::set<MembershipListener *> &ClientConfig::getMembershipListeners() const {
            return membershipListeners;
        }

        ClientConfig &ClientConfig::setCredentials(Credentials *credentials) {
            this->credentials = credentials;
            return *this;
        }

        const Credentials *ClientConfig::getCredentials() {
            return credentials;
        }

        ClientConfig &ClientConfig::setSocketInterceptor(SocketInterceptor *socketInterceptor) {
            this->socketInterceptor = socketInterceptor;
            return *this;
        }

        SocketInterceptor *ClientConfig::getSocketInterceptor() {
            return socketInterceptor;
        }

        ClientConfig &ClientConfig::setSmart(bool smart) {
            networkConfig.setSmartRouting(smart);
            return *this;
        }

        bool ClientConfig::isSmart() const {
            return networkConfig.isSmartRouting();
        }

        SerializationConfig &ClientConfig::getSerializationConfig() {
            return serializationConfig;
        }

        ClientConfig &ClientConfig::setSerializationConfig(SerializationConfig const &serializationConfig) {
            this->serializationConfig = serializationConfig;
            return *this;
        }


        std::map<std::string, std::string> &ClientConfig::getProperties() {
            return properties;
        }

        ClientConfig &ClientConfig::setProperty(const std::string &name, const std::string &value) {
            properties[name] = value;
            return *this;
        }

        ClientConfig &ClientConfig::addReliableTopicConfig(const config::ReliableTopicConfig &reliableTopicConfig) {
            reliableTopicConfigMap[reliableTopicConfig.getName()] = reliableTopicConfig;
            return *this;
        }

        const config::ReliableTopicConfig *ClientConfig::getReliableTopicConfig(const std::string &name) {
            std::map<std::string, config::ReliableTopicConfig>::const_iterator it = reliableTopicConfigMap.find(name);
            if (reliableTopicConfigMap.end() == it) {
                reliableTopicConfigMap[name] = config::ReliableTopicConfig(name.c_str());
            }
            return &reliableTopicConfigMap[name];
        }

        config::ClientNetworkConfig &ClientConfig::getNetworkConfig() {
            return networkConfig;
        }

        ClientConfig &ClientConfig::setNetworkConfig(const config::ClientNetworkConfig &networkConfig) {
            this->networkConfig = networkConfig;
            return *this;
        }

        const std::shared_ptr<mixedtype::config::MixedNearCacheConfig>
        ClientConfig::getMixedNearCacheConfig(const std::string &name) {
            return std::static_pointer_cast<mixedtype::config::MixedNearCacheConfig>(
                    getNearCacheConfig<TypedData, TypedData>(name));
        }

        const std::shared_ptr<std::string> &ClientConfig::getInstanceName() const {
            return instanceName;
        }

        void ClientConfig::setInstanceName(const std::shared_ptr<std::string> &instanceName) {
            ClientConfig::instanceName = instanceName;
        }

        int32_t ClientConfig::getExecutorPoolSize() const {
            return executorPoolSize;
        }

        void ClientConfig::setExecutorPoolSize(int32_t executorPoolSize) {
            ClientConfig::executorPoolSize = executorPoolSize;
        }

        config::ClientConnectionStrategyConfig &ClientConfig::getConnectionStrategyConfig() {
            return connectionStrategyConfig;
        }

        ClientConfig &ClientConfig::setConnectionStrategyConfig(
                const config::ClientConnectionStrategyConfig &connectionStrategyConfig) {
            ClientConfig::connectionStrategyConfig = connectionStrategyConfig;
            return *this;
        }

        std::shared_ptr<config::ClientFlakeIdGeneratorConfig>
        ClientConfig::findFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
                    configPatternMatcher, flakeIdGeneratorConfigMap, baseName);
            if (config.get() != NULL) {
                return config;
            }
            return getFlakeIdGeneratorConfig("default");
        }


        std::shared_ptr<config::ClientFlakeIdGeneratorConfig>
        ClientConfig::getFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
                    configPatternMatcher, flakeIdGeneratorConfigMap, baseName);
            if (config.get() != NULL) {
                return config;
            }
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> defConfig = flakeIdGeneratorConfigMap.get("default");
            if (defConfig.get() == NULL) {
                defConfig.reset(new config::ClientFlakeIdGeneratorConfig("default"));
                flakeIdGeneratorConfigMap.put(defConfig->getName(), defConfig);
            }
            config.reset(new config::ClientFlakeIdGeneratorConfig(*defConfig));
            config->setName(name);
            flakeIdGeneratorConfigMap.put(config->getName(), config);
            return config;
        }

        ClientConfig &
        ClientConfig::addFlakeIdGeneratorConfig(const std::shared_ptr<config::ClientFlakeIdGeneratorConfig> &config) {
            flakeIdGeneratorConfigMap.put(config->getName(), config);
            return *this;
        }

        const std::set<std::shared_ptr<MembershipListener> > &ClientConfig::getManagedMembershipListeners() const {
            return managedMembershipListeners;
        }

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif