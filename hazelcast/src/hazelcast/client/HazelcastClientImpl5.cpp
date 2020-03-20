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

        const std::string ClientProperties::IO_THREAD_COUNT = "hazelcast.client.io.thread.count";
        const std::string ClientProperties::IO_THREAD_COUNT_DEFAULT = "1";

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
                  ioThreadCount(IO_THREAD_COUNT, IO_THREAD_COUNT_DEFAULT),
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

        const ClientProperty &ClientProperties::getIOThreadCount() const {
            return ioThreadCount;
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
