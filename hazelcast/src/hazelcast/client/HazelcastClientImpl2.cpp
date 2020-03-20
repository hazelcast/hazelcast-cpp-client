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
        namespace connection {
            int ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC = 2;
            int ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC = 20;

            ClientConnectionManagerImpl::ClientConnectionManagerImpl(spi::ClientContext &client,
                                                                     const std::shared_ptr<AddressTranslator> &addressTranslator,
                                                                     const std::vector<std::shared_ptr<AddressProvider> > &addressProviders)
                    : logger(client.getLogger()), client(client),
                      socketInterceptor(client.getClientConfig().getSocketInterceptor()),
                      executionService(client.getClientExecutionService()),
                      translator(addressTranslator), connectionIdGen(0), socketFactory(client, ioContext) {
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

                ioThreadCount = clientProperties.getInteger(clientProperties.getIOThreadCount());
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

                for (int j = 0; j < ioThreadCount; ++j) {
                    ioThreads.emplace_back([&]() {
                        boost::asio::executor_work_guard<decltype(ioContext.get_executor())> work{
                                ioContext.get_executor()};
                        ioContext.run();
                    });
                }

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

                ioContext.stop();

                heartbeat->shutdown();

                connectionStrategy->shutdown();

                // let the waiting authentication futures not block anymore
                for (auto &authFutureTuple : connectionsInProgress.values()) {
                    auto &authFuture = std::get<0>(*authFutureTuple);
                    authFuture->onFailure(
                            std::make_shared<exception::IllegalStateException>("ClientConnectionManagerImpl::shutdown",
                                                                               "Client is shutting down"));
                    std::get<1>(*authFutureTuple)->close();
                }

                // close connections
                for (auto &connection : activeConnections.values()) {
                    // prevent any exceptions
                    util::IOUtil::closeResource(connection.get(), "Hazelcast client is shutting down");
                }

                spi::impl::ClientExecutionServiceImpl::shutdownExecutor("cluster", *clusterConnectionExecutor, logger);

                connectionListeners.clear();
                activeConnections.clear();

                std::for_each(ioThreads.begin(), ioThreads.end(), [](std::thread &t) { t.join(); });
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
                    if (connection == nullptr) {
                        return nullptr;
                    }
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

                    connection->reAuthenticateAsOwner();
                    return connection;
                }
                return std::shared_ptr<Connection>();
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::getActiveConnection(const Address &target) {
                return activeConnections.get(target);
            }

            std::shared_ptr<Address> ClientConnectionManagerImpl::getOwnerConnectionAddress() {
                return ownerConnectionAddress;
            }

            std::shared_ptr<ClientConnectionManagerImpl::FutureTuple>
            ClientConnectionManagerImpl::triggerConnect(const Address &target, bool asOwner) {
                if (!asOwner) {
                    connectionStrategy->beforeOpenConnection(target);
                }
                if (!alive) {
                    throw exception::HazelcastException("ConnectionManager::triggerConnect",
                                                        "ConnectionManager is not active!");
                }

                Address address = translator->translate(target);
                std::shared_ptr<AuthenticationFuture> future(new AuthenticationFuture(address, connectionsInProgress));

                auto connection = std::make_shared<Connection>(address, client, ++connectionIdGen, future,
                                                               socketFactory, ioContext, asOwner, *this,
                                                               connectionTimeoutMillis);

                auto authTuple = std::make_shared<FutureTuple>(future, connection);
                auto oldFutureTuple = connectionsInProgress.putIfAbsent(address, authTuple);
                if (oldFutureTuple.get() == NULL) {
                    // double check here
                    auto activeConnection = activeConnections.get(target);
                    if (activeConnection.get()) {
                        connectionsInProgress.remove(address);
                        return std::make_shared<FutureTuple>(nullptr, activeConnection);
                    }

                    connection->asyncStart();

                    return authTuple;
                }
                return oldFutureTuple;
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::getOrConnect(const Address &address, bool asOwner) {
                while (true) {
                    std::shared_ptr<Connection> connection = getConnection(address, asOwner);
                    if (connection.get() != NULL) {
                        return connection;
                    }
                    auto firstCallbackTuple = triggerConnect(address, asOwner);
                    auto firstCallback = std::get<0>(*firstCallbackTuple);
                    if (firstCallback == nullptr) {
                        auto activeConnection = std::get<1>(*firstCallbackTuple);
                        if (asOwner && !activeConnection->isAuthenticatedAsOwner()) {
                            activeConnection->reAuthenticateAsOwner();
                        }

                        return activeConnection;
                    }

                    if (!alive) {
                        std::get<1>(*firstCallbackTuple)->close("Client is being shutdown.");
                        firstCallback->onFailure(
                                std::make_shared<exception::IllegalStateException>(
                                        "ClientConnectionManagerImpl::getOrConnect",
                                        "Client is being shutdown."));
                        return nullptr;
                    }
                    connection = firstCallback->get();

                    // call the interceptor from user thread
                    if (socketInterceptor != NULL) {
                        socketInterceptor->onConnect(connection->getSocket());
                    }

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

            void
            ClientConnectionManagerImpl::reAuthenticate(const Address &target, std::shared_ptr<Connection> &connection,
                                                        bool asOwner, std::shared_ptr<AuthenticationFuture> &future) {
                future.reset(new AuthenticationFuture(target, connectionsInProgress));
                authenticate(target, connection, asOwner, future);
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

            bool ClientConnectionManagerImpl::isAlive() {
                return alive;
            }

            void ClientConnectionManagerImpl::onClose(Connection &connection) {
                removeFromActiveConnections(connection.shared_from_this());
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
            AuthenticationFuture::AuthenticationFuture(const Address &address,
                                                       util::SynchronizedMap<Address, FutureTuple> &connectionsInProgress)
                    : countDownLatch(new util::CountDownLatch(1)), address(address),
                      connectionsInProgress(connectionsInProgress), isSet(false) {
            }

            void AuthenticationFuture::onSuccess(const std::shared_ptr<Connection> &connection) {
                bool expected = false;
                if (!isSet.compare_exchange_strong(expected, true)) {
                    return;
                }
                this->connection = connection;
                countDownLatch->countDown();
            }

            void AuthenticationFuture::onFailure(const std::shared_ptr<exception::IException> &throwable) {
                bool expected = false;
                if (!isSet.compare_exchange_strong(expected, true)) {
                    return;
                }
                connectionsInProgress.remove(address);
                this->throwable = throwable;
                countDownLatch->countDown();
            }

            std::shared_ptr<Connection> AuthenticationFuture::get() {
                countDownLatch->await();
                auto connPtr = connection.get();
                if (connPtr.get() != NULL) {
                    return connPtr;
                }

                auto exceptionPtr = throwable.get();
                assert(exceptionPtr.get() != NULL);
                throw exception::ExecutionException("AuthenticationFuture::get", "Could not be authenticated.",
                                                    exceptionPtr);
            }

        }
    }
}




//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {
            ReadHandler::ReadHandler(Connection &connection, size_t bufferSize)
                    : buffer(new char[bufferSize]), byteBuffer(buffer, bufferSize), builder(connection) {
                lastReadTimeMillis = util::currentTimeMillis();
            }

            ReadHandler::~ReadHandler() {
                delete[] buffer;
            }

            void ReadHandler::handle() {
                lastReadTimeMillis = util::currentTimeMillis();

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
            Connection::Connection(const Address &address, spi::ClientContext &clientContext, int connectionId,
                                   const std::shared_ptr<AuthenticationFuture> &authFuture,
                                   internal::socket::SocketFactory &socketFactory, boost::asio::io_context &ioContext,
                                   bool asOwner,
                                   ClientConnectionManagerImpl &clientConnectionManager, int64_t connectTimeoutInMillis)
                    : readHandler(*this, 16 << 10),
                      startTimeInMillis(util::currentTimeMillis()), closedTimeMillis(0),
                      clientContext(clientContext),
                      invocationService(clientContext.getInvocationService()),
                      authFuture(authFuture),
                      connectionId(connectionId),
                      connectedServerVersion(impl::BuildInfo::UNKNOWN_HAZELCAST_VERSION),
                      logger(clientContext.getLogger()), asOwner(asOwner),
                      connectionManager(clientConnectionManager) {
                socket = socketFactory.create(address, connectTimeoutInMillis);
            }

            Connection::~Connection() {
            }

            void Connection::authenticate() {
                auto thisConnection = shared_from_this();
                connectionManager.authenticate(socket->getRemoteEndpoint(), thisConnection, asOwner, authFuture);
            }

            void Connection::reAuthenticateAsOwner() {
                asOwner = true;
                auto thisConnection = shared_from_this();
                connectionManager.reAuthenticate(socket->getRemoteEndpoint(), thisConnection, asOwner, authFuture);
                authFuture->get();
            }

            void Connection::asyncStart() {
                socket->asyncStart(shared_from_this(), authFuture);
            }

            void Connection::close() {
                close("");
            }

            void Connection::close(const std::string &reason) {
                close(reason, std::shared_ptr<exception::IException>());
            }

            void Connection::close(const std::string &reason, const std::shared_ptr<exception::IException> &cause) {
                int64_t expected = 0;
                if (!closedTimeMillis.compare_exchange_strong(expected, util::currentTimeMillis())) {
                    return;
                }

                closeCause = cause;
                closeReason = reason;

                logClose();

                try {
                    innerClose();
                } catch (exception::IException &e) {
                    clientContext.getLogger().warning("Exception while closing connection", e.getMessage());
                }

                clientContext.getConnectionManager().onClose(*this);
            }

            bool Connection::write(const std::shared_ptr<protocol::ClientMessage> &message) {
                socket->asyncWrite(shared_from_this(), message);
                return true;
            }

            const std::shared_ptr<Address> &Connection::getRemoteEndpoint() const {
                return remoteEndpoint;
            }

            void Connection::setRemoteEndpoint(const std::shared_ptr<Address> &remoteEndpoint) {
                this->remoteEndpoint = remoteEndpoint;
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

            const Socket &Connection::getSocket() const {
                return *socket;
            }
        }
    }
}





//#define BOOST_THREAD_PROVIDES_FUTURE

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

        Address::Address() : host("localhost"), type(IPV4), scopeId(0) {
        }

        Address::Address(const std::string &url, int port)
                : host(url), port(port), type(IPV4), scopeId(0) {
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif