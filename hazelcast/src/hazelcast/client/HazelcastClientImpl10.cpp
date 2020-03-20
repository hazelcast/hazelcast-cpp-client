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
            namespace pimpl {

                DataInput::DataInput(const std::vector<byte> &buf) : buffer(buf), pos(0) {
                }

                DataInput::DataInput(const std::vector<byte> &buf, int offset)
                        : buffer(buf), pos(offset) {
                }

                DataInput::~DataInput() {
                }

                void DataInput::readFully(std::vector<byte> &bytes) {
                    size_t length = bytes.size();
                    checkAvailable(length);
                    memcpy(&(bytes[0]), &(buffer[pos]), length);
                    pos += length;
                }

                int DataInput::skipBytes(int i) {
                    checkAvailable(i);
                    pos += i;
                    return i;
                }

                bool DataInput::readBoolean() {
                    checkAvailable(1);
                    return readBooleanUnchecked();
                }

                bool DataInput::readBooleanUnchecked() { return readByteUnchecked() != 0; }

                byte DataInput::readByte() {
                    checkAvailable(1);
                    return readByteUnchecked();
                }

                byte DataInput::readByteUnchecked() { return buffer[pos++]; }

                int16_t DataInput::readShort() {
                    checkAvailable(util::Bits::SHORT_SIZE_IN_BYTES);
                    return readShortUnchecked();
                }

                int16_t DataInput::readShortUnchecked() {
                    int16_t result;
                    util::Bits::bigEndianToNative2(&buffer[pos], &result);
                    pos += util::Bits::SHORT_SIZE_IN_BYTES;
                    return result;
                }

                char DataInput::readChar() {
                    checkAvailable(util::Bits::CHAR_SIZE_IN_BYTES);
                    return readCharUnchecked();
                }

                char DataInput::readCharUnchecked() {
                    // skip the first byte
                    byte b = buffer[pos + 1];
                    pos += util::Bits::CHAR_SIZE_IN_BYTES;
                    return b;
                }

                int32_t DataInput::readInt() {
                    checkAvailable(util::Bits::INT_SIZE_IN_BYTES);
                    return readIntUnchecked();
                }

                int32_t DataInput::readIntUnchecked() {
                    int32_t result;
                    util::Bits::bigEndianToNative4(&buffer[pos], &result);
                    pos += util::Bits::INT_SIZE_IN_BYTES;
                    return result;
                }

                int64_t DataInput::readLong() {
                    checkAvailable(util::Bits::LONG_SIZE_IN_BYTES);
                    return readLongUnchecked();
                }

                int64_t DataInput::readLongUnchecked() {
                    int64_t result;
                    util::Bits::bigEndianToNative8(&buffer[pos], &result);
                    pos += util::Bits::LONG_SIZE_IN_BYTES;
                    return result;
                }

                float DataInput::readFloat() {
                    checkAvailable(util::Bits::FLOAT_SIZE_IN_BYTES);
                    return readFloatUnchecked();
                }

                float DataInput::readFloatUnchecked() {
                    union {
                        int32_t i;
                        float f;
                    } u;
                    u.i = readIntUnchecked();
                    return u.f;
                }

                double DataInput::readDouble() {
                    checkAvailable(util::Bits::DOUBLE_SIZE_IN_BYTES);
                    return readDoubleUnchecked();
                }

                double DataInput::readDoubleUnchecked() {
                    union {
                        double d;
                        int64_t l;
                    } u;
                    u.l = readLongUnchecked();
                    return u.d;
                }

                std::unique_ptr<std::string> DataInput::readUTF() {
                    int32_t charCount = readInt();
                    if (util::Bits::NULL_ARRAY == charCount) {
                        return std::unique_ptr<std::string>();
                    } else {
                        utfBuffer.clear();
                        utfBuffer.reserve((size_t) MAX_UTF_CHAR_SIZE * charCount);
                        byte b;
                        for (int i = 0; i < charCount; ++i) {
                            b = readByte();
                            util::UTFUtil::readUTF8Char(*this, b, utfBuffer);
                        }

                        return std::unique_ptr<std::string>(new std::string(utfBuffer.begin(), utfBuffer.end()));
                    }
                }

                int DataInput::position() {
                    return pos;
                }

                void DataInput::position(int position) {
                    if (position > pos) {
                        checkAvailable((size_t) (position - pos));
                    }
                    pos = position;
                }
                //private functions

                std::unique_ptr<std::vector<byte> > DataInput::readByteArray() {
                    return readArray<byte>();
                }

                std::unique_ptr<std::vector<bool> > DataInput::readBooleanArray() {
                    return readArray<bool>();
                }

                std::unique_ptr<std::vector<char> > DataInput::readCharArray() {
                    return readArray<char>();
                }

                std::unique_ptr<std::vector<int32_t> > DataInput::readIntArray() {
                    return readArray<int32_t>();
                }

                std::unique_ptr<std::vector<int64_t> > DataInput::readLongArray() {
                    return readArray<int64_t>();
                }

                std::unique_ptr<std::vector<double> > DataInput::readDoubleArray() {
                    return readArray<double>();
                }

                std::unique_ptr<std::vector<float> > DataInput::readFloatArray() {
                    return readArray<float>();
                }

                std::unique_ptr<std::vector<int16_t> > DataInput::readShortArray() {
                    return readArray<int16_t>();
                }

                std::unique_ptr<std::vector<std::string> > DataInput::readUTFArray() {
                    int32_t len = readInt();
                    if (util::Bits::NULL_ARRAY == len) {
                        return std::unique_ptr<std::vector<std::string> >();
                    }

                    std::unique_ptr<std::vector<std::string> > values(
                            new std::vector<std::string>());
                    for (int32_t i = 0; i < len; ++i) {
                        std::unique_ptr<std::string> value = readUTF();
                        // handle null pointer possibility
                        if ((std::string *) NULL == value.get()) {
                            values->push_back(std::string(""));
                        } else {
                            values->push_back(*value);
                        }
                    }
                    return values;
                }

                std::unique_ptr<std::vector<std::string *> > DataInput::readUTFPointerArray() {
                    int32_t len = readInt();
                    if (util::Bits::NULL_ARRAY == len) {
                        return std::unique_ptr<std::vector<std::string *> >();
                    }

                    std::unique_ptr<std::vector<std::string *> > values(
                            new std::vector<std::string *>());
                    try {
                        for (int32_t i = 0; i < len; ++i) {
                            values->push_back(readUTF().release());
                        }
                    } catch (exception::IException &) {
                        // clean resources to avoid any leaks
                        typedef std::vector<std::string *> STRING_ARRAY;
                        for (STRING_ARRAY::value_type value  : *values) {
                            delete value;
                        }
                        throw;
                    }
                    return values;
                }

                void DataInput::checkAvailable(size_t requestedLength) {
                    size_t available = buffer.size() - pos;

                    if (requestedLength > available) {
                        char msg[100];
                        util::hz_snprintf(msg, 100,
                                          "Not enough bytes in internal buffer. Available:%lu bytes but needed %lu bytes",
                                          (unsigned long) available, (unsigned long) requestedLength);
                        throw exception::IOException("DataInput::checkBoundary", msg);
                    }
                }

                template<>
                byte DataInput::read() {
                    return readByteUnchecked();
                }

                template<>
                char DataInput::read() {
                    return readCharUnchecked();
                }

                template<>
                bool DataInput::read() {
                    return readBooleanUnchecked();
                }

                template<>
                int16_t DataInput::read() {
                    return readShortUnchecked();
                }

                template<>
                int32_t DataInput::read() {
                    return readIntUnchecked();
                }

                template<>
                int64_t DataInput::read() {
                    return readLongUnchecked();
                }

                template<>
                float DataInput::read() {
                    return readFloatUnchecked();
                }

                template<>
                double DataInput::read() {
                    return readDoubleUnchecked();
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                PortableReaderBase::PortableReaderBase(PortableContext &portableContext, ObjectDataInput &input,
                                                       std::shared_ptr<ClassDefinition> cd)
                        : cd(cd), dataInput(input), serializerHolder(portableContext.getSerializerHolder()),
                          raw(false) {
                    int fieldCount;
                    try {
                        // final position after portable is read
                        finalPosition = input.readInt();
                        // field count
                        fieldCount = input.readInt();
                    } catch (exception::IException &e) {
                        throw exception::HazelcastSerializationException(
                                "[DefaultPortableReader::DefaultPortableReader]", e.what());
                    }
                    if (fieldCount != cd->getFieldCount()) {
                        char msg[50];
                        util::hz_snprintf(msg, 50, "Field count[%d] in stream does not match %d", fieldCount,
                                          cd->getFieldCount());
                        throw new exception::IllegalStateException("[DefaultPortableReader::DefaultPortableReader]",
                                                                   msg);
                    }
                    this->offset = input.position();
                }

                PortableReaderBase::~PortableReaderBase() {

                }

                int32_t PortableReaderBase::readInt(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_INT);
                    return dataInput.readInt();
                }

                int64_t PortableReaderBase::readLong(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG);
                    return dataInput.readLong();
                }

                bool PortableReaderBase::readBoolean(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN);
                    return dataInput.readBoolean();
                }

                hazelcast::byte PortableReaderBase::readByte(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE);
                    return dataInput.readByte();
                }

                char PortableReaderBase::readChar(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR);
                    return dataInput.readChar();
                }

                double PortableReaderBase::readDouble(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE);
                    return dataInput.readDouble();
                }

                float PortableReaderBase::readFloat(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT);
                    return dataInput.readFloat();
                }

                int16_t PortableReaderBase::readShort(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT);
                    return dataInput.readShort();
                }

                std::unique_ptr<std::string> PortableReaderBase::readUTF(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_UTF);
                    return dataInput.readUTF();
                }

                std::unique_ptr<std::vector<byte> > PortableReaderBase::readByteArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                    return dataInput.readByteArray();
                }

                std::unique_ptr<std::vector<bool> > PortableReaderBase::readBooleanArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN_ARRAY);
                    return dataInput.readBooleanArray();
                }

                std::unique_ptr<std::vector<char> > PortableReaderBase::readCharArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                    return dataInput.readCharArray();
                }

                std::unique_ptr<std::vector<int32_t> > PortableReaderBase::readIntArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_INT_ARRAY);
                    return dataInput.readIntArray();
                }

                std::unique_ptr<std::vector<int64_t> > PortableReaderBase::readLongArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG_ARRAY);
                    return dataInput.readLongArray();
                }

                std::unique_ptr<std::vector<double> > PortableReaderBase::readDoubleArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                    return dataInput.readDoubleArray();
                }

                std::unique_ptr<std::vector<float> > PortableReaderBase::readFloatArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                    return dataInput.readFloatArray();
                }

                std::unique_ptr<std::vector<int16_t> > PortableReaderBase::readShortArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                    return dataInput.readShortArray();
                }

                void PortableReaderBase::setPosition(char const *fieldName, FieldType const &fieldType) {
                    dataInput.position(readPosition(fieldName, fieldType));
                }

                int PortableReaderBase::readPosition(const char *fieldName, FieldType const &fieldType) {
                    if (raw) {
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                         "Cannot read Portable fields after getRawDataInput() is called!");
                    }
                    if (!cd->hasField(fieldName)) {
                        // TODO: if no field def found, java client reads nested position:
                        // readNestedPosition(fieldName, type);
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                         "Don't have a field named " +
                                                                         std::string(fieldName));
                    }

                    if (cd->getFieldType(fieldName) != fieldType) {
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                         "Field type did not matched for " +
                                                                         std::string(fieldName));
                    }

                    dataInput.position(offset + cd->getField(fieldName).getIndex() * util::Bits::INT_SIZE_IN_BYTES);
                    int32_t pos = dataInput.readInt();

                    dataInput.position(pos);
                    int16_t len = dataInput.readShort();

                    // name + len + type
                    return pos + util::Bits::SHORT_SIZE_IN_BYTES + len + 1;
                }

                hazelcast::client::serialization::ObjectDataInput &PortableReaderBase::getRawDataInput() {
                    if (!raw) {
                        dataInput.position(offset + cd->getFieldCount() * util::Bits::INT_SIZE_IN_BYTES);
                        int32_t pos = dataInput.readInt();
                        dataInput.position(pos);
                    }
                    raw = true;
                    return dataInput;
                }

                void PortableReaderBase::end() {
                    dataInput.position(finalPosition);
                }

                void
                PortableReaderBase::checkFactoryAndClass(FieldDefinition fd, int32_t factoryId, int32_t classId) const {
                    if (factoryId != fd.getFactoryId()) {
                        char msg[100];
                        util::hz_snprintf(msg, 100, "Invalid factoryId! Expected: %d, Current: %d", fd.getFactoryId(),
                                          factoryId);
                        throw exception::HazelcastSerializationException("DefaultPortableReader::checkFactoryAndClass ",
                                                                         std::string(msg));
                    }
                    if (classId != fd.getClassId()) {
                        char msg[100];
                        util::hz_snprintf(msg, 100, "Invalid classId! Expected: %d, Current: %d", fd.getClassId(),
                                          classId);
                        throw exception::HazelcastSerializationException("DefaultPortableReader::checkFactoryAndClass ",
                                                                         std::string(msg));
                    }
                }

            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                int32_t IntegerSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_INTEGER;
                }

                void IntegerSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeInt(*(static_cast<const int32_t *>(object)));
                }

                void *IntegerSerializer::read(ObjectDataInput &in) {
                    return new int32_t(in.readInt());
                }

                int32_t ByteSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BYTE;
                }

                void ByteSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeByte(*(static_cast<const byte *>(object)));
                }

                void *ByteSerializer::read(ObjectDataInput &in) {
                    return new byte(in.readByte());
                }

                int32_t BooleanSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
                }

                void BooleanSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeBoolean(*(static_cast<const bool *>(object)));
                }

                void *BooleanSerializer::read(ObjectDataInput &in) {
                    return new bool(in.readBoolean());
                }

                int32_t CharSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_CHAR;
                }

                void CharSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeChar(*(static_cast<const char *>(object)));
                }

                void *CharSerializer::read(ObjectDataInput &in) {
                    return new char(in.readChar());
                }

                int32_t ShortSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_SHORT;
                }

                void ShortSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeShort(*(static_cast<const int16_t *>(object)));
                }

                void *ShortSerializer::read(ObjectDataInput &in) {
                    return new int16_t(in.readShort());
                }

                int32_t LongSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_LONG;
                }

                void LongSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeLong(*(static_cast<const int64_t *>(object)));
                }

                void *LongSerializer::read(ObjectDataInput &in) {
                    return new int64_t(in.readLong());;
                }

                int32_t FloatSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_FLOAT;
                }

                void FloatSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeFloat(*(static_cast<const float *>(object)));
                }

                void *FloatSerializer::read(ObjectDataInput &in) {
                    return new float(in.readFloat());
                }


                int32_t DoubleSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DOUBLE;
                }

                void DoubleSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeDouble(*(static_cast<const double *>(object)));
                }

                void *DoubleSerializer::read(ObjectDataInput &in) {
                    return new double(in.readDouble());
                }

                int32_t StringSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_STRING;
                }

                void StringSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTF(static_cast<const std::string *>(object));
                }

                void *StringSerializer::read(ObjectDataInput &in) {
                    return in.readUTF().release();
                }

                int32_t NullSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_NULL;
                }

                void *NullSerializer::read(ObjectDataInput &in) {
                    return NULL;
                }

                void NullSerializer::write(ObjectDataOutput &out, const void *object) {
                }

                int32_t TheByteArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
                }

                void TheByteArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeByteArray(static_cast<const std::vector<byte> *>(object));
                }

                void *TheByteArraySerializer::read(ObjectDataInput &in) {
                    return in.readByteArray().release();
                }

                int32_t BooleanArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY;
                }

                void BooleanArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeBooleanArray(static_cast<const std::vector<bool> *>(object));
                }

                void *BooleanArraySerializer::read(ObjectDataInput &in) {
                    return in.readBooleanArray().release();
                }

                int32_t CharArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
                }

                void CharArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeCharArray(static_cast<const std::vector<char> *>(object));
                }

                void *CharArraySerializer::read(ObjectDataInput &in) {
                    return in.readCharArray().release();
                }

                int32_t ShortArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
                }

                void ShortArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeShortArray(static_cast<const std::vector<int16_t> *>(object));
                }

                void *ShortArraySerializer::read(ObjectDataInput &in) {
                    return in.readShortArray().release();
                }

                int32_t IntegerArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
                }

                void IntegerArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeIntArray(static_cast<const std::vector<int32_t> *>(object));
                }

                void *IntegerArraySerializer::read(ObjectDataInput &in) {
                    return in.readIntArray().release();
                }

                int32_t LongArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
                }

                void LongArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeLongArray(static_cast<const std::vector<int64_t> *>(object));
                }

                void *LongArraySerializer::read(ObjectDataInput &in) {
                    return in.readLongArray().release();
                }

                int32_t FloatArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
                }

                void FloatArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeFloatArray(static_cast<const std::vector<float> *>(object));
                }

                void *FloatArraySerializer::read(ObjectDataInput &in) {
                    return in.readFloatArray().release();
                }

                int32_t DoubleArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
                }

                void DoubleArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeDoubleArray(static_cast<const std::vector<double> *>(object));
                }

                void *DoubleArraySerializer::read(ObjectDataInput &in) {
                    return in.readDoubleArray().release();
                }

                int32_t StringArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
                }

                void StringArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTFArray(static_cast<const std::vector<std::string *> *>(object));
                }

                void *StringArraySerializer::read(ObjectDataInput &in) {
                    return in.readUTFPointerArray().release();
                }

                int32_t HazelcastJsonValueSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::JAVASCRIPT_JSON_SERIALIZATION_TYPE;
                }

                void HazelcastJsonValueSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTF(&(static_cast<const HazelcastJsonValue *>(object)->toString()));
                }

                void *HazelcastJsonValueSerializer::read(ObjectDataInput &in) {
                    return new HazelcastJsonValue(*in.readUTF());
                }
            }
        }
    }
}
//
//  MorphingPortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                MorphingPortableReader::MorphingPortableReader(PortableContext &portableContext, ObjectDataInput &input,
                                                               std::shared_ptr<ClassDefinition> cd)
                        : PortableReaderBase(portableContext, input, cd) {
                }

                int32_t MorphingPortableReader::readInt(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                int64_t MorphingPortableReader::readLong(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_LONG) {
                        return PortableReaderBase::readLong(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                bool MorphingPortableReader::readBoolean(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return false;
                    }
                    return PortableReaderBase::readBoolean(fieldName);
                }

                byte MorphingPortableReader::readByte(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    return PortableReaderBase::readByte(fieldName);
                }

                char MorphingPortableReader::readChar(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }

                    return PortableReaderBase::readChar(fieldName);
                }

                double MorphingPortableReader::readDouble(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0.0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_FLOAT) {
                        return PortableReaderBase::readFloat(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_DOUBLE) {
                        return PortableReaderBase::readDouble(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_LONG) {
                        return PortableReaderBase::readLong(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                float MorphingPortableReader::readFloat(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0.0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_FLOAT) {
                        return PortableReaderBase::readFloat(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_INT) {
                        return (float) PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return (float) PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return (float) PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return (float) PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                int16_t MorphingPortableReader::readShort(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                std::unique_ptr<std::string> MorphingPortableReader::readUTF(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::string>(new std::string(""));
                    }
                    return PortableReaderBase::readUTF(fieldName);
                }

                std::unique_ptr<std::vector<byte> > MorphingPortableReader::readByteArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<byte> >(new std::vector<byte>(1, 0));
                    }
                    return PortableReaderBase::readByteArray(fieldName);
                }

                std::unique_ptr<std::vector<char> > MorphingPortableReader::readCharArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<char> >(new std::vector<char>(1, 0));
                    }
                    return PortableReaderBase::readCharArray(fieldName);
                }

                std::unique_ptr<std::vector<int32_t> > MorphingPortableReader::readIntArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<int32_t> >(new std::vector<int32_t>(1, 0));
                    }
                    return PortableReaderBase::readIntArray(fieldName);
                }

                std::unique_ptr<std::vector<int64_t> > MorphingPortableReader::readLongArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<int64_t> >(new std::vector<int64_t>(1, 0));
                    }
                    return PortableReaderBase::readLongArray(fieldName);
                }

                std::unique_ptr<std::vector<double> > MorphingPortableReader::readDoubleArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<double> >(new std::vector<double>(1, 0));
                    }
                    return PortableReaderBase::readDoubleArray(fieldName);
                }

                std::unique_ptr<std::vector<float> > MorphingPortableReader::readFloatArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<float> >(new std::vector<float>(1, 0));
                    }
                    return PortableReaderBase::readFloatArray(fieldName);
                }

                std::unique_ptr<std::vector<int16_t> > MorphingPortableReader::readShortArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<int16_t> >(new std::vector<int16_t>(1, 0));
                    }
                    return PortableReaderBase::readShortArray(fieldName);
                }


            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                int PortableVersionHelper::getVersion(const Portable *portable, int defaultVersion) {
                    int version = defaultVersion;
                    if (const VersionedPortable *versionedPortable = dynamic_cast<const VersionedPortable *>(portable)) {
                        version = versionedPortable->getClassVersion();
                        if (version < 0) {
                            throw exception::IllegalArgumentException("PortableVersionHelper:getVersion",
                                                                      "Version cannot be negative!");
                        }
                    }
                    return version;
                }
            }
        }
    }
}


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
