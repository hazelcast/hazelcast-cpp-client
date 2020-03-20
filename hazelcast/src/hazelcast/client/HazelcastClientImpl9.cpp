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


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
