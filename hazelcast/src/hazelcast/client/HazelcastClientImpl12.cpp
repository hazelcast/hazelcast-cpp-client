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
                SocketFactory::SocketFactory(spi::ClientContext &clientContext, boost::asio::io_context &io)
                        : clientContext(clientContext), io(io) {
                }

                bool SocketFactory::start() {
#ifdef HZ_BUILD_WITH_SSL
                    const client::config::SSLConfig &sslConfig = clientContext.getClientConfig().getNetworkConfig().getSSLConfig();
                    if (sslConfig.isEnabled()) {
                        sslContext = std::unique_ptr<boost::asio::ssl::context>(new boost::asio::ssl::context(
                                (boost::asio::ssl::context_base::method) sslConfig.getProtocol()));

                        const std::vector<std::string> &verifyFiles = sslConfig.getVerifyFiles();
                        bool success = true;
                        util::ILogger &logger = clientContext.getLogger();
                        for (std::vector<std::string>::const_iterator it = verifyFiles.begin(); it != verifyFiles.end();
                             ++it) {
                            boost::system::error_code ec;
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

                std::unique_ptr<Socket> SocketFactory::create(const Address &address, int64_t connectTimeoutInMillis) {
#ifdef HZ_BUILD_WITH_SSL
                    if (sslContext.get()) {
                        return std::unique_ptr<Socket>(new internal::socket::SSLSocket(io, *sslContext, address,
                                                                                       clientContext.getClientConfig().getNetworkConfig().getSocketOptions(),
                                                                                       connectTimeoutInMillis));
                    }
#endif

                    return std::unique_ptr<Socket>(new internal::socket::TcpSocket(io, address,
                                                                                   clientContext.getClientConfig().getNetworkConfig().getSocketOptions(),
                                                                                   connectTimeoutInMillis));
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
                SSLSocket::SSLSocket(boost::asio::io_context &ioService, boost::asio::ssl::context &sslContext,
                                     const client::Address &address, client::config::SocketOptions &socketOptions,
                                     int64_t connectTimeoutInMillis)
                        : BaseSocket<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(
                        std::make_unique<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(ioService, sslContext),
                        address, socketOptions, ioService, connectTimeoutInMillis) {
                }

                std::vector<SSLSocket::CipherInfo> SSLSocket::getCiphers() const {
                    STACK_OF(SSL_CIPHER) *ciphers = SSL_get_ciphers(socket_->native_handle());
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

                void SSLSocket::async_handle_connect(const std::shared_ptr<connection::Connection> &connection,
                                                     const std::shared_ptr<connection::AuthenticationFuture> &authFuture) {
                    socket_->async_handshake(boost::asio::ssl::stream_base::client,
                                             [=](const boost::system::error_code &ec) {
                                                 if (ec) {
                                                     authFuture->onFailure(std::make_shared<exception::IOException>(
                                                             "Connection::do_connect", (boost::format(
                                                                     "Handshake with server %1% failed. %2%") %
                                                                                        remoteEndpoint % ec).str()));
                                                     return;
                                                 }

                                                 BaseSocket<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>::async_handle_connect(
                                                         connection, authFuture);
                                             });
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
                TcpSocket::TcpSocket(boost::asio::io_context &io, const Address &address,
                                     client::config::SocketOptions &socketOptions, int64_t connectTimeoutInMillis)
                        : BaseSocket<boost::asio::ip::tcp::socket>(std::make_unique<boost::asio::ip::tcp::socket>(io),
                                                                   address, socketOptions, io, connectTimeoutInMillis) {
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
