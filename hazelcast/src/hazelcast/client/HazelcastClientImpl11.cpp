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


using namespace hazelcast::util;
using namespace hazelcast::client::serialization;
using namespace std;


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                PortableContext::PortableContext(const SerializationConfig &serializationConf) :
                        serializationConfig(serializationConf),
                        serializerHolder(serializationConf.getGlobalSerializer()) {
                }

                int PortableContext::getClassVersion(int factoryId, int classId) {
                    return getClassDefinitionContext(factoryId).getClassVersion(classId);
                }

                void PortableContext::setClassVersion(int factoryId, int classId, int version) {
                    getClassDefinitionContext(factoryId).setClassVersion(classId, version);
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::lookupClassDefinition(int factoryId, int classId, int version) {
                    return getClassDefinitionContext(factoryId).lookup(classId, version);
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::readClassDefinition(ObjectDataInput &in, int factoryId, int classId, int version) {
                    bool shouldRegister = true;
                    ClassDefinitionBuilder builder(factoryId, classId, version);

                    // final position after portable is read
                    in.readInt();

                    // field count
                    int fieldCount = in.readInt();
                    int offset = in.position();
                    for (int i = 0; i < fieldCount; i++) {
                        in.position(offset + i * Bits::INT_SIZE_IN_BYTES);
                        int pos = in.readInt();
                        in.position(pos);

                        short len = in.readShort();
                        vector<byte> chars(len);
                        in.readFully(chars);
                        chars.push_back('\0');

                        FieldType type(in.readByte());
                        std::string name((char *) &(chars[0]));
                        int fieldFactoryId = 0;
                        int fieldClassId = 0;
                        int fieldVersion = version;
                        if (type == FieldTypes::TYPE_PORTABLE) {
                            // is null
                            if (in.readBoolean()) {
                                shouldRegister = false;
                            }
                            fieldFactoryId = in.readInt();
                            fieldClassId = in.readInt();

                            // TODO: what if there's a null inner Portable field
                            if (shouldRegister) {
                                fieldVersion = in.readInt();
                                readClassDefinition(in, fieldFactoryId, fieldClassId, fieldVersion);
                            }
                        } else if (type == FieldTypes::TYPE_PORTABLE_ARRAY) {
                            int k = in.readInt();
                            if (k > 0) {
                                fieldFactoryId = in.readInt();
                                fieldClassId = in.readInt();

                                int p = in.readInt();
                                in.position(p);

                                // TODO: what if there's a null inner Portable field
                                fieldVersion = in.readInt();
                                readClassDefinition(in, fieldFactoryId, fieldClassId, fieldVersion);
                            } else {
                                shouldRegister = false;
                            }

                        }
                        FieldDefinition fieldDef(i, name, type, fieldFactoryId, fieldClassId, fieldVersion);
                        builder.addField(fieldDef);
                    }
                    std::shared_ptr<ClassDefinition> classDefinition = builder.build();
                    if (shouldRegister) {
                        classDefinition = registerClassDefinition(classDefinition);
                    }
                    return classDefinition;
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::registerClassDefinition(std::shared_ptr<ClassDefinition> cd) {
                    return getClassDefinitionContext(cd->getFactoryId()).registerClassDefinition(cd);
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::lookupOrRegisterClassDefinition(const Portable &portable) {
                    int portableVersion = PortableVersionHelper::getVersion(&portable,
                                                                            serializationConfig.getPortableVersion());
                    std::shared_ptr<ClassDefinition> cd = lookupClassDefinition(portable.getFactoryId(),
                                                                                portable.getClassId(), portableVersion);
                    if (cd.get() == NULL) {
                        ClassDefinitionBuilder classDefinitionBuilder(portable.getFactoryId(), portable.getClassId(),
                                                                      portableVersion);
                        ClassDefinitionWriter cdw(*this, classDefinitionBuilder);
                        PortableWriter portableWriter(&cdw);
                        portable.writePortable(portableWriter);
                        cd = cdw.registerAndGet();
                    }
                    return cd;
                }

                int PortableContext::getVersion() {
                    return serializationConfig.getPortableVersion();
                }

                SerializerHolder &PortableContext::getSerializerHolder() {
                    return serializerHolder;
                }

                ClassDefinitionContext &PortableContext::getClassDefinitionContext(int factoryId) {
                    std::shared_ptr<ClassDefinitionContext> value = classDefContextMap.get(factoryId);
                    if (value == NULL) {
                        value = std::shared_ptr<ClassDefinitionContext>(new ClassDefinitionContext(factoryId, this));
                        std::shared_ptr<ClassDefinitionContext> current = classDefContextMap.putIfAbsent(factoryId,
                                                                                                         value);
                        if (current != NULL) {
                            value = current;
                        }
                    }
                    return *value;
                }

                const SerializationConfig &PortableContext::getSerializationConfig() const {
                    return serializationConfig;
                }

            }
        }
    }
}



//
//  SerializationService.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                ObjectType::ObjectType() : typeId(0), factoryId(-1), classId(-1) {}

                std::ostream &operator<<(std::ostream &os, const ObjectType &type) {
                    os << "typeId: " << type.typeId << " factoryId: " << type.factoryId << " classId: "
                       << type.classId;
                    return os;
                }

                SerializationService::SerializationService(const SerializationConfig &serializationConfig)
                        : portableContext(serializationConfig),
                          serializationConfig(serializationConfig) {
                    registerConstantSerializers();

                    std::vector<std::shared_ptr<SerializerBase> > const &serializers = serializationConfig.getSerializers();
                    for (std::vector<std::shared_ptr<SerializerBase> >::const_iterator it = serializers.begin();
                         it < serializers.end(); ++it) {
                        registerSerializer(std::static_pointer_cast<StreamSerializer>(*it));
                    }
                }

                SerializerHolder &SerializationService::getSerializerHolder() {
                    return portableContext.getSerializerHolder();
                }

                bool SerializationService::registerSerializer(std::shared_ptr<StreamSerializer> serializer) {
                    return getSerializerHolder().registerSerializer(serializer);
                }

                bool SerializationService::isNullData(const Data &data) {
                    return data.dataSize() == 0 && data.getType() == SerializationConstants::CONSTANT_TYPE_NULL;
                }

                const byte SerializationService::getVersion() const {
                    return 1;
                }

                ObjectType SerializationService::getObjectType(const Data *data) {
                    ObjectType type;

                    if (NULL == data) {
                        return type;
                    }

                    type.typeId = data->getType();

                    // Constant 4 is Data::TYPE_OFFSET. Windows DLL export does not
                    // let usage of static member.
                    DataInput dataInput(data->toByteArray(), 4);

                    ObjectDataInput objectDataInput(dataInput, getSerializerHolder());

                    if (SerializationConstants::CONSTANT_TYPE_DATA == type.typeId ||
                        SerializationConstants::CONSTANT_TYPE_PORTABLE == type.typeId) {
                        int32_t objectTypeId = objectDataInput.readInt();
                        assert(type.typeId == objectTypeId);

                        if (SerializationConstants::CONSTANT_TYPE_DATA == type.typeId) {
                            bool identified = objectDataInput.readBoolean();
                            if (!identified) {
                                throw exception::HazelcastSerializationException("SerializationService::getObjectType",
                                                                                 " DataSerializable is not identified");
                            }
                        }

                        type.factoryId = objectDataInput.readInt();
                        type.classId = objectDataInput.readInt();
                    }

                    return type;
                }

                void SerializationService::registerConstantSerializers() {
                    registerSerializer(std::shared_ptr<StreamSerializer>(new NullSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new DataSerializer(serializationConfig)));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new PortableSerializer(portableContext)));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new HazelcastJsonValueSerializer()));
                    //primitives and String
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::ByteSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::BooleanSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::CharSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::ShortSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::IntegerSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::LongSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::FloatSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::DoubleSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::StringSerializer));
                    //Arrays of primitives and String
                    registerSerializer(std::shared_ptr<StreamSerializer>(new TheByteArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new BooleanArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new CharArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new ShortArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new IntegerArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new LongArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new FloatArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new DoubleArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new StringArraySerializer()));
                }

                void SerializationService::dispose() {
                    getSerializerHolder().dispose();
                }

                template<>
                Data SerializationService::toData(const TypedData *object) {
                    if (!object) {
                        return Data();
                    }

                    const std::shared_ptr<Data> data = object->getData();
                    if ((Data *) NULL == data.get()) {
                        return Data();
                    }

                    return Data(*data);
                }

            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                size_t const DataOutput::DEFAULT_SIZE = 4 * 1024;

                DataOutput::DataOutput()
                        : outputStream(new std::vector<byte>()) {
                    outputStream->reserve(DEFAULT_SIZE);
                }


                DataOutput::~DataOutput() {
                }

                DataOutput::DataOutput(DataOutput const &rhs) {
                    //private
                }

                DataOutput &DataOutput::operator=(DataOutput const &rhs) {
                    //private
                    return *this;
                }

                std::unique_ptr<std::vector<byte> > DataOutput::toByteArray() {
                    std::unique_ptr<std::vector<byte> > byteArrayPtr(new std::vector<byte>(*outputStream));
                    return byteArrayPtr;
                }

                void DataOutput::write(const std::vector<byte> &bytes) {
                    outputStream->insert(outputStream->end(), bytes.begin(), bytes.end());
                }

                void DataOutput::writeBoolean(bool i) {
                    writeByte((byte) i);
                }

                void DataOutput::writeByte(int index, int32_t i) {
                    (*outputStream)[index] = byte(0xff & i);
                }

                void DataOutput::writeByte(int32_t i) {
                    outputStream->push_back(byte(0xff & i));
                }

                void DataOutput::writeShort(int32_t v) {
                    int16_t value = (int16_t) v;
                    int16_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian2(&value, target);
                    outputStream->insert(outputStream->end(), target, target + util::Bits::SHORT_SIZE_IN_BYTES);
                }

                void DataOutput::writeChar(int32_t i) {
                    writeByte((byte) (i >> 8));
                    writeByte((byte) i);
                }

                void DataOutput::writeInt(int32_t v) {
                    int32_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian4(&v, target);
                    outputStream->insert(outputStream->end(), target, target + util::Bits::INT_SIZE_IN_BYTES);
                }

                void DataOutput::writeLong(int64_t l) {
                    int64_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian8(&l, target);
                    outputStream->insert(outputStream->end(), target, target + util::Bits::LONG_SIZE_IN_BYTES);
                }

                void DataOutput::writeFloat(float x) {
                    union {
                        float f;
                        int32_t i;
                    } u;
                    u.f = x;
                    writeInt(u.i);
                }

                void DataOutput::writeDouble(double v) {
                    union {
                        double d;
                        int64_t l;
                    } u;
                    u.d = v;
                    writeLong(u.l);
                }

                void DataOutput::writeUTF(const std::string *str) {
                    int32_t len = util::Bits::NULL_ARRAY;
                    if (str) {
                        len = util::UTFUtil::isValidUTF8(*str);
                        if (len < 0) {
                            throw (exception::ExceptionBuilder<exception::UTFDataFormatException>(
                                    "DataOutput::writeUTF")
                                    << "String \"" << (*str) << "\" is not UTF-8 formatted !!!").build();
                        }
                    }

                    writeInt(len);
                    if (len > 0) {
                        outputStream->insert(outputStream->end(), str->begin(), str->end());
                    }
                }

                void DataOutput::writeInt(int index, int32_t v) {
                    int32_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian4(&v, &result);
                    (*outputStream)[index++] = *(target++);
                    (*outputStream)[index++] = *(target++);
                    (*outputStream)[index++] = *(target++);
                    (*outputStream)[index] = *target;
                }

                void DataOutput::writeBytes(const byte *bytes, size_t len) {
                    outputStream->insert(outputStream->end(), bytes, bytes + len);
                }

                void DataOutput::writeByteArray(const std::vector<byte> *data) {
                    writeArray<byte>(data);
                }

                void DataOutput::writeCharArray(const std::vector<char> *data) {
                    writeArray<char>(data);
                }

                void DataOutput::writeBooleanArray(const std::vector<bool> *data) {
                    writeArray<bool>(data);
                }

                void DataOutput::writeShortArray(const std::vector<int16_t> *data) {
                    writeArray<int16_t>(data);
                }

                void DataOutput::writeIntArray(const std::vector<int32_t> *data) {
                    writeArray<int32_t>(data);
                }

                void DataOutput::writeLongArray(const std::vector<int64_t> *data) {
                    writeArray<int64_t>(data);
                }

                void DataOutput::writeFloatArray(const std::vector<float> *data) {
                    writeArray<float>(data);
                }

                void DataOutput::writeDoubleArray(const std::vector<double> *data) {
                    writeArray<double>(data);
                }

                void DataOutput::writeUTFArray(const std::vector<std::string> *data) {
                    writeArray<std::string>(data);
                }

                void DataOutput::writeZeroBytes(int numberOfBytes) {
                    for (int k = 0; k < numberOfBytes; k++) {
                        writeByte(0);
                    }
                }

                size_t DataOutput::position() {
                    return outputStream->size();
                }

                void DataOutput::position(size_t newPos) {
                    if (outputStream->size() < newPos)
                        outputStream->resize(newPos, 0);
                }

                int DataOutput::getUTF8CharCount(const std::string &str) {
                    int size = 0;
                    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
                        // Any additional byte for an UTF character has a bit mask of 10xxxxxx
                        size += (*it & 0xC0) != 0x80;
                    }

                    return size;
                }

                template<>
                void DataOutput::write(const byte &value) {
                    writeByte(value);
                }

                template<>
                void DataOutput::write(const char &value) {
                    writeChar(value);
                }

                template<>
                void DataOutput::write(const bool &value) {
                    writeBoolean(value);
                }

                template<>
                void DataOutput::write(const int16_t &value) {
                    writeShort(value);
                }

                template<>
                void DataOutput::write(const int32_t &value) {
                    writeInt(value);
                }

                template<>
                void DataOutput::write(const int64_t &value) {
                    writeLong(value);
                }

                template<>
                void DataOutput::write(const float &value) {
                    writeFloat(value);
                }

                template<>
                void DataOutput::write(const double &value) {
                    writeDouble(value);
                }

                template<>
                void DataOutput::write(const std::string &value) {
                    writeUTF(&value);
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DataSerializer::DataSerializer(const SerializationConfig &serializationConfig)
                        : serializationConfig(serializationConfig) {
                }

                DataSerializer::~DataSerializer() {
                }

                void DataSerializer::checkIfIdentifiedDataSerializable(ObjectDataInput &in) const {
                    bool identified = in.readBoolean();
                    if (!identified) {
                        throw exception::HazelcastSerializationException("void DataSerializer::read",
                                                                         " DataSerializable is not identified");
                    }
                }

                int32_t DataSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DATA;
                }

                void DataSerializer::write(ObjectDataOutput &out, const IdentifiedDataSerializable *dataSerializable) {
                    out.writeBoolean(true);
                    out.writeInt(dataSerializable->getFactoryId());
                    out.writeInt(dataSerializable->getClassId());
                    dataSerializable->writeData(out);
                }

                void DataSerializer::write(ObjectDataOutput &out, const void *object) {
                    // should not be called
                    assert(0);
                }

                void *DataSerializer::read(ObjectDataInput &in) {
                    // should not be called
                    assert(0);
                    return NULL;
                }

                int32_t DataSerializer::readInt(ObjectDataInput &in) const {
                    return in.readInt();
                }
            }
        }
    }
}


//
//  PortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DefaultPortableReader::DefaultPortableReader(PortableContext &portableContext,
                                                             ObjectDataInput &input,
                                                             std::shared_ptr<ClassDefinition> cd)
                        : PortableReaderBase(portableContext, input, cd) {
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                ClassDefinitionContext::ClassDefinitionContext(int factoryId, PortableContext *portableContext)
                        : factoryId(factoryId), portableContext(portableContext) {
                }

                int ClassDefinitionContext::getClassVersion(int classId) {
                    std::shared_ptr<int> version = currentClassVersions.get(classId);
                    return version != NULL ? *version : -1;
                }

                void ClassDefinitionContext::setClassVersion(int classId, int version) {
                    std::shared_ptr<int> current = currentClassVersions.putIfAbsent(classId, std::shared_ptr<int>(
                            new int(version)));
                    if (current != NULL && *current != version) {
                        std::stringstream error;
                        error << "Class-id: " << classId << " is already registered!";
                        throw exception::IllegalArgumentException("ClassDefinitionContext::setClassVersion",
                                                                  error.str());
                    }
                }

                std::shared_ptr<ClassDefinition> ClassDefinitionContext::lookup(int classId, int version) {
                    long long key = combineToLong(classId, version);
                    return versionedDefinitions.get(key);

                }

                std::shared_ptr<ClassDefinition>
                ClassDefinitionContext::registerClassDefinition(std::shared_ptr<ClassDefinition> cd) {
                    if (cd.get() == NULL) {
                        return std::shared_ptr<ClassDefinition>();
                    }
                    if (cd->getFactoryId() != factoryId) {
                        throw (exception::ExceptionBuilder<exception::HazelcastSerializationException>(
                                "ClassDefinitionContext::registerClassDefinition") << "Invalid factory-id! "
                                                                                   << factoryId << " -> "
                                                                                   << cd).build();
                    }

                    cd->setVersionIfNotSet(portableContext->getVersion());

                    long long versionedClassId = combineToLong(cd->getClassId(), cd->getVersion());
                    std::shared_ptr<ClassDefinition> currentCd = versionedDefinitions.putIfAbsent(versionedClassId, cd);
                    if (currentCd.get() == NULL) {
                        return cd;
                    }

                    if (currentCd.get() != cd.get() && *currentCd != *cd) {
                        throw (exception::ExceptionBuilder<exception::HazelcastSerializationException>(
                                "ClassDefinitionContext::registerClassDefinition")
                                << "Incompatible class-definitions with same class-id: " << *cd << " VS "
                                << *currentCd).build();
                    }

                    return currentCd;
                }

                int64_t ClassDefinitionContext::combineToLong(int x, int y) const {
                    return ((int64_t) x) << 32 | (((int64_t) y) & 0xFFFFFFFL);
                }

            }
        }
    }
}

//
//  PortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DefaultPortableWriter::DefaultPortableWriter(PortableContext &portableContext,
                                                             std::shared_ptr<ClassDefinition> cd,
                                                             ObjectDataOutput &output)
                        : raw(false), serializerHolder(portableContext.getSerializerHolder()),
                          dataOutput(*output.getDataOutput()), objectDataOutput(output), begin(dataOutput.position()),
                          cd(cd) {
                    // room for final offset
                    dataOutput.writeZeroBytes(4);

                    objectDataOutput.writeInt(cd->getFieldCount());

                    offset = dataOutput.position();
                    // one additional for raw data
                    int fieldIndexesLength = (cd->getFieldCount() + 1) * util::Bits::INT_SIZE_IN_BYTES;
                    dataOutput.writeZeroBytes(fieldIndexesLength);
                }

                void DefaultPortableWriter::writeInt(const char *fieldName, int32_t value) {
                    setPosition(fieldName, FieldTypes::TYPE_INT);
                    dataOutput.writeInt(value);
                }

                void DefaultPortableWriter::writeLong(const char *fieldName, int64_t value) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG);
                    dataOutput.writeLong(value);
                }

                void DefaultPortableWriter::writeBoolean(const char *fieldName, bool value) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN);
                    dataOutput.writeBoolean(value);
                }

                void DefaultPortableWriter::writeByte(const char *fieldName, byte value) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE);
                    dataOutput.writeByte(value);
                }

                void DefaultPortableWriter::writeChar(const char *fieldName, int32_t value) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR);
                    dataOutput.writeChar(value);
                }

                void DefaultPortableWriter::writeDouble(const char *fieldName, double value) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE);
                    dataOutput.writeDouble(value);
                }

                void DefaultPortableWriter::writeFloat(const char *fieldName, float value) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT);
                    dataOutput.writeFloat(value);
                }

                void DefaultPortableWriter::writeShort(const char *fieldName, int32_t value) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT);
                    dataOutput.writeShort(value);
                }

                void DefaultPortableWriter::writeUTF(const char *fieldName, const std::string *value) {
                    setPosition(fieldName, FieldTypes::TYPE_UTF);
                    dataOutput.writeUTF(value);
                }

                void DefaultPortableWriter::writeByteArray(const char *fieldName, const std::vector<byte> *bytes) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                    dataOutput.writeByteArray(bytes);
                }

                void DefaultPortableWriter::writeBooleanArray(const char *fieldName, const std::vector<bool> *bytes) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN_ARRAY);
                    dataOutput.writeBooleanArray(bytes);
                }

                void DefaultPortableWriter::writeCharArray(const char *fieldName, const std::vector<char> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                    dataOutput.writeCharArray(data);
                }

                void DefaultPortableWriter::writeShortArray(const char *fieldName, const std::vector<int16_t> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                    dataOutput.writeShortArray(data);
                }

                void DefaultPortableWriter::writeIntArray(const char *fieldName, const std::vector<int32_t> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_INT_ARRAY);
                    dataOutput.writeIntArray(data);
                }

                void DefaultPortableWriter::writeLongArray(const char *fieldName, const std::vector<int64_t> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG_ARRAY);
                    dataOutput.writeLongArray(data);
                }

                void DefaultPortableWriter::writeFloatArray(const char *fieldName, const std::vector<float> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                    dataOutput.writeFloatArray(data);
                }

                void DefaultPortableWriter::writeDoubleArray(const char *fieldName, const std::vector<double> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                    dataOutput.writeDoubleArray(data);
                }

                FieldDefinition const &DefaultPortableWriter::setPosition(const char *fieldName, FieldType fieldType) {
                    if (raw) {
                        throw exception::HazelcastSerializationException("PortableWriter::setPosition",
                                                                         "Cannot write Portable fields after getRawDataOutput() is called!");
                    }

                    try {
                        FieldDefinition const &fd = cd->getField(fieldName);

                        if (writtenFields.count(fieldName) != 0) {
                            throw exception::HazelcastSerializationException("PortableWriter::setPosition",
                                                                             "Field '" + std::string(fieldName) +
                                                                             "' has already been written!");
                        }

                        writtenFields.insert(fieldName);
                        size_t pos = dataOutput.position();
                        int32_t index = fd.getIndex();
                        dataOutput.writeInt((int32_t) (offset + index * util::Bits::INT_SIZE_IN_BYTES), (int32_t) pos);
                        size_t nameLen = strlen(fieldName);
                        dataOutput.writeShort(nameLen);
                        dataOutput.writeBytes((byte *) fieldName, nameLen);
                        dataOutput.writeByte(fieldType.getId());

                        return fd;

                    } catch (exception::IllegalArgumentException &iae) {
                        std::stringstream error;
                        error << "HazelcastSerializationException( Invalid field name: '" << fieldName;
                        error << "' for ClassDefinition {class id: " << util::IOUtil::to_string(cd->getClassId());
                        error << ", factoryId:" + util::IOUtil::to_string(cd->getFactoryId());
                        error << ", version: " << util::IOUtil::to_string(cd->getVersion()) << "}. Error:";
                        error << iae.what();

                        throw exception::HazelcastSerializationException("PortableWriter::setPosition", error.str());
                    }

                }


                ObjectDataOutput &DefaultPortableWriter::getRawDataOutput() {
                    if (!raw) {
                        size_t pos = dataOutput.position();
                        int32_t index = cd->getFieldCount(); // last index
                        dataOutput.writeInt((int32_t) (offset + index * util::Bits::INT_SIZE_IN_BYTES), (int32_t) pos);
                    }
                    raw = true;
                    return objectDataOutput;
                }

                void DefaultPortableWriter::end() {
                    dataOutput.writeInt((int32_t) begin, (int32_t) dataOutput.position()); // write final offset
                }

                void DefaultPortableWriter::write(const Portable &p) {
                    std::shared_ptr<PortableSerializer> serializer = std::static_pointer_cast<PortableSerializer>(
                            serializerHolder.serializerFor(SerializationConstants::CONSTANT_TYPE_PORTABLE));
                    serializer->writeInternal(objectDataOutput, &p);
                }


                void
                DefaultPortableWriter::checkPortableAttributes(const FieldDefinition &fd, const Portable &portable) {
                    if (fd.getFactoryId() != portable.getFactoryId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                                     << " Expected factory-id: " << fd.getFactoryId() << ", Actual factory-id: "
                                     << portable.getFactoryId();
                        throw exception::HazelcastSerializationException(
                                "DefaultPortableWriter::::checkPortableAttributes", errorMessage.str());
                    }
                    if (fd.getClassId() != portable.getClassId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                                     << "Expected class-id: " << fd.getClassId() << ", Actual class-id: "
                                     << portable.getClassId();
                        throw exception::HazelcastSerializationException(
                                "DefaultPortableWriter::::checkPortableAttributes", errorMessage.str());
                    }
                }
            }
        }
    }
}
//
//  ClassDefinitionWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                ClassDefinitionWriter::ClassDefinitionWriter(PortableContext &portableContext,
                                                             ClassDefinitionBuilder &builder)
                        : builder(builder), context(portableContext) {
                }

                std::shared_ptr<ClassDefinition> ClassDefinitionWriter::registerAndGet() {
                    std::shared_ptr<ClassDefinition> cd = builder.build();
                    return context.registerClassDefinition(cd);
                }

                void ClassDefinitionWriter::writeInt(const char *fieldName, int32_t value) {
                    builder.addIntField(fieldName);
                }

                void ClassDefinitionWriter::writeLong(const char *fieldName, int64_t value) {
                    builder.addLongField(fieldName);
                }

                void ClassDefinitionWriter::writeBoolean(const char *fieldName, bool value) {

                    builder.addBooleanField(fieldName);
                }

                void ClassDefinitionWriter::writeByte(const char *fieldName, byte value) {
                    builder.addByteField(fieldName);
                }

                void ClassDefinitionWriter::writeChar(const char *fieldName, int32_t value) {
                    builder.addCharField(fieldName);
                }

                void ClassDefinitionWriter::writeDouble(const char *fieldName, double value) {

                    builder.addDoubleField(fieldName);
                }

                void ClassDefinitionWriter::writeFloat(const char *fieldName, float value) {
                    builder.addFloatField(fieldName);
                }

                void ClassDefinitionWriter::writeShort(const char *fieldName, int16_t value) {
                    builder.addShortField(fieldName);
                }

                void ClassDefinitionWriter::writeUTF(const char *fieldName, const std::string *value) {

                    builder.addUTFField(fieldName);
                }

                void ClassDefinitionWriter::writeByteArray(const char *fieldName, const std::vector<byte> *values) {
                    builder.addByteArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeBooleanArray(const char *fieldName, const std::vector<bool> *values) {
                    builder.addBooleanArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeCharArray(const char *fieldName, const std::vector<char> *values) {
                    builder.addCharArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeIntArray(const char *fieldName, const std::vector<int32_t> *values) {
                    builder.addIntArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeLongArray(const char *fieldName, const std::vector<int64_t> *values) {
                    builder.addLongArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeDoubleArray(const char *fieldName, const std::vector<double> *values) {
                    builder.addDoubleArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeFloatArray(const char *fieldName, const std::vector<float> *values) {
                    builder.addFloatArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeShortArray(const char *fieldName, const std::vector<int16_t> *values) {
                    builder.addShortArrayField(fieldName);
                }

                ObjectDataOutput &ClassDefinitionWriter::getRawDataOutput() {
                    return emptyDataOutput;
                }

                void ClassDefinitionWriter::end() {

                }

                std::shared_ptr<ClassDefinition> ClassDefinitionWriter::createNestedClassDef(const Portable &portable) {
                    int version = pimpl::PortableVersionHelper::getVersion(&portable, context.getVersion());
                    ClassDefinitionBuilder definitionBuilder(portable.getFactoryId(), portable.getClassId(), version);

                    ClassDefinitionWriter nestedWriter(context, definitionBuilder);
                    PortableWriter portableWriter(&nestedWriter);
                    portable.writePortable(portableWriter);
                    return context.registerClassDefinition(definitionBuilder.build());
                }
            }
        }
    }
}
//
//  PortableSerializer.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                PortableSerializer::PortableSerializer(PortableContext &portableContext)
                        : context(portableContext) {
                }

                void
                PortableSerializer::read(ObjectDataInput &in, Portable &portable, int32_t factoryId, int32_t classId) {
                    int version = in.readInt();

                    int portableVersion = findPortableVersion(factoryId, classId, portable);

                    PortableReader reader = createReader(in, factoryId, classId, version, portableVersion);
                    portable.readPortable(reader);
                    reader.end();
                }

                PortableReader
                PortableSerializer::createReader(ObjectDataInput &input, int factoryId, int classId, int version,
                                                 int portableVersion) const {

                    int effectiveVersion = version;
                    if (version < 0) {
                        effectiveVersion = context.getVersion();
                    }

                    std::shared_ptr<ClassDefinition> cd = context.lookupClassDefinition(factoryId, classId,
                                                                                        effectiveVersion);
                    if (cd == NULL) {
                        int begin = input.position();
                        cd = context.readClassDefinition(input, factoryId, classId, effectiveVersion);
                        input.position(begin);
                    }

                    if (portableVersion == effectiveVersion) {
                        PortableReader reader(context, input, cd, true);
                        return reader;
                    } else {
                        PortableReader reader(context, input, cd, false);
                        return reader;
                    }
                }

                int
                PortableSerializer::findPortableVersion(int factoryId, int classId, const Portable &portable) const {
                    int currentVersion = context.getClassVersion(factoryId, classId);
                    if (currentVersion < 0) {
                        currentVersion = PortableVersionHelper::getVersion(&portable, context.getVersion());
                        if (currentVersion > 0) {
                            context.setClassVersion(factoryId, classId, currentVersion);
                        }
                    }
                    return currentVersion;
                }

                std::unique_ptr<Portable>
                PortableSerializer::createNewPortableInstance(int32_t factoryId, int32_t classId) {
                    const std::map<int32_t, std::shared_ptr<PortableFactory> > &portableFactories =
                            context.getSerializationConfig().getPortableFactories();
                    std::map<int, std::shared_ptr<hazelcast::client::serialization::PortableFactory> >::const_iterator factoryIt =
                            portableFactories.find(factoryId);

                    if (portableFactories.end() == factoryIt) {
                        return std::unique_ptr<Portable>();
                    }

                    return factoryIt->second->create(classId);
                }

                int32_t PortableSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_PORTABLE;
                }

                void PortableSerializer::write(ObjectDataOutput &out, const void *object) {
                    const Portable *p = static_cast<const Portable *>(object);

                    if (p->getClassId() == 0) {
                        throw exception::IllegalArgumentException("Portable class ID cannot be zero!");
                    }

                    out.writeInt(p->getFactoryId());
                    out.writeInt(p->getClassId());

                    writeInternal(out, p);
                }

                void PortableSerializer::writeInternal(ObjectDataOutput &out, const Portable *p) const {
                    std::shared_ptr<ClassDefinition> cd = context.lookupOrRegisterClassDefinition(*p);
                    out.writeInt(cd->getVersion());

                    DefaultPortableWriter dpw(context, cd, out);
                    PortableWriter portableWriter(&dpw);
                    p->writePortable(portableWriter);
                    portableWriter.end();
                }

                void *PortableSerializer::read(ObjectDataInput &in) {
                    // should not be called
                    assert(0);
                    return NULL;
                }

                int32_t PortableSerializer::readInt(ObjectDataInput &in) const {
                    return in.readInt();
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
