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
        namespace aws {
            namespace impl {
                const char *Constants::DATE_FORMAT = "%Y%m%dT%H%M%SZ";
                const char *Constants::DOC_VERSION = "2016-11-15";
                const char *Constants::SIGNATURE_METHOD_V4 = "AWS4-HMAC-SHA256";
                const char *Constants::GET = "GET";
                const char *Constants::ECS_CREDENTIALS_ENV_VAR_NAME = "AWS_CONTAINER_CREDENTIALS_RELATIVE_URI";
            }
        }
    }
}


#ifdef HZ_BUILD_WITH_SSL
#endif


namespace hazelcast {
    namespace client {
        namespace aws {
            namespace security {
                std::string EC2RequestSigner::NEW_LINE = "\n";
                size_t EC2RequestSigner::DATE_LENGTH = 8;

                EC2RequestSigner::EC2RequestSigner(const config::ClientAwsConfig &awsConfig,
                                                   const std::string &timestamp,
                                                   const std::string &endpoint) : awsConfig(awsConfig),
                                                                                  timestamp(timestamp),
                                                                                  endpoint(endpoint) {
                }

                EC2RequestSigner::~EC2RequestSigner() {
                }

                std::string EC2RequestSigner::sign(const std::map<std::string, std::string> &attributes) {
                    std::string canonicalRequest = getCanonicalizedRequest(attributes);
                    std::string stringToSign = createStringToSign(canonicalRequest);
                    std::vector<unsigned char> signingKey = deriveSigningKey();

                    return createSignature(stringToSign, signingKey);
                }

                std::string EC2RequestSigner::createFormattedCredential() const {
                    std::stringstream out;
                    out << awsConfig.getAccessKey() << '/' << timestamp.substr(0, DATE_LENGTH) << '/'
                        << awsConfig.getRegion() << '/' << "ec2/aws4_request";
                    return out.str();
                }

                std::string EC2RequestSigner::getCanonicalizedQueryString(
                        const std::map<std::string, std::string> &attributes) const {
                    std::vector<std::string> components = getListOfEntries(attributes);
                    std::sort(components.begin(), components.end());
                    return getCanonicalizedQueryString(components);
                }

                /* Task 1 */
                std::string EC2RequestSigner::getCanonicalizedRequest(
                        const std::map<std::string, std::string> &attributes) const {
                    std::ostringstream out;
                    out << impl::Constants::GET << NEW_LINE
                        << '/' << NEW_LINE
                        << getCanonicalizedQueryString(attributes) << NEW_LINE
                        << getCanonicalHeaders() << NEW_LINE
                        << "host" << NEW_LINE
                        << sha256Hashhex("");
                    return out.str();
                }

                std::string EC2RequestSigner::getCanonicalHeaders() const {
                    std::ostringstream out;
                    out << "host:" << endpoint << NEW_LINE;
                    return out.str();
                }

                std::string EC2RequestSigner::getCanonicalizedQueryString(const std::vector<std::string> &list) const {
                    std::ostringstream result;
                    std::vector<std::string>::const_iterator it = list.begin();
                    result << (*it);
                    ++it;
                    for (; it != list.end(); ++it) {
                        result << "&" << *it;
                    }
                    return result.str();
                }

                std::vector<std::string> EC2RequestSigner::getListOfEntries(
                        const std::map<std::string, std::string> &entries) const {
                    std::vector<std::string> components;
                    for (std::map<std::string, std::string>::const_iterator it = entries.begin();
                         it != entries.end(); ++it) {
                        addComponents(components, entries, it->first);
                    }
                    return components;
                }

                void EC2RequestSigner::addComponents(std::vector<std::string> &components,
                                                     const std::map<std::string, std::string> &attributes,
                                                     const std::string &key) const {
                    std::ostringstream out;
                    out << utility::AwsURLEncoder::urlEncode(key) << '=' << utility::AwsURLEncoder::urlEncode(
                            (const_cast<std::map<std::string, std::string> &>(attributes))[key]);
                    components.push_back(out.str());
                }

                /* Task 2 */
                std::string EC2RequestSigner::createStringToSign(const std::string &canonicalRequest) const {
                    std::ostringstream out;
                    out << impl::Constants::SIGNATURE_METHOD_V4 << NEW_LINE
                        << timestamp << NEW_LINE
                        << getCredentialScope() << NEW_LINE
                        << sha256Hashhex(canonicalRequest);
                    return out.str();
                }

                std::string EC2RequestSigner::getCredentialScope() const {
                    // datestamp/region/service/API_TERMINATOR
                    // dateStamp
                    std::ostringstream out;
                    out << timestamp.substr(0, DATE_LENGTH) << "/" << awsConfig.getRegion() << "/ec2/aws4_request";
                    return out.str();
                }

                /* Task 3 */
                std::vector<unsigned char> EC2RequestSigner::deriveSigningKey() const {
                    const std::string &signKey = awsConfig.getSecretKey();
                    std::string dateStamp = timestamp.substr(0, DATE_LENGTH);
                    // this is derived from
                    // http://docs.aws.amazon.com/general/latest/gr/signature-v4-examples.html#signature-v4-examples-python

                    unsigned char kDate[32];
                    std::string key = std::string("AWS4") + signKey;
                    int kDateLen = hmacSHA256Bytes(key, dateStamp, kDate);

                    unsigned char kRegion[32];
                    int kRegionLen = hmacSHA256Bytes(kDate, kDateLen, awsConfig.getRegion(), kRegion);

                    unsigned char kService[32];
                    int kServiceLen = hmacSHA256Bytes(kRegion, kRegionLen, "ec2", kService);

                    std::vector<unsigned char> mSigning(32);
                    hmacSHA256Bytes(kService, kServiceLen, "aws4_request", &mSigning[0]);

                    return mSigning;
                }

                std::string EC2RequestSigner::createSignature(const std::string &stringToSign,
                                                              const std::vector<unsigned char> &signingKey) const {
                    return hmacSHA256Hex(signingKey, stringToSign);
                }

                std::string EC2RequestSigner::hmacSHA256Hex(const std::vector<unsigned char> &key,
                                                            const std::string &msg) const {
                    unsigned char hash[32];

                    unsigned int len = hmacSHA256Bytes(key, msg, hash);

                    return convertToHexString(hash, len);
                }

                std::string EC2RequestSigner::convertToHexString(const unsigned char *buffer, unsigned int len) const {
                    std::stringstream ss;
                    ss << std::hex << std::setfill('0');
                    for (unsigned int i = 0; i < len; i++) {
                        ss << std::hex << std::setw(2) << (unsigned int) buffer[i];
                    }

                    return (ss.str());
                }

                unsigned int EC2RequestSigner::hmacSHA256Bytes(const void *key, int keyLen, const std::string &msg,
                                                               unsigned char *hash) const {
                    return hmacSHA256Bytes(key, keyLen, (unsigned char *) &msg[0], msg.length(),
                                           hash);
                }

                unsigned int EC2RequestSigner::hmacSHA256Bytes(const std::string &key, const std::string &msg,
                                                               unsigned char *hash) const {
                    return hmacSHA256Bytes(&key[0], (int) key.length(), (unsigned char *) &msg[0], msg.length(),
                                           hash);
                }

                unsigned int EC2RequestSigner::hmacSHA256Bytes(const std::vector<unsigned char> &key,
                                                               const std::string &msg,
                                                               unsigned char *hash) const {
                    return hmacSHA256Bytes(&key[0], (int) key.size(), (unsigned char *) &msg[0], msg.length(),
                                           hash);
                }

                unsigned int EC2RequestSigner::hmacSHA256Bytes(const void *keyBuffer, int keyLen,
                                                               const unsigned char *data,
                                                               size_t dataLen,
                                                               unsigned char *hash) const {
#ifdef HZ_BUILD_WITH_SSL

#if OPENSSL_VERSION_NUMBER > 0x10100000L
                    HMAC_CTX *hmac = HMAC_CTX_new();
#else
                    HMAC_CTX *hmac = new HMAC_CTX;
                    HMAC_CTX_init(hmac);
#endif

                    HMAC_Init_ex(hmac, keyBuffer, keyLen, EVP_sha256(), NULL);
                    HMAC_Update(hmac, data, dataLen);
                    unsigned int len = 32;
                    HMAC_Final(hmac, hash, &len);

#if OPENSSL_VERSION_NUMBER > 0x10100000L
                    HMAC_CTX_free(hmac);
#else
                    HMAC_CTX_cleanup(hmac);
                    delete hmac;
#endif

                    return len;
#else
                    util::Preconditions::checkSSL("EC2RequestSigner::hmacSHA256Bytes");
                    return 0;
#endif
                }

                std::string EC2RequestSigner::sha256Hashhex(const std::string &in) const {
#ifdef HZ_BUILD_WITH_SSL
#ifdef OPENSSL_FIPS
                    unsigned int hashLen = 0;
                        unsigned char hash[EVP_MAX_MD_SIZE];
                        EVP_MD_CTX ctx;
                        EVP_MD_CTX_init(&ctx);
                        EVP_DigestInit_ex(&ctx, EVP_sha256(), NULL);
                        EVP_DigestUpdate(&ctx, in.c_str(), in.size());
                        EVP_DigestFinal_ex(&ctx, hash, &hashLen);
                        EVP_MD_CTX_cleanup(&ctx);
                        return convertToHexString(hash, hashLen);
#else
                    unsigned char hash[SHA256_DIGEST_LENGTH];
                    SHA256_CTX sha256;
                    SHA256_Init(&sha256);
                    SHA256_Update(&sha256, in.c_str(), in.size());
                    SHA256_Final(hash, &sha256);

                    return convertToHexString(hash, SHA256_DIGEST_LENGTH);
#endif // OPENSSL_FIPS
#else
                    util::Preconditions::checkSSL("EC2RequestSigner::hmacSHA256Bytes");
                    return "";
#endif // HZ_BUILD_WITH_SSL
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace aws {
            AWSClient::AWSClient(config::ClientAwsConfig &awsConfig, util::ILogger &logger) : awsConfig(awsConfig),
                                                                                              logger(logger) {
                this->endpoint = awsConfig.getHostHeader();
                if (!awsConfig.getRegion().empty() && awsConfig.getRegion().length() > 0) {
                    if (awsConfig.getHostHeader().find("ec2.") != 0) {
                        throw exception::InvalidConfigurationException("AWSClient::AWSClient",
                                                                       "HostHeader should start with \"ec2.\" prefix");
                    }
                    boost::replace_all(this->endpoint, "ec2.", std::string("ec2.") + awsConfig.getRegion() + ".");
                }
            }

            std::map<std::string, std::string> AWSClient::getAddresses() {
                return impl::DescribeInstances(awsConfig, endpoint, logger).execute();
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace aws {
            namespace utility {
                std::string AwsURLEncoder::urlEncode(const std::string &value) {
                    std::string result = escapeEncode(value);
                    boost::replace_all(result, "+", "%20");
                    return result;
                }

                std::string AwsURLEncoder::escapeEncode(const std::string &value) {
                    std::ostringstream escaped;
                    escaped.fill('0');
                    escaped << std::hex;

                    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
                        std::string::value_type c = (*i);

                        // Keep alphanumeric and other accepted characters intact
                        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                            escaped << c;
                            continue;
                        }

                        // Any other characters are percent-encoded
                        escaped << std::uppercase;
                        escaped << '%' << std::setw(2) << int((unsigned char) c);
                        escaped << std::nouppercase;
                    }

                    return escaped.str();
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace aws {
            namespace utility {
                std::map<std::string, std::string> CloudUtility::unmarshalTheResponse(std::istream &stream,
                                                                                      util::ILogger &logger) {
                    std::map<std::string, std::string> privatePublicPairs;

                    pt::ptree tree;
                    try {
                        pt::read_xml(stream, tree);
                    } catch (pt::xml_parser_error &e) {
                        logger.warning(
                                std::string("The parsed xml stream has errors: ") + e.what());
                        return privatePublicPairs;
                    }

                    // Use get_child to find the node containing the reservation set, and iterate over
                    // its children.
                    for (pt::ptree::value_type &item : tree.get_child("DescribeInstancesResponse.reservationSet")) {
                        for (pt::ptree::value_type &instanceItem : item.second.get_child("instancesSet")) {
                            auto privateIp = instanceItem.second.get_optional<std::string>("privateIpAddress");
                            auto publicIp = instanceItem.second.get_optional<std::string>("ipAddress");

                            auto prIp = privateIp.value_or("");
                            auto pubIp = publicIp.value_or("");

                            if (privateIp) {
                                privatePublicPairs[prIp] = pubIp;
                                if (logger.isFinestEnabled()) {
                                    auto instName = instanceItem.second.get_optional<std::string>(
                                            "tagset.item.value").value_or("");

                                    logger.finest(
                                            std::string("Accepting EC2 instance [") + instName +
                                            "][" + prIp + "]");
                                }
                            }
                        }
                    }
                    return privatePublicPairs;
                }

                void CloudUtility::unmarshalJsonResponse(std::istream &stream, config::ClientAwsConfig &awsConfig,
                                                         std::map<std::string, std::string> &attributes) {
                    pt::ptree json;
                    pt::read_json(stream, json);
                    awsConfig.setAccessKey(json.get_optional<std::string>("AccessKeyId").get_value_or(""));
                    awsConfig.setSecretKey(json.get_optional<std::string>("SecretAccessKey").get_value_or(""));
                    attributes["X-Amz-Security-Token"] = json.get_optional<std::string>("Token").get_value_or("");
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        const std::vector<Member> &InitialMembershipEvent::getMembers() const {
            return members;
        }

        Cluster &InitialMembershipEvent::getCluster() {
            return cluster;
        }

        InitialMembershipEvent::InitialMembershipEvent(Cluster &cluster, const std::set<Member> &members) : cluster(
                cluster) {
            for (const Member &member : members) {
                this->members.push_back(Member(member));
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            InstanceOfPredicate::InstanceOfPredicate(const char *javaClassName) : className(javaClassName) {
            }

            int InstanceOfPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int InstanceOfPredicate::getClassId() const {
                return impl::predicates::INSTANCEOF_PREDICATE;
            }

            void InstanceOfPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&className);
            }

            void InstanceOfPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("InstanceOfPredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            ILikePredicate::ILikePredicate(const std::string &attribute, const std::string &expression) : attributeName(
                    attribute), expressionString(expression) {
            }

            int ILikePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int ILikePredicate::getClassId() const {
                return impl::predicates::ILIKE_PREDICATE;
            }

            void ILikePredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&attributeName);
                out.writeUTF(&expressionString);
            }

            void ILikePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("ILikePredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            int TruePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int TruePredicate::getClassId() const {
                return impl::predicates::TRUE_PREDICATE;
            }

            void TruePredicate::writeData(serialization::ObjectDataOutput &out) const {
            }

            void TruePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("TruePredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            OrPredicate::~OrPredicate() {
                for (std::vector<Predicate *>::const_iterator it = predicates.begin();
                     it != predicates.end(); ++it) {
                    delete *it;
                }
            }

            OrPredicate &OrPredicate::add(std::unique_ptr<Predicate> &predicate) {
                return add(std::move(predicate));
            }

            OrPredicate &OrPredicate::add(std::unique_ptr<Predicate> &&predicate) {
                predicates.push_back(predicate.release());
                return *this;
            }

            int OrPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int OrPredicate::getClassId() const {
                return impl::predicates::OR_PREDICATE;
            }

            void OrPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeInt((int) predicates.size());
                for (std::vector<Predicate *>::const_iterator it = predicates.begin();
                     it != predicates.end(); ++it) {
                    out.writeObject<serialization::IdentifiedDataSerializable>(*it);
                }
            }

            void OrPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("OrPredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            NotPredicate::NotPredicate(std::unique_ptr<Predicate> &predicate)
                    : NotPredicate::NotPredicate(std::move(predicate)) {
            }

            NotPredicate::NotPredicate(std::unique_ptr<Predicate> &&predicate)
                    : internalPredicate(std::move(predicate)) {
            }

            int NotPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int NotPredicate::getClassId() const {
                return impl::predicates::NOT_PREDICATE;
            }

            void NotPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeObject<serialization::IdentifiedDataSerializable>(internalPredicate.get());
            }

            void NotPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("NotPredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            const char *QueryConstants::getKeyAttributeName() {
                return "__key";
            }

            const char *QueryConstants::getValueAttributeName() {
                return "this";
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            SqlPredicate::SqlPredicate(const std::string &sqlString) : sql(sqlString) {
            }

            int SqlPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int SqlPredicate::getClassId() const {
                return impl::predicates::SQL_PREDICATE;
            }

            void SqlPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&sql);
            }

            void SqlPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("SqlPredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            int FalsePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int FalsePredicate::getClassId() const {
                return impl::predicates::FALSE_PREDICATE;
            }

            void FalsePredicate::writeData(serialization::ObjectDataOutput &out) const {
            }

            void FalsePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("FalsePredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            RegexPredicate::RegexPredicate(const char *attribute, const char *regex) : attributeName(attribute),
                                                                                       regularExpression(regex) {
            }

            int RegexPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int RegexPredicate::getClassId() const {
                return impl::predicates::REGEX_PREDICATE;
            }

            void RegexPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&attributeName);
                out.writeUTF(&regularExpression);
            }

            void RegexPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("RegexPredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            AndPredicate::~AndPredicate() {
                for (std::vector<Predicate *>::const_iterator it = predicates.begin();
                     it != predicates.end(); ++it) {
                    delete *it;
                }
            }

            AndPredicate &AndPredicate::add(std::unique_ptr<Predicate> &predicate) {
                return add(std::move(predicate));
            }

            AndPredicate &AndPredicate::add(std::unique_ptr<Predicate> &&predicate) {
                predicates.push_back(predicate.release());
                return *this;
            }

            int AndPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int AndPredicate::getClassId() const {
                return impl::predicates::AND_PREDICATE;
            }

            void AndPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeInt((int) predicates.size());
                for (std::vector<Predicate *>::const_iterator it = predicates.begin();
                     it != predicates.end(); ++it) {
                    out.writeObject<serialization::IdentifiedDataSerializable>(*it);
                }
            }

            void AndPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("AndPredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace query {
            LikePredicate::LikePredicate(const std::string &attribute, const std::string &expression) : attributeName(
                    attribute), expressionString(expression) {
            }

            int LikePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int LikePredicate::getClassId() const {
                return impl::predicates::LIKE_PREDICATE;
            }

            void LikePredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&attributeName);
                out.writeUTF(&expressionString);
            }

            void LikePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("LikePredicate::readData",
                                                                 "Client should not need to use readData method!!!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {

        const std::string ClientProperties::PROP_HEARTBEAT_TIMEOUT = "hazelcast_client_heartbeat_timeout";
        const std::string ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT = "60";
        const std::string ClientProperties::PROP_HEARTBEAT_INTERVAL = "hazelcast_client_heartbeat_interval";
        const std::string ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT = "10";
        const std::string ClientProperties::PROP_REQUEST_RETRY_COUNT = "hazelcast_client_request_retry_count";
        const std::string ClientProperties::PROP_REQUEST_RETRY_COUNT_DEFAULT = "20";
        const std::string ClientProperties::PROP_REQUEST_RETRY_WAIT_TIME = "hazelcast_client_request_retry_wait_time";
        const std::string ClientProperties::PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT = "1";

        const std::string ClientProperties::PROP_AWS_MEMBER_PORT = "hz-port";
        const std::string ClientProperties::PROP_AWS_MEMBER_PORT_DEFAULT = "5701";

        const std::string ClientProperties::CLEAN_RESOURCES_PERIOD_MILLIS = "hazelcast.client.internal.clean.resources.millis";
        const std::string ClientProperties::CLEAN_RESOURCES_PERIOD_MILLIS_DEFAULT = "100";

        const std::string ClientProperties::INVOCATION_RETRY_PAUSE_MILLIS = "hazelcast.client.invocation.retry.pause.millis";
        const std::string ClientProperties::INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT = "1000";

        const std::string ClientProperties::INVOCATION_TIMEOUT_SECONDS = "hazelcast.client.invocation.timeout.seconds";
        const std::string ClientProperties::INVOCATION_TIMEOUT_SECONDS_DEFAULT = "120";

        const std::string ClientProperties::EVENT_THREAD_COUNT = "hazelcast.client.event.thread.count";
        const std::string ClientProperties::EVENT_THREAD_COUNT_DEFAULT = "5";

        const std::string ClientProperties::EVENT_QUEUE_CAPACITY = "hazelcast.client.event.queue.capacity";
        const std::string ClientProperties::EVENT_QUEUE_CAPACITY_DEFAULT = "1000000";

        const std::string ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE = "hazelcast.client.internal.executor.pool.size";
        const std::string ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT = "3";

        const std::string ClientProperties::SHUFFLE_MEMBER_LIST = "hazelcast.client.shuffle.member.list";
        const std::string ClientProperties::SHUFFLE_MEMBER_LIST_DEFAULT = "true";

        const std::string ClientProperties::MAX_CONCURRENT_INVOCATIONS = "hazelcast.client.max.concurrent.invocations";
        const std::string ClientProperties::MAX_CONCURRENT_INVOCATIONS_DEFAULT = util::IOUtil::to_string<int32_t>(
                INT32_MAX);

        const std::string ClientProperties::BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS = "hazelcast.client.invocation.backoff.timeout.millis";
        const std::string ClientProperties::BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT = "-1";

        const std::string ClientProperties::STATISTICS_ENABLED = "hazelcast.client.statistics.enabled";
        const std::string ClientProperties::STATISTICS_ENABLED_DEFAULT = "false";

        const std::string ClientProperties::STATISTICS_PERIOD_SECONDS = "hazelcast.client.statistics.period.seconds";
        const std::string ClientProperties::STATISTICS_PERIOD_SECONDS_DEFAULT = "3";

        const std::string ClientProperties::IO_THREAD_COUNT = "hazelcast.client.io.thread.count";
        const std::string ClientProperties::IO_THREAD_COUNT_DEFAULT = "1";

        ClientProperty::ClientProperty(const std::string &name, const std::string &defaultValue)
                : name(name), defaultValue(defaultValue) {
        }

        const std::string &ClientProperty::getName() const {
            return name;
        }

        const std::string &ClientProperty::getDefaultValue() const {
            return defaultValue;
        }

        const char *ClientProperty::getSystemProperty() const {
            return ::getenv(name.c_str());
        }

        ClientProperties::ClientProperties(const std::map<std::string, std::string> &properties)
                : heartbeatTimeout(PROP_HEARTBEAT_TIMEOUT, PROP_HEARTBEAT_TIMEOUT_DEFAULT),
                  heartbeatInterval(PROP_HEARTBEAT_INTERVAL, PROP_HEARTBEAT_INTERVAL_DEFAULT),
                  retryCount(PROP_REQUEST_RETRY_COUNT, PROP_REQUEST_RETRY_COUNT_DEFAULT),
                  retryWaitTime(PROP_REQUEST_RETRY_WAIT_TIME, PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT),
                  awsMemberPort(PROP_AWS_MEMBER_PORT, PROP_AWS_MEMBER_PORT_DEFAULT),
                  cleanResourcesPeriod(CLEAN_RESOURCES_PERIOD_MILLIS,
                                       CLEAN_RESOURCES_PERIOD_MILLIS_DEFAULT),
                  invocationRetryPauseMillis(INVOCATION_RETRY_PAUSE_MILLIS,
                                             INVOCATION_RETRY_PAUSE_MILLIS_DEFAULT),
                  invocationTimeoutSeconds(INVOCATION_TIMEOUT_SECONDS,
                                           INVOCATION_TIMEOUT_SECONDS_DEFAULT),
                  eventThreadCount(EVENT_THREAD_COUNT, EVENT_THREAD_COUNT_DEFAULT),
                  eventQueueCapacity(EVENT_QUEUE_CAPACITY, EVENT_QUEUE_CAPACITY_DEFAULT),
                  internalExecutorPoolSize(INTERNAL_EXECUTOR_POOL_SIZE,
                                           INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT),
                  shuffleMemberList(SHUFFLE_MEMBER_LIST, SHUFFLE_MEMBER_LIST_DEFAULT),
                  maxConcurrentInvocations(MAX_CONCURRENT_INVOCATIONS,
                                           MAX_CONCURRENT_INVOCATIONS_DEFAULT),
                  backpressureBackoffTimeoutMillis(BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS,
                                                   BACKPRESSURE_BACKOFF_TIMEOUT_MILLIS_DEFAULT),
                  statisticsEnabled(STATISTICS_ENABLED, STATISTICS_ENABLED_DEFAULT),
                  statisticsPeriodSeconds(STATISTICS_PERIOD_SECONDS, STATISTICS_PERIOD_SECONDS_DEFAULT),
                  ioThreadCount(IO_THREAD_COUNT, IO_THREAD_COUNT_DEFAULT),
                  propertiesMap(properties) {
        }

        const ClientProperty &ClientProperties::getHeartbeatTimeout() const {
            return heartbeatTimeout;
        }

        const ClientProperty &ClientProperties::getHeartbeatInterval() const {
            return heartbeatInterval;
        }

        const ClientProperty &ClientProperties::getAwsMemberPort() const {
            return awsMemberPort;
        }

        const ClientProperty &ClientProperties::getCleanResourcesPeriodMillis() const {
            return cleanResourcesPeriod;
        }

        const ClientProperty &ClientProperties::getInvocationRetryPauseMillis() const {
            return invocationRetryPauseMillis;
        }

        const ClientProperty &ClientProperties::getInvocationTimeoutSeconds() const {
            return invocationTimeoutSeconds;
        }

        const ClientProperty &ClientProperties::getEventThreadCount() const {
            return eventThreadCount;
        }

        const ClientProperty &ClientProperties::getEventQueueCapacity() const {
            return eventQueueCapacity;
        }

        const ClientProperty &ClientProperties::getInternalExecutorPoolSize() const {
            return internalExecutorPoolSize;
        }

        const ClientProperty &ClientProperties::getShuffleMemberList() const {
            return shuffleMemberList;
        }

        const ClientProperty &ClientProperties::getMaxConcurrentInvocations() const {
            return maxConcurrentInvocations;
        }

        const ClientProperty &ClientProperties::getBackpressureBackoffTimeoutMillis() const {
            return backpressureBackoffTimeoutMillis;
        }

        const ClientProperty &ClientProperties::getStatisticsEnabled() const {
            return statisticsEnabled;
        }

        const ClientProperty &ClientProperties::getStatisticsPeriodSeconds() const {
            return statisticsPeriodSeconds;
        }

        const ClientProperty &ClientProperties::getIOThreadCount() const {
            return ioThreadCount;
        }

        std::string ClientProperties::getString(const ClientProperty &property) const {
            std::map<std::string, std::string>::const_iterator valueIt = propertiesMap.find(property.getName());
            if (valueIt != propertiesMap.end()) {
                return valueIt->second;
            }

            const char *value = property.getSystemProperty();
            if (value != NULL) {
                return value;
            }

            return property.getDefaultValue();
        }

        bool ClientProperties::getBoolean(const ClientProperty &property) const {
            return util::IOUtil::to_value<bool>(getString(property));
        }

        int32_t ClientProperties::getInteger(const ClientProperty &property) const {
            return util::IOUtil::to_value<int32_t>(getString(property));
        }

        int64_t ClientProperties::getLong(const ClientProperty &property) const {
            return util::IOUtil::to_value<int64_t>(getString(property));
        }
    }
}




namespace hazelcast {
    namespace client {

        GroupConfig::GroupConfig() : name("dev"), password("dev-pass") {

        }

        GroupConfig::GroupConfig(const std::string &name, const std::string &password)
                : name(name), password(password) {
        }

        std::string GroupConfig::getName() const {
            return name;
        }

        GroupConfig &GroupConfig::setName(const std::string &name) {
            this->name = name;
            return (*this);
        }

        GroupConfig &GroupConfig::setPassword(const std::string &password) {
            this->password = password;
            return (*this);
        }

        std::string GroupConfig::getPassword() const {
            return password;
        }

    }
}


namespace hazelcast {
    namespace client {
        LifecycleEvent::LifecycleEvent(LifeCycleState state)
                : state(state) {

        }

        LifecycleEvent::LifeCycleState LifecycleEvent::getState() const {
            return state;
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace exception {
            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, const std::string &details,
                                   int32_t errorNo, int32_t causeCode, bool isRuntime, bool retryable) : src(source),
                                                                                                         msg(message),
                                                                                                         details(details),
                                                                                                         errorCode(
                                                                                                                 errorNo),
                                                                                                         causeErrorCode(
                                                                                                                 causeCode),
                                                                                                         runtimeException(
                                                                                                                 isRuntime),
                                                                                                         retryable(
                                                                                                                 retryable) {
                std::ostringstream out;
                out << exceptionName << " {" << message << ". Details:" << details << " Error code:" << errorNo
                    << ", Cause error code:" << causeCode << "} at " + source;
                report = out.str();
            }

            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, int32_t errorNo,
                                   int32_t causeCode, bool isRuntime, bool retryable) : src(source), msg(message),
                                                                                        errorCode(errorNo),
                                                                                        causeErrorCode(causeCode),
                                                                                        runtimeException(isRuntime),
                                                                                        retryable(retryable) {
                std::ostringstream out;
                out << exceptionName << " {" << message << " Error code:" << errorNo << ", Cause error code:"
                    << causeCode << "} at " + source;
                report = out.str();
            }

            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, int32_t errorNo, bool isRuntime, bool retryable) : src(
                    source), msg(message), errorCode(errorNo), runtimeException(isRuntime), retryable(retryable) {
                std::ostringstream out;
                out << exceptionName << " {" << message << " Error code:" << errorNo << "} at " + source;
                report = out.str();
            }

            IException::IException(const std::string &exceptionName, const std::string &source,
                                   const std::string &message, int32_t errorNo,
                                   const std::shared_ptr<IException> &cause, bool isRuntime, bool retryable) : src(
                    source), msg(message),
                                                                                                               errorCode(
                                                                                                                       errorNo),
                                                                                                               cause(cause),
                                                                                                               runtimeException(
                                                                                                                       isRuntime),
                                                                                                               retryable(
                                                                                                                       retryable) {
                std::ostringstream out;
                out << exceptionName << " {" << message << " Error code:" << errorNo << ", Caused by:" << *cause
                    << "} at " + source;
                report = out.str();

            }

            IException::~IException() throw() {
            }

            char const *IException::what() const throw() {
                return report.c_str();
            }

            const std::string &IException::getSource() const {
                return src;
            }

            const std::string &IException::getMessage() const {
                return msg;
            }

            void IException::raise() const {
                throw *this;
            }

            std::ostream &operator<<(std::ostream &os, const IException &exception) {
                os << exception.what();
                return os;
            }

            const std::shared_ptr<IException> &IException::getCause() const {
                return cause;
            }

            std::unique_ptr<IException> IException::clone() const {
                return std::unique_ptr<IException>(new IException(*this));
            }

            const std::string &IException::getDetails() const {
                return details;
            }

            int32_t IException::getErrorCode() const {
                return errorCode;
            }

            int32_t IException::getCauseErrorCode() const {
                return causeErrorCode;
            }

            bool IException::isRuntimeException() const {
                return runtimeException;
            }

            bool IException::isRetryable() const {
                return retryable;
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
