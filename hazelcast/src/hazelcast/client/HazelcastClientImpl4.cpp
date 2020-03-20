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


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
