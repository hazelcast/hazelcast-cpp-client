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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
