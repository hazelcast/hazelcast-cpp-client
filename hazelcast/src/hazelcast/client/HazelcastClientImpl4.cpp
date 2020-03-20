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
/*
 *
 *  Created on: May 17, 2016
 *      Author: ihsan
 */


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

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {
                    AbstractClientListenerService::AbstractClientListenerService(ClientContext &clientContext,
                                                                                 int32_t eventThreadCount,
                                                                                 int32_t eventQueueCapacity)
                            : clientContext(clientContext),
                              serializationService(clientContext.getSerializationService()),
                              logger(clientContext.getLogger()),
                              clientConnectionManager(clientContext.getConnectionManager()),
                              eventExecutor(logger, clientContext.getName() + ".event-", eventThreadCount,
                                            eventQueueCapacity),
                              registrationExecutor(logger, clientContext.getName() + ".eventRegistration-", 1) {
                        AbstractClientInvocationService &invocationService = (AbstractClientInvocationService &) clientContext.getInvocationService();
                        invocationTimeoutMillis = invocationService.getInvocationTimeoutMillis();
                        invocationRetryPauseMillis = invocationService.getInvocationRetryPauseMillis();
                    }

                    AbstractClientListenerService::~AbstractClientListenerService() {
                    }

                    std::string
                    AbstractClientListenerService::registerListener(
                            const std::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                            const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/
                        std::shared_ptr<util::Callable<std::string> > task(
                                new RegisterListenerTask("AbstractClientListenerService::registerListener",
                                                         shared_from_this(), listenerMessageCodec, handler));
                        return *registrationExecutor.submit<std::string>(task)->get();
                    }

                    bool AbstractClientListenerService::deregisterListener(const std::string &registrationId) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/

                        try {
                            std::shared_ptr<util::Future<bool> > future = registrationExecutor.submit(
                                    std::shared_ptr<util::Callable<bool> >(
                                            new DeregisterListenerTask(
                                                    "AbstractClientListenerService::deregisterListener",
                                                    shared_from_this(), registrationId)));

                            return *future->get();
                        } catch (exception::RejectedExecutionException &) {
                            //RejectedExecutionException executor(hence the client) is already shutdown
                            //listeners are cleaned up by the server side. We can ignore the exception and return true safely
                            return true;
                        }
                    }

                    void AbstractClientListenerService::connectionAdded(
                            const std::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        registrationExecutor.execute(
                                std::shared_ptr<util::Runnable>(
                                        new ConnectionAddedTask("AbstractClientListenerService::connectionAdded",
                                                                shared_from_this(), connection)));
                    }

                    void AbstractClientListenerService::connectionRemoved(
                            const std::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/

                        registrationExecutor.execute(
                                std::shared_ptr<util::Runnable>(
                                        new ConnectionRemovedTask("AbstractClientListenerService::connectionRemoved",
                                                                  shared_from_this(), connection)));
                    }

                    void AbstractClientListenerService::addEventHandler(int64_t callId,
                                                                        const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        eventHandlerMap.put(callId, handler);
                    }

                    void AbstractClientListenerService::removeEventHandler(int64_t callId) {
                        eventHandlerMap.remove(callId);
                    }

                    void AbstractClientListenerService::handleClientMessage(
                            const std::shared_ptr<protocol::ClientMessage> &clientMessage,
                            const std::shared_ptr<connection::Connection> &connection) {
                        try {
                            eventExecutor.execute(
                                    std::shared_ptr<util::StripedRunnable>(
                                            new ClientEventProcessor(clientMessage, connection, eventHandlerMap,
                                                                     logger)));
                        } catch (exception::RejectedExecutionException &e) {
                            logger.warning("Event clientMessage could not be handled. ", e);
                        }
                    }

                    void AbstractClientListenerService::shutdown() {
                        clientContext.getClientExecutionService().shutdownExecutor(eventExecutor.getThreadNamePrefix(),
                                                                                   eventExecutor, logger);
                        clientContext.getClientExecutionService().shutdownExecutor(
                                registrationExecutor.getThreadNamePrefix(), registrationExecutor, logger);
                    }

                    void AbstractClientListenerService::start() {
                        registrationExecutor.start();
                        eventExecutor.start();
                        clientConnectionManager.addConnectionListener(shared_from_this());
                    }

                    void AbstractClientListenerService::ClientEventProcessor::run() {
                        int64_t correlationId = clientMessage->getCorrelationId();
                        std::shared_ptr<EventHandler < protocol::ClientMessage> > eventHandler = eventHandlerMap.get(
                                correlationId);
                        if (eventHandler.get() == NULL) {
                            logger.warning("No eventHandler for callId: ", correlationId, ", event: ", *clientMessage);
                            return;
                        }

                        eventHandler->handle(clientMessage);
                    }

                    const std::string AbstractClientListenerService::ClientEventProcessor::getName() const {
                        return "AbstractClientListenerService::ClientEventProcessor";
                    }

                    int32_t AbstractClientListenerService::ClientEventProcessor::getKey() {
                        return clientMessage->getPartitionId();
                    }

                    AbstractClientListenerService::ClientEventProcessor::ClientEventProcessor(
                            const std::shared_ptr<protocol::ClientMessage> &clientMessage,
                            const std::shared_ptr<connection::Connection> &connection,
                            util::SynchronizedMap<int64_t, EventHandler<protocol::ClientMessage> > &eventHandlerMap,
                            util::ILogger &logger)
                            : clientMessage(clientMessage), eventHandlerMap(eventHandlerMap), logger(logger) {
                    }

                    AbstractClientListenerService::ClientEventProcessor::~ClientEventProcessor() {
                    }

                    AbstractClientListenerService::RegisterListenerTask::RegisterListenerTask(
                            const std::string &taskName,
                            const std::shared_ptr<AbstractClientListenerService> &listenerService,
                            const std::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                            const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) : taskName(
                            taskName), listenerService(listenerService), listenerMessageCodec(listenerMessageCodec),
                                                                                                      handler(handler) {}

                    std::shared_ptr<std::string> AbstractClientListenerService::RegisterListenerTask::call() {
                        return std::shared_ptr<std::string>(new std::string(
                                listenerService->registerListenerInternal(listenerMessageCodec, handler)));
                    }

                    const std::string AbstractClientListenerService::RegisterListenerTask::getName() const {
                        return taskName;
                    }

                    AbstractClientListenerService::DeregisterListenerTask::DeregisterListenerTask(
                            const std::string &taskName,
                            const std::shared_ptr<AbstractClientListenerService> &listenerService,
                            const std::string &registrationId) : taskName(taskName), listenerService(listenerService),
                                                                 registrationId(registrationId) {}

                    std::shared_ptr<bool> AbstractClientListenerService::DeregisterListenerTask::call() {
                        return std::shared_ptr<bool>(
                                new bool(listenerService->deregisterListenerInternal(registrationId)));
                    }

                    const std::string AbstractClientListenerService::DeregisterListenerTask::getName() const {
                        return taskName;
                    }

                    AbstractClientListenerService::ConnectionAddedTask::ConnectionAddedTask(const std::string &taskName,
                                                                                            const std::shared_ptr<AbstractClientListenerService> &listenerService,
                                                                                            const std::shared_ptr<connection::Connection> &connection)
                            : taskName(taskName), listenerService(listenerService), connection(connection) {}

                    const std::string AbstractClientListenerService::ConnectionAddedTask::getName() const {
                        return taskName;
                    }

                    void AbstractClientListenerService::ConnectionAddedTask::run() {
                        listenerService->connectionAddedInternal(connection);
                    }


                    AbstractClientListenerService::ConnectionRemovedTask::ConnectionRemovedTask(
                            const std::string &taskName,
                            const std::shared_ptr<AbstractClientListenerService> &listenerService,
                            const std::shared_ptr<connection::Connection> &connection) : taskName(taskName),
                                                                                         listenerService(
                                                                                                 listenerService),
                                                                                         connection(connection) {}

                    const std::string AbstractClientListenerService::ConnectionRemovedTask::getName() const {
                        return taskName;
                    }

                    void AbstractClientListenerService::ConnectionRemovedTask::run() {
                        listenerService->connectionRemovedInternal(connection);
                    }

                    std::string AbstractClientListenerService::registerListenerInternal(
                            const std::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                            const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        std::string userRegistrationId = util::UuidUtil::newUnsecureUuidString();

                        ClientRegistrationKey registrationKey(userRegistrationId, handler, listenerMessageCodec);
                        registrations.put(registrationKey, std::shared_ptr<ConnectionRegistrationsMap>(
                                new ConnectionRegistrationsMap()));
                        for (const std::shared_ptr<connection::Connection> &connection : clientConnectionManager.getActiveConnections()) {
                            try {
                                invoke(registrationKey, connection);
                            } catch (exception::IException &e) {
                                if (connection->isAlive()) {
                                    deregisterListenerInternal(userRegistrationId);
                                    throw (exception::ExceptionBuilder<exception::HazelcastException>(
                                            "AbstractClientListenerService::RegisterListenerTask::call")
                                            << "Listener can not be added " << e).build();
                                }
                            }
                        }
                        return userRegistrationId;
                    }

                    bool
                    AbstractClientListenerService::deregisterListenerInternal(const std::string &userRegistrationId) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        ClientRegistrationKey key(userRegistrationId);
                        std::shared_ptr<ConnectionRegistrationsMap> registrationMap = registrations.get(key);
                        if (registrationMap.get() == NULL) {
                            return false;
                        }
                        bool successful = true;

                        for (ConnectionRegistrationsMap::iterator it = registrationMap->begin();
                             it != registrationMap->end();) {
                            ClientEventRegistration &registration = (*it).second;
                            std::shared_ptr<connection::Connection> subscriber = registration.getSubscriber();
                            try {
                                const std::shared_ptr<ListenerMessageCodec> &listenerMessageCodec = registration.getCodec();
                                const std::string &serverRegistrationId = registration.getServerRegistrationId();
                                std::unique_ptr<protocol::ClientMessage> request = listenerMessageCodec->encodeRemoveRequest(
                                        serverRegistrationId);
                                std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(clientContext,
                                                                                                        request, "",
                                                                                                        subscriber);
                                invocation->invoke()->get();
                                removeEventHandler(registration.getCallId());

                                ConnectionRegistrationsMap::iterator oldEntry = it;
                                ++it;
                                registrationMap->erase(oldEntry);
                            } catch (exception::IException &e) {
                                ++it;

                                if (subscriber->isAlive()) {
                                    successful = false;
                                    std::ostringstream endpoint;
                                    if (subscriber->getRemoteEndpoint().get()) {
                                        endpoint << *subscriber->getRemoteEndpoint();
                                    } else {
                                        endpoint << "null";
                                    }
                                    logger.warning("AbstractClientListenerService::deregisterListenerInternal",
                                                   "Deregistration of listener with ID ", userRegistrationId,
                                                   " has failed to address ", subscriber->getRemoteEndpoint(), e);
                                }
                            }
                        }
                        if (successful) {
                            registrations.remove(key);
                        }
                        return successful;
                    }

                    void AbstractClientListenerService::connectionAddedInternal(
                            const std::shared_ptr<connection::Connection> &connection) {
                        for (const ClientRegistrationKey &registrationKey : registrations.keys()) {
                            invokeFromInternalThread(registrationKey, connection);
                        }
                    }

                    void AbstractClientListenerService::connectionRemovedInternal(
                            const std::shared_ptr<connection::Connection> &connection) {
                        typedef std::vector<std::pair<ClientRegistrationKey, std::shared_ptr<ConnectionRegistrationsMap> > > ENTRY_VECTOR;
                        for (const ENTRY_VECTOR::value_type &registrationMapEntry : registrations.entrySet()) {
                            std::shared_ptr<ConnectionRegistrationsMap> registrationMap = registrationMapEntry.second;
                            ConnectionRegistrationsMap::iterator foundRegistration = registrationMap->find(
                                    connection);
                            if (foundRegistration != registrationMap->end()) {
                                removeEventHandler(foundRegistration->second.getCallId());
                                registrationMap->erase(foundRegistration);
                                registrations.put(registrationMapEntry.first,
                                                  registrationMap);
                            }
                        }
                    }

                    void
                    AbstractClientListenerService::invokeFromInternalThread(
                            const ClientRegistrationKey &registrationKey,
                            const std::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        try {
                            invoke(registrationKey, connection);
                        } catch (exception::IException &e) {
                            logger.warning("Listener ", registrationKey, " can not be added to a new connection: ",
                                           *connection, ", reason: ", e.getMessage());
                        }
                    }

                    void
                    AbstractClientListenerService::invoke(const ClientRegistrationKey &registrationKey,
                                                          const std::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/

                        std::shared_ptr<ConnectionRegistrationsMap> registrationMap = registrations.get(
                                registrationKey);
                        if (registrationMap->find(connection) != registrationMap->end()) {
                            return;
                        }

                        const std::shared_ptr<ListenerMessageCodec> &codec = registrationKey.getCodec();
                        std::unique_ptr<protocol::ClientMessage> request = codec->encodeAddRequest(
                                registersLocalOnly());
                        std::shared_ptr<EventHandler < protocol::ClientMessage> >
                        handler = registrationKey.getHandler();
                        handler->beforeListenerRegister();

                        std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(clientContext,
                                                                                                request, "",
                                                                                                connection);
                        invocation->setEventHandler(handler);

                        std::shared_ptr<protocol::ClientMessage> clientMessage = invocation->invokeUrgent()->get();

                        std::string serverRegistrationId = codec->decodeAddResponse(*clientMessage);
                        handler->onListenerRegister();
                        int64_t correlationId = invocation->getClientMessage()->getCorrelationId();
                        ClientEventRegistration registration(serverRegistrationId, correlationId, connection, codec);

                        (*registrationMap)[connection] = registration;
                    }

                    bool AbstractClientListenerService::ConnectionPointerLessComparator::operator()(
                            const std::shared_ptr<connection::Connection> &lhs,
                            const std::shared_ptr<connection::Connection> &rhs) const {
                        if (lhs == rhs) {
                            return false;
                        }
                        if (!lhs.get()) {
                            return true;
                        }
                        if (!rhs.get()) {
                            return false;
                        }

                        return *lhs < *rhs;
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
                namespace listener {
                    ClientEventRegistration::ClientEventRegistration(const std::string &serverRegistrationId,
                                                                     int64_t callId,
                                                                     const std::shared_ptr<connection::Connection> &subscriber,
                                                                     const std::shared_ptr<ListenerMessageCodec> &codec)
                            : serverRegistrationId(serverRegistrationId), callId(callId), subscriber(subscriber),
                              codec(codec) {
                    }

                    const std::string &ClientEventRegistration::getServerRegistrationId() const {
                        return serverRegistrationId;
                    }

                    int64_t ClientEventRegistration::getCallId() const {
                        return callId;
                    }

                    const std::shared_ptr<connection::Connection> &ClientEventRegistration::getSubscriber() const {
                        return subscriber;
                    }

                    const std::shared_ptr<ListenerMessageCodec> &ClientEventRegistration::getCodec() const {
                        return codec;
                    }

                    bool ClientEventRegistration::operator==(const ClientEventRegistration &rhs) const {
                        return serverRegistrationId == rhs.serverRegistrationId;
                    }

                    bool ClientEventRegistration::operator!=(const ClientEventRegistration &rhs) const {
                        return !(rhs == *this);
                    }

                    bool ClientEventRegistration::operator<(const ClientEventRegistration &rhs) const {
                        return serverRegistrationId < rhs.serverRegistrationId;
                    }

                    ClientEventRegistration::ClientEventRegistration() {}
                }
            }
        }

    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {
                    SmartClientListenerService::SmartClientListenerService(ClientContext &clientContext,
                                                                           int32_t eventThreadCount,
                                                                           int32_t eventQueueCapacity)
                            : AbstractClientListenerService(clientContext, eventThreadCount, eventQueueCapacity) {
                    }


                    void SmartClientListenerService::start() {
                        AbstractClientListenerService::start();

                        registrationExecutor.scheduleAtFixedRate(
                                std::shared_ptr<util::Runnable>(new AsyncConnectToAllMembersTask(
                                        std::static_pointer_cast<SmartClientListenerService>(shared_from_this()))),
                                1000, 1000);
                    }

                    std::string
                    SmartClientListenerService::registerListener(
                            const std::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                            const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/
                        trySyncConnectToAllMembers();

                        return AbstractClientListenerService::registerListener(listenerMessageCodec, handler);
                    }

                    void SmartClientListenerService::trySyncConnectToAllMembers() {
                        ClientClusterService &clientClusterService = clientContext.getClientClusterService();
                        int64_t startMillis = util::currentTimeMillis();

                        do {
                            Member lastFailedMember;
                            std::shared_ptr<exception::IException> lastException;

                            for (const Member &member : clientClusterService.getMemberList()) {
                                try {
                                    clientConnectionManager.getOrConnect(member.getAddress());
                                } catch (exception::IException &e) {
                                    lastFailedMember = member;
                                    lastException = e.clone();
                                }
                            }

                            if (lastException.get() == NULL) {
                                // successfully connected to all members, break loop.
                                break;
                            }

                            timeOutOrSleepBeforeNextTry(startMillis, lastFailedMember, lastException);

                        } while (clientContext.getLifecycleService().isRunning());
                    }

                    void SmartClientListenerService::timeOutOrSleepBeforeNextTry(int64_t startMillis,
                                                                                 const Member &lastFailedMember,
                                                                                 std::shared_ptr<exception::IException> &lastException) {
                        int64_t nowInMillis = util::currentTimeMillis();
                        int64_t elapsedMillis = nowInMillis - startMillis;
                        bool timedOut = elapsedMillis > invocationTimeoutMillis;

                        if (timedOut) {
                            throwOperationTimeoutException(startMillis, nowInMillis, elapsedMillis, lastFailedMember,
                                                           lastException);
                        } else {
                            sleepBeforeNextTry();
                        }

                    }

                    void
                    SmartClientListenerService::throwOperationTimeoutException(int64_t startMillis, int64_t nowInMillis,
                                                                               int64_t elapsedMillis,
                                                                               const Member &lastFailedMember,
                                                                               std::shared_ptr<exception::IException> &lastException) {
                        throw (exception::ExceptionBuilder<exception::OperationTimeoutException>(
                                "SmartClientListenerService::throwOperationTimeoutException")
                                << "Registering listeners is timed out."
                                << " Last failed member : " << lastFailedMember << ", "
                                << " Current time: " << util::StringUtil::timeToString(nowInMillis) << ", "
                                << " Start time : " << util::StringUtil::timeToString(startMillis) << ", "
                                << " Client invocation timeout : " << invocationTimeoutMillis << " ms, "
                                << " Elapsed time : " << elapsedMillis << " ms. " << *lastException).build();

                    }

                    void SmartClientListenerService::sleepBeforeNextTry() {
                        // TODO: change with interruptible sleep
                        util::sleepmillis(invocationRetryPauseMillis);
                    }

                    bool SmartClientListenerService::registersLocalOnly() const {
                        return true;
                    }

                    void SmartClientListenerService::asyncConnectToAllMembersInternal() {
                        std::vector<Member> memberList = clientContext.getClientClusterService().getMemberList();
                        for (const Member &member : memberList) {
                            try {
                                if (!clientContext.getLifecycleService().isRunning()) {
                                    return;
                                }
                                clientContext.getConnectionManager().getOrTriggerConnect(
                                        member.getAddress());
                            } catch (exception::IOException &) {
                                return;
                            }
                        }

                    }

                    SmartClientListenerService::AsyncConnectToAllMembersTask::AsyncConnectToAllMembersTask(
                            const std::shared_ptr<SmartClientListenerService> &listenerService) : listenerService(
                            listenerService) {}

                    void SmartClientListenerService::AsyncConnectToAllMembersTask::run() {
                        listenerService->asyncConnectToAllMembersInternal();
                    }

                    const std::string SmartClientListenerService::AsyncConnectToAllMembersTask::getName() const {
                        return "SmartClientListenerService::AsyncConnectToAllMembersTask";
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
                namespace listener {
                    ClientRegistrationKey::ClientRegistrationKey(const std::string &userRegistrationId,
                                                                 const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler,
                                                                 const std::shared_ptr<ListenerMessageCodec> &codec)
                            : userRegistrationId(userRegistrationId), handler(handler), codec(codec) {
                    }

                    ClientRegistrationKey::ClientRegistrationKey(const std::string &userRegistrationId)
                            : userRegistrationId(userRegistrationId) {
                    }

                    const std::string &ClientRegistrationKey::getUserRegistrationId() const {
                        return userRegistrationId;
                    }

                    const std::shared_ptr<EventHandler<protocol::ClientMessage> > &
                    ClientRegistrationKey::getHandler() const {
                        return handler;
                    }

                    const std::shared_ptr<ListenerMessageCodec> &ClientRegistrationKey::getCodec() const {
                        return codec;
                    }

                    bool ClientRegistrationKey::operator==(const ClientRegistrationKey &rhs) const {
                        return userRegistrationId == rhs.userRegistrationId;
                    }

                    bool ClientRegistrationKey::operator!=(const ClientRegistrationKey &rhs) const {
                        return !(rhs == *this);
                    }

                    bool ClientRegistrationKey::operator<(const ClientRegistrationKey &rhs) const {
                        return userRegistrationId < rhs.userRegistrationId;
                    }

                    std::ostream &operator<<(std::ostream &os, const ClientRegistrationKey &key) {
                        os << "ClientRegistrationKey{ userRegistrationId='" << key.userRegistrationId + '\'' + '}';
                        return os;
                    }

                    ClientRegistrationKey::ClientRegistrationKey() {}
                }
            }
        }

    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {

                    NonSmartClientListenerService::NonSmartClientListenerService(ClientContext &clientContext,
                                                                                 int32_t eventThreadCount,
                                                                                 int32_t eventQueueCapacity)
                            : AbstractClientListenerService(clientContext, eventThreadCount, eventQueueCapacity) {

                    }

                    bool NonSmartClientListenerService::registersLocalOnly() const {
                        return false;
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

                NonSmartClientInvocationService::NonSmartClientInvocationService(ClientContext &client)
                        : AbstractClientInvocationService(client) {

                }

                void NonSmartClientInvocationService::invokeOnConnection(
                        std::shared_ptr<impl::ClientInvocation> invocation,
                        std::shared_ptr<connection::Connection> connection) {
                    assert(connection.get() != NULL);
                    send(invocation, connection);
                }

                void NonSmartClientInvocationService::invokeOnPartitionOwner(
                        std::shared_ptr<impl::ClientInvocation> invocation, int partitionId) {
                    invocation->getClientMessage()->setPartitionId(partitionId);
                    send(invocation, getOwnerConnection());
                }

                void NonSmartClientInvocationService::invokeOnRandomTarget(
                        std::shared_ptr<impl::ClientInvocation> invocation) {
                    send(invocation, getOwnerConnection());
                }

                void
                NonSmartClientInvocationService::invokeOnTarget(std::shared_ptr<impl::ClientInvocation> invocation,
                                                                const std::shared_ptr<Address> &target) {
                    send(invocation, getOwnerConnection());
                }

                std::shared_ptr<connection::Connection> NonSmartClientInvocationService::getOwnerConnection() {
                    std::shared_ptr<Address> ownerConnectionAddress = connectionManager->getOwnerConnectionAddress();
                    if (ownerConnectionAddress.get() == NULL) {
                        throw exception::IOException("NonSmartClientInvocationService::getOwnerConnection",
                                                     "Owner connection address is not available.");
                    }
                    std::shared_ptr<connection::Connection> ownerConnection = connectionManager->getActiveConnection(
                            *ownerConnectionAddress);
                    if (ownerConnection.get() == NULL) {
                        throw exception::IOException("NonSmartClientInvocationService::getOwnerConnection",
                                                     "Owner connection is not available.");
                    }
                    return ownerConnection;
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                DefaultAddressProvider::DefaultAddressProvider(config::ClientNetworkConfig &networkConfig,
                                                               bool noOtherAddressProviderExist) : networkConfig(
                        networkConfig), noOtherAddressProviderExist(noOtherAddressProviderExist) {
                }

                std::vector<Address> DefaultAddressProvider::loadAddresses() {
                    std::vector<Address> addresses = networkConfig.getAddresses();
                    if (addresses.empty() && noOtherAddressProviderExist) {
                        addresses.push_back(Address("127.0.0.1", 5701));
                    }

                    // TODO Implement AddressHelper to add alternative ports for the same address

                    return addresses;
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                impl::ClientClusterServiceImpl::ClientClusterServiceImpl(hazelcast::client::spi::ClientContext &client)
                        : client(client) {
                }

                std::string ClientClusterServiceImpl::addMembershipListenerWithoutInit(
                        const std::shared_ptr<MembershipListener> &listener) {
                    std::string id = util::UuidUtil::newUnsecureUuidString();
                    listeners.put(id, listener);
                    listener->setRegistrationId(id);
                    return id;
                }

                std::shared_ptr<Member> ClientClusterServiceImpl::getMember(const Address &address) {
                    std::map<Address, std::shared_ptr<Member> > currentMembers = members.get();
                    const std::map<hazelcast::client::Address, std::shared_ptr<hazelcast::client::Member> >::iterator &it = currentMembers.find(
                            address);
                    if (it == currentMembers.end()) {
                        return std::shared_ptr<Member>();
                    }
                    return it->second;
                }

                std::shared_ptr<Member> ClientClusterServiceImpl::getMember(const std::string &uuid) {
                    std::vector<Member> memberList = getMemberList();
                    for (const Member &member : memberList) {
                        if (uuid == member.getUuid()) {
                            return std::shared_ptr<Member>(new Member(member));
                        }
                    }
                    return std::shared_ptr<Member>();
                }

                std::vector<Member> ClientClusterServiceImpl::getMemberList() {
                    typedef std::map<Address, std::shared_ptr<Member> > MemberMap;
                    MemberMap memberMap = members.get();
                    std::vector<Member> memberList;
                    for (const MemberMap::value_type &entry : memberMap) {
                        memberList.push_back(*entry.second);
                    }
                    return memberList;
                }

                void ClientClusterServiceImpl::initMembershipListener(MembershipListener &listener) {
                    if (listener.shouldRequestInitialMembers()) {
                        Cluster &cluster = client.getCluster();
                        std::vector<Member> memberCollection = getMemberList();
                        InitialMembershipEvent event(cluster, std::set<Member>(memberCollection.begin(),
                                                                               memberCollection.end()));
                        ((InitialMembershipListener &) listener).init(event);
                    }
                }

                void ClientClusterServiceImpl::start() {
                    clientMembershipListener.reset(new ClientMembershipListener(client));

                    ClientConfig &config = client.getClientConfig();
                    const std::set<std::shared_ptr<MembershipListener> > &membershipListeners = config.getManagedMembershipListeners();

                    for (const std::shared_ptr<MembershipListener> &listener : membershipListeners) {
                        addMembershipListenerWithoutInit(listener);
                    }
                }

                void ClientClusterServiceImpl::handleMembershipEvent(const MembershipEvent &event) {
                    util::LockGuard guard(initialMembershipListenerMutex);
                    const Member &member = event.getMember();
                    std::map<Address, std::shared_ptr<Member> > newMap = members.get();
                    if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                        newMap[member.getAddress()] = std::shared_ptr<Member>(new Member(member));
                    } else {
                        newMap.erase(member.getAddress());
                    }
                    members = newMap;
                    fireMembershipEvent(event);
                }

                void ClientClusterServiceImpl::fireMembershipEvent(const MembershipEvent &event) {
                    for (const std::shared_ptr<MembershipListener> &listener : listeners.values()) {
                        if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                            listener->memberAdded(event);
                        } else {
                            listener->memberRemoved(event);
                        }
                    }
                }

                void ClientClusterServiceImpl::fireMemberAttributeEvent(const MemberAttributeEvent &event) {
                    for (const std::shared_ptr<MembershipListener> &listener : listeners.values()) {
                        listener->memberAttributeChanged(event);
                    }
                }

                void ClientClusterServiceImpl::handleInitialMembershipEvent(const InitialMembershipEvent &event) {
                    util::LockGuard guard(initialMembershipListenerMutex);
                    const std::vector<Member> &initialMembers = event.getMembers();
                    std::map<Address, std::shared_ptr<Member> > newMap;
                    for (const Member &initialMember : initialMembers) {
                        newMap[initialMember.getAddress()] = std::shared_ptr<Member>(
                                new Member(initialMember));
                    }
                    members.set(newMap);
                    fireInitialMembershipEvent(event);

                }

                void ClientClusterServiceImpl::fireInitialMembershipEvent(const InitialMembershipEvent &event) {
                    for (const std::shared_ptr<MembershipListener> &listener : listeners.values()) {
                        if (listener->shouldRequestInitialMembers()) {
                            ((InitialMembershipListener *) listener.get())->init(event);
                        }
                    }
                }

                void ClientClusterServiceImpl::shutdown() {
                }

                void ClientClusterServiceImpl::listenMembershipEvents(
                        const std::shared_ptr<connection::Connection> &ownerConnection) {
                    clientMembershipListener->listenMembershipEvents(clientMembershipListener, ownerConnection);
                }

                std::string
                ClientClusterServiceImpl::addMembershipListener(const std::shared_ptr<MembershipListener> &listener) {
                    if (listener.get() == NULL) {
                        throw exception::NullPointerException("ClientClusterServiceImpl::addMembershipListener",
                                                              "listener can't be null");
                    }

                    util::LockGuard guard(initialMembershipListenerMutex);
                    std::string id = addMembershipListenerWithoutInit(listener);
                    initMembershipListener(*listener);
                    return id;
                }

                bool ClientClusterServiceImpl::removeMembershipListener(const std::string &registrationId) {
                    return listeners.remove(registrationId).get() != NULL;
                }

                std::vector<Member>
                ClientClusterServiceImpl::getMembers(const cluster::memberselector::MemberSelector &selector) {
                    std::vector<Member> result;
                    for (const Member &member : getMemberList()) {
                        if (selector.select(member)) {
                            result.push_back(member);
                        }
                    }

                    return result;
                }

                int ClientClusterServiceImpl::getSize() {
                    return (int) getMemberList().size();
                }

                Client ClientClusterServiceImpl::getLocalClient() const {
                    connection::ClientConnectionManagerImpl &cm = client.getConnectionManager();
                    std::shared_ptr<connection::Connection> connection = cm.getOwnerConnection();
                    std::shared_ptr<Address> inetSocketAddress =
                            connection.get() != NULL ? std::shared_ptr<Address>(connection->getLocalSocketAddress())
                                                     : std::shared_ptr<Address>();
                    const std::shared_ptr<protocol::Principal> principal = cm.getPrincipal();
                    std::shared_ptr<std::string> uuid =
                            principal.get() != NULL ? std::make_shared<std::string>(*principal->getUuid())
                                                    : std::shared_ptr<std::string>();
                    return Client(uuid, inetSocketAddress, client.getName());
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                SmartClientInvocationService::SmartClientInvocationService(ClientContext &client)
                        : AbstractClientInvocationService(client),
                          loadBalancer(*client.getClientConfig().getLoadBalancer()) {}

                void
                SmartClientInvocationService::invokeOnConnection(std::shared_ptr<impl::ClientInvocation> invocation,
                                                                 std::shared_ptr<connection::Connection> connection) {
                    send(invocation, connection);
                }

                void SmartClientInvocationService::invokeOnPartitionOwner(
                        std::shared_ptr<impl::ClientInvocation> invocation, int partitionId) {
                    std::shared_ptr<Address> owner = partitionService.getPartitionOwner(partitionId);
                    if (owner.get() == NULL) {
                        throw (exception::ExceptionBuilder<exception::IOException>(
                                "SmartClientInvocationService::invokeOnPartitionOwner")
                                << "Partition does not have an owner. partitionId: " << partitionId).build();
                    }
                    if (!isMember(*owner)) {
                        throw (exception::ExceptionBuilder<exception::TargetNotMemberException>(
                                "SmartClientInvocationService::invokeOnPartitionOwner") << "Partition owner '" << *owner
                                                                                        << "' is not a member.").build();
                    }
                    invocation->getClientMessage()->setPartitionId(partitionId);
                    std::shared_ptr<connection::Connection> connection = getOrTriggerConnect(owner);
                    send(invocation, connection);
                }

                void SmartClientInvocationService::invokeOnRandomTarget(
                        std::shared_ptr<impl::ClientInvocation> invocation) {
                    std::shared_ptr<Address> randomAddress = getRandomAddress();
                    if (randomAddress.get() == NULL) {
                        throw (exception::ExceptionBuilder<exception::IOException>(
                                "SmartClientInvocationService::invokeOnRandomTarget")
                                << "No address found to invoke").build();
                    }
                    std::shared_ptr<connection::Connection> connection = getOrTriggerConnect(randomAddress);
                    send(invocation, connection);
                }

                void SmartClientInvocationService::invokeOnTarget(std::shared_ptr<impl::ClientInvocation> invocation,
                                                                  const std::shared_ptr<Address> &target) {
                    if (!isMember(*target)) {
                        throw (exception::ExceptionBuilder<exception::TargetNotMemberException>(
                                "SmartClientInvocationService::invokeOnTarget")
                                << "Target '" << *target << "' is not a member.").build();
                    }
                    std::shared_ptr<connection::Connection> connection = getOrTriggerConnect(target);
                    invokeOnConnection(invocation, connection);

                }

                bool SmartClientInvocationService::isMember(const Address &target) const {
                    std::shared_ptr<Member> member = client.getClientClusterService().getMember(target);
                    return member.get() != NULL;
                }

                std::shared_ptr<connection::Connection>
                SmartClientInvocationService::getOrTriggerConnect(const std::shared_ptr<Address> &target) const {
                    std::shared_ptr<connection::Connection> connection = connectionManager->getOrTriggerConnect(
                            *target);
                    if (connection.get() == NULL) {
                        throw (exception::ExceptionBuilder<exception::IOException>(
                                "SmartClientInvocationService::getOrTriggerConnect")
                                << "No available connection to address " << target).build();
                    }
                    return connection;
                }

                std::shared_ptr<Address> SmartClientInvocationService::getRandomAddress() {
                    // TODO: Change loadBalancer to return pointer as in Java so that it is possible to denote the
                    // case where no address can be provided
                    std::shared_ptr<Address> address;
                    try {
                        Member member = loadBalancer.next();
                        address = std::shared_ptr<Address>(new Address(member.getAddress()));
                    } catch (exception::IllegalStateException &) {
                        // do nothing, there is no available server
                    }
                    return address;
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                int ClientMembershipListener::INITIAL_MEMBERS_TIMEOUT_SECONDS = 5;

                ClientMembershipListener::ClientMembershipListener(ClientContext &client)
                        : client(client), logger(client.getLogger()),
                          clusterService(static_cast<ClientClusterServiceImpl &>(client.getClientClusterService())),
                          partitionService((ClientPartitionServiceImpl &) client.getPartitionService()),
                          connectionManager(client.getConnectionManager()) {}

                void ClientMembershipListener::handleMemberEventV10(const Member &member, const int32_t &eventType) {
                    switch (eventType) {
                        case MembershipEvent::MEMBER_ADDED:
                            memberAdded(member);
                            break;
                        case MembershipEvent::MEMBER_REMOVED:
                            memberRemoved(member);
                            break;
                        default:
                            logger.warning("Unknown event type: ", eventType);
                    }
                    partitionService.refreshPartitions();
                }

                void ClientMembershipListener::handleMemberListEventV10(const std::vector<Member> &initialMembers) {
                    std::map<std::string, Member> prevMembers;
                    if (!members.empty()) {
                        for (const Member &member : members) {
                            prevMembers[member.getUuid()] = member;
                        }
                        members.clear();
                    }

                    for (const Member &initialMember : initialMembers) {
                        members.insert(initialMember);
                    }

                    if (prevMembers.empty()) {
                        //this means this is the first time client connected to server
                        logger.info(membersString());
                        clusterService.handleInitialMembershipEvent(
                                InitialMembershipEvent(client.getCluster(), members));
                        initialListFetchedLatch.get()->countDown();
                        return;
                    }

                    std::vector<MembershipEvent> events = detectMembershipEvents(prevMembers);
                    logger.info(membersString());
                    fireMembershipEvent(events);
                    initialListFetchedLatch.get()->countDown();
                }

                void
                ClientMembershipListener::handleMemberAttributeChangeEventV10(const std::string &uuid,
                                                                              const std::string &key,
                                                                              const int32_t &operationType,
                                                                              std::unique_ptr<std::string> &value) {
                    std::vector<Member> members = clusterService.getMemberList();
                    for (Member &target : members) {
                        if (target.getUuid() == uuid) {
                            Member::MemberAttributeOperationType type = (Member::MemberAttributeOperationType) operationType;
                            target.updateAttribute(type, key, value);
                            MemberAttributeEvent memberAttributeEvent(client.getCluster(), target,
                                                                      (MemberAttributeEvent::MemberAttributeOperationType) type,
                                                                      key, value.get() ? (*value) : "");
                            clusterService.fireMemberAttributeEvent(memberAttributeEvent);
                            break;
                        }
                    }

                }

                void ClientMembershipListener::memberAdded(const Member &member) {
                    members.insert(member);
                    logger.info(membersString());
                    MembershipEvent event(client.getCluster(), member, MembershipEvent::MEMBER_ADDED,
                                          std::vector<Member>(members.begin(), members.end()));
                    clusterService.handleMembershipEvent(event);
                }

                std::string ClientMembershipListener::membersString() const {
                    std::stringstream out;
                    out << std::endl << std::endl << "Members [" << members.size() << "]  {";

                    for (const Member &member : members) {
                        out << std::endl << "\t" << member;
                    }
                    out << std::endl << "}" << std::endl;

                    return out.str();
                }

                void ClientMembershipListener::memberRemoved(const Member &member) {
                    members.erase(member);
                    logger.info(membersString());
                    std::shared_ptr<connection::Connection> connection = connectionManager.getActiveConnection(
                            member.getAddress());
                    if (connection.get() != NULL) {
                        connection->close("", newTargetDisconnectedExceptionCausedByMemberLeftEvent(connection));
                    }
                    MembershipEvent event(client.getCluster(), member, MembershipEvent::MEMBER_REMOVED,
                                          std::vector<Member>(members.begin(), members.end()));
                    clusterService.handleMembershipEvent(event);
                }

                std::shared_ptr<exception::IException>
                ClientMembershipListener::newTargetDisconnectedExceptionCausedByMemberLeftEvent(
                        const std::shared_ptr<connection::Connection> &connection) {
                    return (exception::ExceptionBuilder<exception::TargetDisconnectedException>(
                            "ClientMembershipListener::newTargetDisconnectedExceptionCausedByMemberLeftEvent")
                            << "The client has closed the connection to this member, after receiving a member left event from the cluster. "
                            << *connection).buildShared();
                }

                std::vector<MembershipEvent>
                ClientMembershipListener::detectMembershipEvents(std::map<std::string, Member> &prevMembers) {
                    std::vector<MembershipEvent> events;

                    const std::set<Member> &eventMembers = members;

                    std::vector<Member> newMembers;
                    for (const Member &member : members) {
                        std::map<std::string, Member>::iterator formerEntry = prevMembers.find(
                                member.getUuid());
                        if (formerEntry != prevMembers.end()) {
                            prevMembers.erase(formerEntry);
                        } else {
                            newMembers.push_back(member);
                        }
                    }

                    // removal events should be added before added events
                    typedef const std::map<std::string, Member> MemberMap;
                    for (const MemberMap::value_type &member : prevMembers) {
                        events.push_back(MembershipEvent(client.getCluster(), member.second,
                                                         MembershipEvent::MEMBER_REMOVED,
                                                         std::vector<Member>(eventMembers.begin(),
                                                                             eventMembers.end())));
                        const Address &address = member.second.getAddress();
                        if (clusterService.getMember(address).get() == NULL) {
                            std::shared_ptr<connection::Connection> connection = connectionManager.getActiveConnection(
                                    address);
                            if (connection.get() != NULL) {
                                connection->close("",
                                                  newTargetDisconnectedExceptionCausedByMemberLeftEvent(
                                                          connection));
                            }
                        }
                    }
                    for (const Member &member : newMembers) {
                        events.push_back(
                                MembershipEvent(client.getCluster(), member, MembershipEvent::MEMBER_ADDED,
                                                std::vector<Member>(eventMembers.begin(),
                                                                    eventMembers.end())));
                    }

                    return events;
                }

                void ClientMembershipListener::fireMembershipEvent(std::vector<MembershipEvent> &events) {
                    for (const MembershipEvent &event : events) {
                        clusterService.handleMembershipEvent(event);
                    }
                }

                void
                ClientMembershipListener::listenMembershipEvents(
                        const std::shared_ptr<ClientMembershipListener> &listener,
                        const std::shared_ptr<connection::Connection> &ownerConnection) {
                    listener->initialListFetchedLatch = std::shared_ptr<util::CountDownLatch>(
                            new util::CountDownLatch(1));
                    std::unique_ptr<protocol::ClientMessage> clientMessage = protocol::codec::ClientAddMembershipListenerCodec::encodeRequest(
                            false);
                    std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(listener->client,
                                                                                            clientMessage, "",
                                                                                            ownerConnection);
                    invocation->setEventHandler(listener);
                    invocation->invokeUrgent()->get();
                    listener->waitInitialMemberListFetched();
                }

                void ClientMembershipListener::waitInitialMemberListFetched() {
                    bool success = initialListFetchedLatch.get()->await(INITIAL_MEMBERS_TIMEOUT_SECONDS);
                    if (!success) {
                        logger.warning("Error while getting initial member list from cluster!");
                    }
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif