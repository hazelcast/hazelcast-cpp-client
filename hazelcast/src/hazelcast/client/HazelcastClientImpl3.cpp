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


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
