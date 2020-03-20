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


