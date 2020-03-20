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
        namespace exception {
            UndefinedErrorCodeException::UndefinedErrorCodeException(const std::string &source,
                                                                     const std::string &message,
                                                                     int32_t errorCode, int64_t correlationId,
                                                                     std::string details)
                    : IException("UndefinedErrorCodeException", source, message, protocol::UNDEFINED, true),
                      error(errorCode),
                      messageCallId(correlationId),
                      detailedErrorMessage(details) {
            }

            int32_t UndefinedErrorCodeException::getUndefinedErrorCode() const {
                return error;
            }

            int64_t UndefinedErrorCodeException::getMessageCallId() const {
                return messageCallId;
            }

            const std::string &UndefinedErrorCodeException::getDetailedErrorMessage() const {
                return detailedErrorMessage;
            }

            UndefinedErrorCodeException::~UndefinedErrorCodeException() throw() {
            }

            std::unique_ptr<IException> UndefinedErrorCodeException::clone() const {
                return std::unique_ptr<IException>(new UndefinedErrorCodeException(*this));
            }

            void UndefinedErrorCodeException::raise() const {
                throw *this;
            }

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     const std::string &details, int32_t causeCode)
                    : IException("RetryableHazelcastException", source, message, details, protocol::RETRYABLE_HAZELCAST,
                                 causeCode, true, true), HazelcastException(source, message, details, causeCode) {
            }

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message)
                    : IException("RetryableHazelcastException", source, message, protocol::RETRYABLE_HAZELCAST, true,
                                 true),
                      HazelcastException(source, message) {
            }

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     int32_t causeCode) : IException(
                    "RetryableHazelcastException", source, message, protocol::RETRYABLE_HAZELCAST, causeCode, true,
                    true),
                                                                                          HazelcastException(source,
                                                                                                             message,
                                                                                                             causeCode) {}

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     const std::shared_ptr<IException> &cause)
                    : IException("RetryableHazelcastException", source, message, protocol::RETRYABLE_HAZELCAST, cause,
                                 true, true),
                      HazelcastException(source, message, cause) {}

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message,
                                                     const std::string &details, int32_t causeCode)
                    : IException("MemberLeftException", source, message, details, protocol::MEMBER_LEFT, causeCode,
                                 true),
                      ExecutionException(source, message, details, causeCode),
                      RetryableHazelcastException(source, message, details, causeCode) {
            }

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message,
                                                     int32_t causeCode) : IException("MemberLeftException", source,
                                                                                     message, protocol::MEMBER_LEFT,
                                                                                     causeCode, true, true),
                                                                          ExecutionException(source, message,
                                                                                             causeCode),
                                                                          RetryableHazelcastException(source, message,
                                                                                                      causeCode) {
            }

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message)
                    : IException("MemberLeftException", source, message, protocol::MEMBER_LEFT, true, true),
                      ExecutionException(source, message), RetryableHazelcastException(source, message) {
            }

            void MemberLeftException::raise() const {
                throw *this;
            }

            std::unique_ptr<IException> MemberLeftException::clone() const {
                return std::unique_ptr<IException>(new MemberLeftException(*this));
            }

        }
    }
}


namespace hazelcast {
    namespace client {

        ClientConfig::ClientConfig()
                : loadBalancer(NULL), redoOperation(false), socketInterceptor(NULL), credentials(NULL),
                  executorPoolSize(-1) {
        }

        ClientConfig &ClientConfig::addAddress(const Address &address) {
            networkConfig.addAddress(address);
            return (*this);
        }

        ClientConfig &ClientConfig::addAddresses(const std::vector<Address> &addresses) {
            networkConfig.addAddresses(addresses);
            return (*this);
        }


        std::set<Address> ClientConfig::getAddresses() {
            std::set<Address> result;
            for (const Address &address : networkConfig.getAddresses()) {
                result.insert(address);
            }
            return result;
        }

        ClientConfig &ClientConfig::setGroupConfig(const GroupConfig &groupConfig) {
            this->groupConfig = groupConfig;
            return *this;
        }


        GroupConfig &ClientConfig::getGroupConfig() {
            return groupConfig;
        }


        ClientConfig &ClientConfig::setConnectionAttemptLimit(int connectionAttemptLimit) {
            networkConfig.setConnectionAttemptLimit(connectionAttemptLimit);
            return *this;
        }

        int ClientConfig::getConnectionAttemptLimit() const {
            return networkConfig.getConnectionAttemptLimit();
        }

        ClientConfig &ClientConfig::setConnectionTimeout(int connectionTimeoutInMillis) {
            this->networkConfig.setConnectionTimeout(connectionTimeoutInMillis);
            return *this;
        }

        int ClientConfig::getConnectionTimeout() const {
            return (int) this->networkConfig.getConnectionTimeout();
        }

        ClientConfig &ClientConfig::setAttemptPeriod(int attemptPeriodInMillis) {
            networkConfig.setConnectionAttemptPeriod(attemptPeriodInMillis);
            return *this;
        }

        int ClientConfig::getAttemptPeriod() const {
            return networkConfig.getConnectionAttemptPeriod();
        }

        ClientConfig &ClientConfig::setRedoOperation(bool redoOperation) {
            this->redoOperation = redoOperation;
            return *this;
        }

        bool ClientConfig::isRedoOperation() const {
            return redoOperation;
        }

        LoadBalancer *const ClientConfig::getLoadBalancer() {
            if (loadBalancer == NULL)
                return &defaultLoadBalancer;
            return loadBalancer;
        }

        ClientConfig &ClientConfig::setLoadBalancer(LoadBalancer *loadBalancer) {
            this->loadBalancer = loadBalancer;
            return *this;
        }

        ClientConfig &ClientConfig::setLogLevel(LogLevel loggerLevel) {
            this->loggerConfig.setLogLevel((LoggerLevel::Level) loggerLevel);
            return *this;
        }

        config::LoggerConfig &ClientConfig::getLoggerConfig() {
            return loggerConfig;
        }

        ClientConfig &ClientConfig::addListener(LifecycleListener *listener) {
            lifecycleListeners.insert(listener);
            return *this;
        }

        ClientConfig &ClientConfig::addListener(MembershipListener *listener) {
            if (listener == NULL) {
                throw exception::NullPointerException("ClientConfig::addListener(MembershipListener *)",
                                                      "listener can't be null");
            }

            membershipListeners.insert(listener);
            managedMembershipListeners.insert(
                    std::shared_ptr<MembershipListener>(new MembershipListenerDelegator(listener)));
            return *this;
        }

        ClientConfig &ClientConfig::addListener(InitialMembershipListener *listener) {
            if (listener == NULL) {
                throw exception::NullPointerException("ClientConfig::addListener(InitialMembershipListener *)",
                                                      "listener can't be null");
            }

            membershipListeners.insert(listener);
            managedMembershipListeners.insert(
                    std::shared_ptr<MembershipListener>(new InitialMembershipListenerDelegator(listener)));
            return *this;
        }

        ClientConfig &ClientConfig::addListener(const std::shared_ptr<MembershipListener> &listener) {
            membershipListeners.insert(listener.get());
            managedMembershipListeners.insert(listener);
            return *this;
        }

        ClientConfig &ClientConfig::addListener(const std::shared_ptr<InitialMembershipListener> &listener) {
            membershipListeners.insert(listener.get());
            managedMembershipListeners.insert(listener);
            return *this;
        }

        const std::set<LifecycleListener *> &ClientConfig::getLifecycleListeners() const {
            return lifecycleListeners;
        }

        const std::set<MembershipListener *> &ClientConfig::getMembershipListeners() const {
            return membershipListeners;
        }

        ClientConfig &ClientConfig::setCredentials(Credentials *credentials) {
            this->credentials = credentials;
            return *this;
        }

        const Credentials *ClientConfig::getCredentials() {
            return credentials;
        }

        ClientConfig &ClientConfig::setSocketInterceptor(SocketInterceptor *socketInterceptor) {
            this->socketInterceptor = socketInterceptor;
            return *this;
        }

        SocketInterceptor *ClientConfig::getSocketInterceptor() {
            return socketInterceptor;
        }

        ClientConfig &ClientConfig::setSmart(bool smart) {
            networkConfig.setSmartRouting(smart);
            return *this;
        }

        bool ClientConfig::isSmart() const {
            return networkConfig.isSmartRouting();
        }

        SerializationConfig &ClientConfig::getSerializationConfig() {
            return serializationConfig;
        }

        ClientConfig &ClientConfig::setSerializationConfig(SerializationConfig const &serializationConfig) {
            this->serializationConfig = serializationConfig;
            return *this;
        }


        std::map<std::string, std::string> &ClientConfig::getProperties() {
            return properties;
        }

        ClientConfig &ClientConfig::setProperty(const std::string &name, const std::string &value) {
            properties[name] = value;
            return *this;
        }

        ClientConfig &ClientConfig::addReliableTopicConfig(const config::ReliableTopicConfig &reliableTopicConfig) {
            reliableTopicConfigMap[reliableTopicConfig.getName()] = reliableTopicConfig;
            return *this;
        }

        const config::ReliableTopicConfig *ClientConfig::getReliableTopicConfig(const std::string &name) {
            std::map<std::string, config::ReliableTopicConfig>::const_iterator it = reliableTopicConfigMap.find(name);
            if (reliableTopicConfigMap.end() == it) {
                reliableTopicConfigMap[name] = config::ReliableTopicConfig(name.c_str());
            }
            return &reliableTopicConfigMap[name];
        }

        config::ClientNetworkConfig &ClientConfig::getNetworkConfig() {
            return networkConfig;
        }

        ClientConfig &ClientConfig::setNetworkConfig(const config::ClientNetworkConfig &networkConfig) {
            this->networkConfig = networkConfig;
            return *this;
        }

        const std::shared_ptr<mixedtype::config::MixedNearCacheConfig>
        ClientConfig::getMixedNearCacheConfig(const std::string &name) {
            return std::static_pointer_cast<mixedtype::config::MixedNearCacheConfig>(
                    getNearCacheConfig<TypedData, TypedData>(name));
        }

        const std::shared_ptr<std::string> &ClientConfig::getInstanceName() const {
            return instanceName;
        }

        void ClientConfig::setInstanceName(const std::shared_ptr<std::string> &instanceName) {
            ClientConfig::instanceName = instanceName;
        }

        int32_t ClientConfig::getExecutorPoolSize() const {
            return executorPoolSize;
        }

        void ClientConfig::setExecutorPoolSize(int32_t executorPoolSize) {
            ClientConfig::executorPoolSize = executorPoolSize;
        }

        config::ClientConnectionStrategyConfig &ClientConfig::getConnectionStrategyConfig() {
            return connectionStrategyConfig;
        }

        ClientConfig &ClientConfig::setConnectionStrategyConfig(
                const config::ClientConnectionStrategyConfig &connectionStrategyConfig) {
            ClientConfig::connectionStrategyConfig = connectionStrategyConfig;
            return *this;
        }

        std::shared_ptr<config::ClientFlakeIdGeneratorConfig>
        ClientConfig::findFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
                    configPatternMatcher, flakeIdGeneratorConfigMap, baseName);
            if (config.get() != NULL) {
                return config;
            }
            return getFlakeIdGeneratorConfig("default");
        }


        std::shared_ptr<config::ClientFlakeIdGeneratorConfig>
        ClientConfig::getFlakeIdGeneratorConfig(const std::string &name) {
            std::string baseName = internal::partition::strategy::StringPartitioningStrategy::getBaseName(name);
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = internal::config::ConfigUtils::lookupByPattern<config::ClientFlakeIdGeneratorConfig>(
                    configPatternMatcher, flakeIdGeneratorConfigMap, baseName);
            if (config.get() != NULL) {
                return config;
            }
            std::shared_ptr<config::ClientFlakeIdGeneratorConfig> defConfig = flakeIdGeneratorConfigMap.get("default");
            if (defConfig.get() == NULL) {
                defConfig.reset(new config::ClientFlakeIdGeneratorConfig("default"));
                flakeIdGeneratorConfigMap.put(defConfig->getName(), defConfig);
            }
            config.reset(new config::ClientFlakeIdGeneratorConfig(*defConfig));
            config->setName(name);
            flakeIdGeneratorConfigMap.put(config->getName(), config);
            return config;
        }

        ClientConfig &
        ClientConfig::addFlakeIdGeneratorConfig(const std::shared_ptr<config::ClientFlakeIdGeneratorConfig> &config) {
            flakeIdGeneratorConfigMap.put(config->getName(), config);
            return *this;
        }

        const std::set<std::shared_ptr<MembershipListener> > &ClientConfig::getManagedMembershipListeners() const {
            return managedMembershipListeners;
        }

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
