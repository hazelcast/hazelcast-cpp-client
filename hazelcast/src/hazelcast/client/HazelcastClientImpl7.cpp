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

        MapEvent::MapEvent(const Member &member, EntryEventType eventType, const std::string &name,
                           int numberOfEntriesAffected)
                : member(member), eventType(eventType), name(name), numberOfEntriesAffected(numberOfEntriesAffected) {

        }

        Member MapEvent::getMember() const {
            return member;
        }

        EntryEventType MapEvent::getEventType() const {
            return eventType;
        }

        const std::string &MapEvent::getName() const {
            return name;
        }

        int MapEvent::getNumberOfEntriesAffected() const {
            return numberOfEntriesAffected;
        }

        std::ostream &operator<<(std::ostream &os, const MapEvent &event) {
            os << "MapEvent{member: " << event.member << " eventType: " << event.eventType << " name: " << event.name
               << " numberOfEntriesAffected: " << event.numberOfEntriesAffected;
            return os;
        }
    }
}


namespace hazelcast {
    namespace client {
        Endpoint::Endpoint(std::shared_ptr<std::string> uuid, std::shared_ptr<Address> socketAddress) : uuid(uuid),
                                                                                                        socketAddress(
                                                                                                                socketAddress) {}

        const std::shared_ptr<std::string> &Endpoint::getUuid() const {
            return uuid;
        }

        const std::shared_ptr<Address> &Endpoint::getSocketAddress() const {
            return socketAddress;
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace config {

            ClientFlakeIdGeneratorConfig::ClientFlakeIdGeneratorConfig(const std::string &name)
                    : name(name), prefetchCount(DEFAULT_PREFETCH_COUNT),
                      prefetchValidityMillis(DEFAULT_PREFETCH_VALIDITY_MILLIS) {}

            const std::string &ClientFlakeIdGeneratorConfig::getName() const {
                return name;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::setName(const std::string &name) {
                ClientFlakeIdGeneratorConfig::name = name;
                return *this;
            }

            int32_t ClientFlakeIdGeneratorConfig::getPrefetchCount() const {
                return prefetchCount;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::setPrefetchCount(int32_t prefetchCount) {
                std::ostringstream out;
                out << "prefetch-count must be 1.." << MAXIMUM_PREFETCH_COUNT << ", not " << prefetchCount;
                util::Preconditions::checkTrue(prefetchCount > 0 && prefetchCount <= MAXIMUM_PREFETCH_COUNT, out.str());
                ClientFlakeIdGeneratorConfig::prefetchCount = prefetchCount;
                return *this;
            }

            int64_t ClientFlakeIdGeneratorConfig::getPrefetchValidityMillis() const {
                return prefetchValidityMillis;
            }

            ClientFlakeIdGeneratorConfig &
            ClientFlakeIdGeneratorConfig::setPrefetchValidityMillis(int64_t prefetchValidityMillis) {
                util::Preconditions::checkNotNegative(prefetchValidityMillis,
                                                      "prefetchValidityMs must be non negative");
                ClientFlakeIdGeneratorConfig::prefetchValidityMillis = prefetchValidityMillis;
                return *this;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace config {
            SSLConfig::SSLConfig() : enabled(false), sslProtocol(tlsv12) {
            }

            bool SSLConfig::isEnabled() const {
                return enabled;
            }

            SSLConfig &SSLConfig::setEnabled(bool enabled) {
                util::Preconditions::checkSSL("getAwsConfig");
                this->enabled = enabled;
                return *this;
            }

            SSLConfig &SSLConfig::setProtocol(SSLProtocol protocol) {
                this->sslProtocol = protocol;
                return *this;
            }

            SSLProtocol SSLConfig::getProtocol() const {
                return sslProtocol;
            }

            const std::vector<std::string> &SSLConfig::getVerifyFiles() const {
                return clientVerifyFiles;
            }

            SSLConfig &SSLConfig::addVerifyFile(const std::string &filename) {
                this->clientVerifyFiles.push_back(filename);
                return *this;
            }

            const std::string &SSLConfig::getCipherList() const {
                return cipherList;
            }

            SSLConfig &SSLConfig::setCipherList(const std::string &ciphers) {
                this->cipherList = ciphers;
                return *this;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace config {
            int32_t ClientNetworkConfig::CONNECTION_ATTEMPT_PERIOD = 3000;

            ClientNetworkConfig::ClientNetworkConfig()
                    : connectionTimeout(5000), smartRouting(true), connectionAttemptLimit(-1),
                      connectionAttemptPeriod(CONNECTION_ATTEMPT_PERIOD) {
            }

            SSLConfig &ClientNetworkConfig::getSSLConfig() {
                return sslConfig;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSSLConfig(const config::SSLConfig &sslConfig) {
                this->sslConfig = sslConfig;
                return *this;
            }

            int64_t ClientNetworkConfig::getConnectionTimeout() const {
                return connectionTimeout;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionTimeout(int64_t connectionTimeoutInMillis) {
                this->connectionTimeout = connectionTimeoutInMillis;
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::setAwsConfig(const ClientAwsConfig &clientAwsConfig) {
                this->clientAwsConfig = clientAwsConfig;
                return *this;
            }

            ClientAwsConfig &ClientNetworkConfig::getAwsConfig() {
                return clientAwsConfig;
            }

            bool ClientNetworkConfig::isSmartRouting() const {
                return smartRouting;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSmartRouting(bool smartRouting) {
                ClientNetworkConfig::smartRouting = smartRouting;
                return *this;
            }

            int32_t ClientNetworkConfig::getConnectionAttemptLimit() const {
                return connectionAttemptLimit;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionAttemptLimit(int32_t connectionAttemptLimit) {
                if (connectionAttemptLimit < 0) {
                    throw exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptLimit",
                                                              "connectionAttemptLimit cannot be negative");
                }
                this->connectionAttemptLimit = connectionAttemptLimit;
                return *this;
            }

            int32_t ClientNetworkConfig::getConnectionAttemptPeriod() const {
                return connectionAttemptPeriod;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionAttemptPeriod(int32_t connectionAttemptPeriod) {
                if (connectionAttemptPeriod < 0) {
                    throw exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptPeriod",
                                                              "connectionAttemptPeriod cannot be negative");
                }
                this->connectionAttemptPeriod = connectionAttemptPeriod;
                return *this;
            }

            std::vector<Address> ClientNetworkConfig::getAddresses() const {
                return addressList;
            }

            ClientNetworkConfig &ClientNetworkConfig::addAddresses(const std::vector<Address> &addresses) {
                addressList.insert(addressList.end(), addresses.begin(), addresses.end());
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::setAddresses(const std::vector<Address> &addresses) {
                addressList = addresses;
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::addAddress(const Address &address) {
                addressList.push_back(address);
                return *this;
            }

            SocketOptions &ClientNetworkConfig::getSocketOptions() {
                return socketOptions;
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace config {

            const std::string &LoggerConfig::getConfigurationFileName() const {
                return configurationFileName;
            }

            void LoggerConfig::setConfigurationFileName(const std::string &fileName) {
                LoggerConfig::configurationFileName = fileName;
            }

            LoggerConfig::LoggerConfig() : type(Type::EASYLOGGINGPP), logLevel(LoggerLevel::INFO) {}

            LoggerConfig::Type::LoggerType LoggerConfig::getType() const {
                return type;
            }

            void LoggerConfig::setType(LoggerConfig::Type::LoggerType type) {
                LoggerConfig::type = type;
            }

            LoggerLevel::Level LoggerConfig::getLogLevel() const {
                return logLevel;
            }

            void LoggerConfig::setLogLevel(LoggerLevel::Level logLevel) {
                LoggerConfig::logLevel = logLevel;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace config {
            namespace matcher {
                std::shared_ptr<std::string>
                MatchingPointConfigPatternMatcher::matches(const std::vector<std::string> &configPatterns,
                                                           const std::string &itemName) const {
                    std::shared_ptr<std::string> candidate;
                    std::shared_ptr<std::string> duplicate;
                    int lastMatchingPoint = -1;
                    for (const std::string &pattern  : configPatterns) {
                        int matchingPoint = getMatchingPoint(pattern, itemName);
                        if (matchingPoint > -1 && matchingPoint >= lastMatchingPoint) {
                            if (matchingPoint == lastMatchingPoint) {
                                duplicate = candidate;
                            } else {
                                duplicate.reset();
                            }
                            lastMatchingPoint = matchingPoint;
                            candidate.reset(new std::string(pattern));
                        }
                    }
                    if (duplicate.get() != NULL) {
                        throw (exception::ExceptionBuilder<exception::ConfigurationException>(
                                "MatchingPointConfigPatternMatcher::matches") << "Configuration " << itemName
                                                                              << " has duplicate configuration. Candidate:"
                                                                              << *candidate << ", duplicate:"
                                                                              << *duplicate).build();
                    }
                    return candidate;
                }

                int MatchingPointConfigPatternMatcher::getMatchingPoint(const std::string &pattern,
                                                                        const std::string &itemName) const {
                    size_t index = pattern.find('*');
                    if (index == std::string::npos) {
                        return -1;
                    }

                    std::string firstPart = pattern.substr(0, index);
                    if (itemName.find(firstPart) != 0) {
                        return -1;
                    }

                    std::string secondPart = pattern.substr(index + 1);
                    if (itemName.rfind(secondPart) != (itemName.length() - secondPart.length())) {
                        return -1;
                    }

                    return (int) (firstPart.length() + secondPart.length());
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace config {
            ClientConnectionStrategyConfig::ClientConnectionStrategyConfig() : asyncStart(false), reconnectMode(ON) {
            }

            ClientConnectionStrategyConfig::ReconnectMode ClientConnectionStrategyConfig::getReconnectMode() const {
                return reconnectMode;
            }

            bool ClientConnectionStrategyConfig::isAsyncStart() const {
                return asyncStart;
            }

            ClientConnectionStrategyConfig &ClientConnectionStrategyConfig::setAsyncStart(bool asyncStart) {
                this->asyncStart = asyncStart;
                return *this;
            }

            ClientConnectionStrategyConfig &
            ClientConnectionStrategyConfig::setReconnectMode(ReconnectMode reconnectMode) {
                this->reconnectMode = reconnectMode;
                return *this;
            }

        }
    }
}


namespace hazelcast {
    namespace client {
        namespace config {
            const int ReliableTopicConfig::DEFAULT_READ_BATCH_SIZE = 10;

            ReliableTopicConfig::ReliableTopicConfig() {

            }

            ReliableTopicConfig::ReliableTopicConfig(const char *topicName) : readBatchSize(DEFAULT_READ_BATCH_SIZE),
                                                                              name(topicName) {
            }

            const std::string &ReliableTopicConfig::getName() const {
                return name;
            }

            int ReliableTopicConfig::getReadBatchSize() const {
                return readBatchSize;
            }

            ReliableTopicConfig &ReliableTopicConfig::setReadBatchSize(int batchSize) {
                if (batchSize <= 0) {
                    throw exception::IllegalArgumentException("ReliableTopicConfig::setReadBatchSize",
                                                              "readBatchSize should be positive");
                }

                this->readBatchSize = batchSize;

                return *this;
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace config {
            SocketOptions::SocketOptions() : tcpNoDelay(true), keepAlive(true), reuseAddress(true), lingerSeconds(3),
                                             bufferSize(DEFAULT_BUFFER_SIZE_BYTE) {}

            bool SocketOptions::isTcpNoDelay() const {
                return tcpNoDelay;
            }

            SocketOptions &SocketOptions::setTcpNoDelay(bool tcpNoDelay) {
                SocketOptions::tcpNoDelay = tcpNoDelay;
                return *this;
            }

            bool SocketOptions::isKeepAlive() const {
                return keepAlive;
            }

            SocketOptions &SocketOptions::setKeepAlive(bool keepAlive) {
                SocketOptions::keepAlive = keepAlive;
                return *this;
            }

            bool SocketOptions::isReuseAddress() const {
                return reuseAddress;
            }

            SocketOptions &SocketOptions::setReuseAddress(bool reuseAddress) {
                SocketOptions::reuseAddress = reuseAddress;
                return *this;
            }

            int SocketOptions::getLingerSeconds() const {
                return lingerSeconds;
            }

            SocketOptions &SocketOptions::setLingerSeconds(int lingerSeconds) {
                SocketOptions::lingerSeconds = lingerSeconds;
                return *this;
            }

            int SocketOptions::getBufferSizeInBytes() const {
                return bufferSize;
            }

            SocketOptions &SocketOptions::setBufferSizeInBytes(int bufferSize) {
                SocketOptions::bufferSize = bufferSize;
                return *this;
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace config {
            ClientAwsConfig::ClientAwsConfig() : enabled(false), region("us-east-1"), hostHeader("ec2.amazonaws.com"),
                                                 insideAws(false) {
            }

            const std::string &ClientAwsConfig::getAccessKey() const {
                return accessKey;
            }

            ClientAwsConfig &ClientAwsConfig::setAccessKey(const std::string &accessKey) {
                this->accessKey = util::Preconditions::checkHasText(accessKey, "accessKey must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getSecretKey() const {
                return secretKey;
            }

            ClientAwsConfig &ClientAwsConfig::setSecretKey(const std::string &secretKey) {
                this->secretKey = util::Preconditions::checkHasText(secretKey, "secretKey must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getRegion() const {
                return region;
            }

            ClientAwsConfig &ClientAwsConfig::setRegion(const std::string &region) {
                this->region = util::Preconditions::checkHasText(region, "region must contain text");
                return *this;
            }

            const std::string &ClientAwsConfig::getHostHeader() const {
                return hostHeader;
            }

            ClientAwsConfig &ClientAwsConfig::setHostHeader(const std::string &hostHeader) {
                this->hostHeader = util::Preconditions::checkHasText(hostHeader, "hostHeader must contain text");
                return *this;
            }

            ClientAwsConfig &ClientAwsConfig::setEnabled(bool enabled) {
                util::Preconditions::checkSSL("getAwsConfig");
                this->enabled = enabled;
                return *this;
            }

            bool ClientAwsConfig::isEnabled() const {
                return enabled;
            }

            ClientAwsConfig &ClientAwsConfig::setSecurityGroupName(const std::string &securityGroupName) {
                this->securityGroupName = securityGroupName;
                return *this;
            }

            const std::string &ClientAwsConfig::getSecurityGroupName() const {
                return securityGroupName;
            }

            ClientAwsConfig &ClientAwsConfig::setTagKey(const std::string &tagKey) {
                this->tagKey = tagKey;
                return *this;
            }

            const std::string &ClientAwsConfig::getTagKey() const {
                return tagKey;
            }

            ClientAwsConfig &ClientAwsConfig::setTagValue(const std::string &tagValue) {
                this->tagValue = tagValue;
                return *this;
            }

            const std::string &ClientAwsConfig::getTagValue() const {
                return tagValue;
            }

            const std::string &ClientAwsConfig::getIamRole() const {
                return iamRole;
            }

            ClientAwsConfig &ClientAwsConfig::setIamRole(const std::string &iamRole) {
                this->iamRole = iamRole;
                return *this;
            }

            bool ClientAwsConfig::isInsideAws() const {
                return insideAws;
            }

            ClientAwsConfig &ClientAwsConfig::setInsideAws(bool insideAws) {
                this->insideAws = insideAws;
                return *this;
            }

            std::ostream &operator<<(std::ostream &out, const ClientAwsConfig &config) {
                return out << "ClientAwsConfig{"
                           << "enabled=" << config.isEnabled()
                           << ", region='" << config.getRegion() << '\''
                           << ", securityGroupName='" << config.getSecurityGroupName() << '\''
                           << ", tagKey='" << config.getTagKey() << '\''
                           << ", tagValue='" << config.getTagValue() << '\''
                           << ", hostHeader='" << config.getHostHeader() << '\''
                           << ", iamRole='" << config.getIamRole() << "\'}";
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace mixedtype {
            MultiMap::MultiMap(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::MultiMapImpl(instanceName, context) {
            }

            std::vector<TypedData> MultiMap::keySet() {
                return toTypedDataCollection(proxy::MultiMapImpl::keySetData());
            }

            std::vector<TypedData> MultiMap::values() {
                return toTypedDataCollection(proxy::MultiMapImpl::valuesData());
            }

            std::vector<std::pair<TypedData, TypedData> > MultiMap::entrySet() {
                return toTypedDataEntrySet(proxy::MultiMapImpl::entrySetData());
            }

            int MultiMap::size() {
                return proxy::MultiMapImpl::size();
            }

            void MultiMap::clear() {
                proxy::MultiMapImpl::clear();
            }

            std::string MultiMap::addEntryListener(MixedEntryListener &listener, bool includeValue) {
                spi::ClientClusterService &clusterService = getContext().getClientClusterService();
                serialization::pimpl::SerializationService &ss = getContext().getSerializationService();
                impl::MixedEntryEventHandler<protocol::codec::MultiMapAddEntryListenerCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedEntryEventHandler<protocol::codec::MultiMapAddEntryListenerCodec::AbstractEventHandler>(
                                getName(), clusterService, ss, listener, includeValue);
                return proxy::MultiMapImpl::addEntryListener(entryEventHandler, includeValue);
            }

            bool MultiMap::removeEntryListener(const std::string &registrationId) {
                return proxy::MultiMapImpl::removeEntryListener(registrationId);
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            namespace impl {
                HazelcastClientImpl::HazelcastClientImpl(client::impl::HazelcastClientInstanceImpl &client) : client(
                        client) {
                }

                HazelcastClientImpl::~HazelcastClientImpl() {
                }

                IMap HazelcastClientImpl::getMap(const std::string &name) {
                    map::impl::MapMixedTypeProxyFactory factory(&client.clientContext);
                    std::shared_ptr<spi::ClientProxy> proxy =
                            client.getDistributedObjectForService("hz:impl:mapService", name, factory);
                    return IMap(std::static_pointer_cast<ClientMapProxy>(proxy));
                }

                MultiMap HazelcastClientImpl::getMultiMap(const std::string &name) {
                    return client.getDistributedObject<MultiMap>(name);
                }

                IQueue HazelcastClientImpl::getQueue(const std::string &name) {
                    return client.getDistributedObject<IQueue>(name);
                }

                ISet HazelcastClientImpl::getSet(const std::string &name) {
                    return client.getDistributedObject<ISet>(name);
                }

                IList HazelcastClientImpl::getList(const std::string &name) {
                    return client.getDistributedObject<IList>(name);
                }

                ITopic HazelcastClientImpl::getTopic(const std::string &name) {
                    return client.getDistributedObject<ITopic>(name);
                }

                Ringbuffer HazelcastClientImpl::getRingbuffer(
                        const std::string &instanceName) {
                    return client.getDistributedObject<Ringbuffer>(instanceName);
                }

            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace mixedtype {
            std::string IQueue::addItemListener(MixedItemListener &listener, bool includeValue) {
                spi::ClientClusterService &cs = getContext().getClientClusterService();
                serialization::pimpl::SerializationService &ss = getContext().getSerializationService();
                impl::MixedItemEventHandler<protocol::codec::QueueAddListenerCodec::AbstractEventHandler> *itemEventHandler =
                        new impl::MixedItemEventHandler<protocol::codec::QueueAddListenerCodec::AbstractEventHandler>(
                                getName(), cs, ss, listener);
                return proxy::IQueueImpl::addItemListener(itemEventHandler, includeValue);
            }

            bool IQueue::removeItemListener(const std::string &registrationId) {
                return proxy::IQueueImpl::removeItemListener(registrationId);
            }

            int IQueue::remainingCapacity() {
                return proxy::IQueueImpl::remainingCapacity();
            }

            TypedData IQueue::take() {
                return poll(-1);
            }

            size_t IQueue::drainTo(std::vector<TypedData> &elements) {
                return drainTo(elements, -1);
            }

            size_t IQueue::drainTo(std::vector<TypedData> &elements, int64_t maxElements) {
                typedef std::vector<serialization::pimpl::Data> DATA_VECTOR;
                serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
                size_t numElements = 0;
                for (const DATA_VECTOR::value_type data : proxy::IQueueImpl::drainToData((size_t) maxElements)) {
                    elements.push_back(TypedData(std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(data)), serializationService));
                    ++numElements;
                }
                return numElements;
            }

            TypedData IQueue::poll() {
                return poll(0);
            }

            TypedData IQueue::poll(long timeoutInMillis) {
                return TypedData(proxy::IQueueImpl::pollData(timeoutInMillis), getContext().getSerializationService());
            }

            TypedData IQueue::peek() {
                return TypedData(proxy::IQueueImpl::peekData(), getContext().getSerializationService());
            }

            int IQueue::size() {
                return proxy::IQueueImpl::size();
            }

            bool IQueue::isEmpty() {
                return size() == 0;
            }

            std::vector<TypedData> IQueue::toArray() {
                return toTypedDataCollection(proxy::IQueueImpl::toArrayData());
            }

            void IQueue::clear() {
                proxy::IQueueImpl::clear();
            }

            IQueue::IQueue(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::IQueueImpl(instanceName, context) {
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            IMap::IMap(std::shared_ptr<mixedtype::ClientMapProxy> proxy) : mapImpl(proxy) {
            }

            void IMap::removeAll(const query::Predicate &predicate) {
                mapImpl->removeAll(predicate);
            }

            void IMap::flush() {
                mapImpl->flush();
            }

            void IMap::removeInterceptor(const std::string &id) {
                mapImpl->removeInterceptor(id);
            }

            std::string IMap::addEntryListener(MixedEntryListener &listener, bool includeValue) {
                return mapImpl->addEntryListener(listener, includeValue);
            }

            std::string
            IMap::addEntryListener(MixedEntryListener &listener, const query::Predicate &predicate,
                                   bool includeValue) {
                return mapImpl->addEntryListener(listener, predicate, includeValue);
            }

            bool IMap::removeEntryListener(const std::string &registrationId) {
                return mapImpl->removeEntryListener(registrationId);
            }

            void IMap::evictAll() {
                mapImpl->evictAll();
            }

            std::vector<TypedData> IMap::keySet() {
                return mapImpl->keySet();
            }

            std::vector<TypedData> IMap::keySet(const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->keySet(predicate);
            }

            std::vector<TypedData> IMap::keySet(const query::Predicate &predicate) {
                return mapImpl->keySet(predicate);
            }

            std::vector<TypedData> IMap::values() {
                return mapImpl->values();
            }

            std::vector<TypedData> IMap::values(const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->values(predicate);
            }

            std::vector<TypedData> IMap::values(const query::Predicate &predicate) {
                return mapImpl->values(predicate);
            }

            std::vector<std::pair<TypedData, TypedData> > IMap::entrySet() {
                return mapImpl->entrySet();
            }

            std::vector<std::pair<TypedData, TypedData> >
            IMap::entrySet(const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->entrySet(predicate);
            }

            std::vector<std::pair<TypedData, TypedData> > IMap::entrySet(const query::Predicate &predicate) {
                return mapImpl->entrySet(predicate);
            }

            void IMap::addIndex(const std::string &attribute, bool ordered) {
                mapImpl->addIndex(attribute, ordered);
            }

            int IMap::size() {
                return mapImpl->size();
            }

            bool IMap::isEmpty() {
                return mapImpl->isEmpty();
            }

            void IMap::clear() {
                return mapImpl->clear();
            }

            void IMap::destroy() {
                mapImpl->destroy();
            }

            monitor::LocalMapStats &IMap::getLocalMapStats() {
                return mapImpl->getLocalMapStats();
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            Ringbuffer::Ringbuffer(const std::string &objectName, spi::ClientContext *context) : proxy::ProxyImpl(
                    "hz:impl:ringbufferService", objectName, context), bufferCapacity(-1) {
                partitionId = getPartitionId(toData(objectName));
            }

            Ringbuffer::Ringbuffer(const Ringbuffer &rhs) : proxy::ProxyImpl(rhs), partitionId(rhs.partitionId),
                                                            bufferCapacity(
                                                                    const_cast<Ringbuffer &>(rhs).bufferCapacity.load()) {
            }

            Ringbuffer::~Ringbuffer() {
            }

            int64_t Ringbuffer::capacity() {
                if (-1 == bufferCapacity) {
                    std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferCapacityCodec::encodeRequest(
                            getName());
                    bufferCapacity = invokeAndGetResult<int64_t, protocol::codec::RingbufferCapacityCodec::ResponseParameters>(
                            msg, partitionId);
                }
                return bufferCapacity;
            }

            int64_t Ringbuffer::size() {
                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferSizeCodec::encodeRequest(
                        getName());
                return invokeAndGetResult<int64_t, protocol::codec::RingbufferSizeCodec::ResponseParameters>(msg,
                                                                                                             partitionId);
            }

            int64_t Ringbuffer::tailSequence() {
                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferTailSequenceCodec::encodeRequest(
                        getName());
                return invokeAndGetResult<int64_t, protocol::codec::RingbufferTailSequenceCodec::ResponseParameters>(
                        msg, partitionId);
            }

            int64_t Ringbuffer::headSequence() {
                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferHeadSequenceCodec::encodeRequest(
                        getName());
                return invokeAndGetResult<int64_t, protocol::codec::RingbufferHeadSequenceCodec::ResponseParameters>(
                        msg, partitionId);
            }

            int64_t Ringbuffer::remainingCapacity() {
                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferRemainingCapacityCodec::encodeRequest(
                        getName());
                return invokeAndGetResult<int64_t, protocol::codec::RingbufferRemainingCapacityCodec::ResponseParameters>(
                        msg, partitionId);
            }

            TypedData Ringbuffer::readOne(int64_t sequence) {
                checkSequence(sequence);

                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferReadOneCodec::encodeRequest(
                        getName(), sequence);

                std::unique_ptr<serialization::pimpl::Data> itemData = invokeAndGetResult<
                        std::unique_ptr<serialization::pimpl::Data>, protocol::codec::RingbufferReadOneCodec::ResponseParameters>(
                        msg, partitionId);

                return TypedData(itemData, getContext().getSerializationService());
            }

            void Ringbuffer::checkSequence(int64_t sequence) {
                if (sequence < 0) {
                    throw (exception::ExceptionBuilder<exception::IllegalArgumentException>(
                            "Ringbuffer::checkSequence") << "sequence can't be smaller than 0, but was: "
                                                         << sequence).build();
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            std::string ITopic::addMessageListener(topic::MessageListener &listener) {
                client::impl::BaseEventHandler *topicEventHandler = new mixedtype::topic::impl::TopicEventHandlerImpl(
                        getName(), getContext().getClientClusterService(),
                        getContext().getSerializationService(),
                        listener);
                return proxy::ITopicImpl::addMessageListener(topicEventHandler);
            }

            bool ITopic::removeMessageListener(const std::string &registrationId) {
                return proxy::ITopicImpl::removeMessageListener(registrationId);
            }

            ITopic::ITopic(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ITopicImpl(instanceName, context) {
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            std::string ISet::addItemListener(MixedItemListener &listener, bool includeValue) {
                impl::MixedItemEventHandler<protocol::codec::SetAddListenerCodec::AbstractEventHandler> *itemEventHandler =
                        new impl::MixedItemEventHandler<protocol::codec::SetAddListenerCodec::AbstractEventHandler>(
                                getName(), getContext().getClientClusterService(),
                                getContext().getSerializationService(), listener);
                return proxy::ISetImpl::addItemListener(itemEventHandler, includeValue);
            }

            bool ISet::removeItemListener(const std::string &registrationId) {
                return proxy::ISetImpl::removeItemListener(registrationId);
            }

            int ISet::size() {
                return proxy::ISetImpl::size();
            }

            bool ISet::isEmpty() {
                return proxy::ISetImpl::isEmpty();
            }

            std::vector<TypedData> ISet::toArray() {
                return toTypedDataCollection(proxy::ISetImpl::toArrayData());
            }

            void ISet::clear() {
                proxy::ISetImpl::clear();
            }

            ISet::ISet(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ISetImpl(instanceName, context) {
            }
        }
    }
}


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
