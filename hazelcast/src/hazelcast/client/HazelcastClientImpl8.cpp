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


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
