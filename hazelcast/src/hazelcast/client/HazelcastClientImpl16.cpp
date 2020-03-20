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


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
