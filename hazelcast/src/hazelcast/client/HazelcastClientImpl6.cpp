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
        namespace connection {
            AuthenticationFuture::AuthenticationFuture(const Address &address,
                                                       util::SynchronizedMap<Address, FutureTuple> &connectionsInProgress)
                    : countDownLatch(new util::CountDownLatch(1)), address(address),
                      connectionsInProgress(connectionsInProgress), isSet(false) {
            }

            void AuthenticationFuture::onSuccess(const std::shared_ptr<Connection> &connection) {
                bool expected = false;
                if (!isSet.compare_exchange_strong(expected, true)) {
                    return;
                }
                this->connection = connection;
                countDownLatch->countDown();
            }

            void AuthenticationFuture::onFailure(const std::shared_ptr<exception::IException> &throwable) {
                bool expected = false;
                if (!isSet.compare_exchange_strong(expected, true)) {
                    return;
                }
                connectionsInProgress.remove(address);
                this->throwable = throwable;
                countDownLatch->countDown();
            }

            std::shared_ptr<Connection> AuthenticationFuture::get() {
                countDownLatch->await();
                auto connPtr = connection.get();
                if (connPtr.get() != NULL) {
                    return connPtr;
                }

                auto exceptionPtr = throwable.get();
                assert(exceptionPtr.get() != NULL);
                throw exception::ExecutionException("AuthenticationFuture::get", "Could not be authenticated.",
                                                    exceptionPtr);
            }

        }
    }
}




//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {
            ReadHandler::ReadHandler(Connection &connection, size_t bufferSize)
                    : buffer(new char[bufferSize]), byteBuffer(buffer, bufferSize), builder(connection) {
                lastReadTimeMillis = util::currentTimeMillis();
            }

            ReadHandler::~ReadHandler() {
                delete[] buffer;
            }

            void ReadHandler::handle() {
                lastReadTimeMillis = util::currentTimeMillis();

                if (byteBuffer.position() == 0)
                    return;

                byteBuffer.flip();

                // it is important to check the onData return value since there may be left data less than a message
                // header size, and this may cause an infinite loop.
                while (byteBuffer.hasRemaining() && builder.onData(byteBuffer)) {
                }

                if (byteBuffer.hasRemaining()) {
                    byteBuffer.compact();
                } else {
                    byteBuffer.clear();
                }
            }

            int64_t ReadHandler::getLastReadTimeMillis() {
                return lastReadTimeMillis;
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address &address, spi::ClientContext &clientContext, int connectionId,
                                   const std::shared_ptr<AuthenticationFuture> &authFuture,
                                   internal::socket::SocketFactory &socketFactory, boost::asio::io_context &ioContext,
                                   bool asOwner,
                                   ClientConnectionManagerImpl &clientConnectionManager, int64_t connectTimeoutInMillis)
                    : readHandler(*this, 16 << 10),
                      startTimeInMillis(util::currentTimeMillis()), closedTimeMillis(0),
                      clientContext(clientContext),
                      invocationService(clientContext.getInvocationService()),
                      authFuture(authFuture),
                      connectionId(connectionId),
                      connectedServerVersion(impl::BuildInfo::UNKNOWN_HAZELCAST_VERSION),
                      logger(clientContext.getLogger()), asOwner(asOwner),
                      connectionManager(clientConnectionManager) {
                socket = socketFactory.create(address, connectTimeoutInMillis);
            }

            Connection::~Connection() {
            }

            void Connection::authenticate() {
                auto thisConnection = shared_from_this();
                connectionManager.authenticate(socket->getRemoteEndpoint(), thisConnection, asOwner, authFuture);
            }

            void Connection::reAuthenticateAsOwner() {
                asOwner = true;
                auto thisConnection = shared_from_this();
                connectionManager.reAuthenticate(socket->getRemoteEndpoint(), thisConnection, asOwner, authFuture);
                authFuture->get();
            }

            void Connection::asyncStart() {
                socket->asyncStart(shared_from_this(), authFuture);
            }

            void Connection::close() {
                close("");
            }

            void Connection::close(const std::string &reason) {
                close(reason, std::shared_ptr<exception::IException>());
            }

            void Connection::close(const std::string &reason, const std::shared_ptr<exception::IException> &cause) {
                int64_t expected = 0;
                if (!closedTimeMillis.compare_exchange_strong(expected, util::currentTimeMillis())) {
                    return;
                }

                closeCause = cause;
                closeReason = reason;

                logClose();

                try {
                    innerClose();
                } catch (exception::IException &e) {
                    clientContext.getLogger().warning("Exception while closing connection", e.getMessage());
                }

                clientContext.getConnectionManager().onClose(*this);
            }

            bool Connection::write(const std::shared_ptr<protocol::ClientMessage> &message) {
                socket->asyncWrite(shared_from_this(), message);
                return true;
            }

            const std::shared_ptr<Address> &Connection::getRemoteEndpoint() const {
                return remoteEndpoint;
            }

            void Connection::setRemoteEndpoint(const std::shared_ptr<Address> &remoteEndpoint) {
                this->remoteEndpoint = remoteEndpoint;
            }

            void Connection::handleClientMessage(const std::shared_ptr<protocol::ClientMessage> &message) {
                if (message->isFlagSet(protocol::ClientMessage::LISTENER_EVENT_FLAG)) {
                    spi::impl::listener::AbstractClientListenerService &listenerService =
                            (spi::impl::listener::AbstractClientListenerService &) clientContext.getClientListenerService();
                    listenerService.handleClientMessage(message, shared_from_this());
                } else {
                    invocationService.handleClientMessage(shared_from_this(), message);
                }
            }

            int Connection::getConnectionId() const {
                return connectionId;
            }

            bool Connection::isAlive() {
                return closedTimeMillis == 0;
            }

            const std::string &Connection::getCloseReason() const {
                return closeReason;
            }

            void Connection::logClose() {
                std::ostringstream message;
                message << *this << " closed. Reason: ";
                if (!closeReason.empty()) {
                    message << closeReason;
                } else if (closeCause.get() != NULL) {
                    message << closeCause->getSource() << "[" + closeCause->getMessage() << "]";
                } else {
                    message << "Socket explicitly closed";
                }

                util::ILogger &logger = clientContext.getLogger();
                if (clientContext.getLifecycleService().isRunning()) {
                    if (!closeCause.get()) {
                        logger.info(message.str());
                    } else {
                        logger.warning(message.str(), *closeCause);
                    }
                } else {
                    if (closeCause.get() == NULL) {
                        logger.finest(message.str());
                    } else {
                        logger.finest(message.str(), *closeCause);
                    }
                }
            }

            bool Connection::isAuthenticatedAsOwner() {
                return authenticatedAsOwner;
            }

            void Connection::setIsAuthenticatedAsOwner() {
                authenticatedAsOwner.store(true);
            }

            bool Connection::operator==(const Connection &rhs) const {
                return connectionId == rhs.connectionId;
            }

            bool Connection::operator!=(const Connection &rhs) const {
                return !(rhs == *this);
            }

            const std::string &Connection::getConnectedServerVersionString() const {
                return connectedServerVersionString;
            }

            void Connection::setConnectedServerVersion(const std::string &connectedServerVersionString) {
                Connection::connectedServerVersionString = connectedServerVersionString;
                connectedServerVersion = impl::BuildInfo::calculateVersion(connectedServerVersionString);
            }

            int Connection::getConnectedServerVersion() const {
                return connectedServerVersion;
            }

            std::unique_ptr<Address> Connection::getLocalSocketAddress() const {
                return socket->localSocketAddress();
            }

            int64_t Connection::lastReadTimeMillis() {
                return readHandler.getLastReadTimeMillis();
            }

            void Connection::innerClose() {
                if (!socket.get()) {
                    return;;
                }

                socket->close();
            }

            std::ostream &operator<<(std::ostream &os, const Connection &connection) {
                Connection &conn = const_cast<Connection &>(connection);
                int64_t lastRead = conn.lastReadTimeMillis();
                int64_t closedTime = conn.closedTimeMillis;
                os << "ClientConnection{"
                   << "alive=" << conn.isAlive()
                   << ", connectionId=" << connection.getConnectionId()
                   << ", remoteEndpoint=";
                if (connection.getRemoteEndpoint().get()) {
                    os << *connection.getRemoteEndpoint();
                } else {
                    os << "null";
                }
                os << ", lastReadTime=" << util::StringUtil::timeToStringFriendly(lastRead)
                   << ", closedTime=" << util::StringUtil::timeToStringFriendly(closedTime)
                   << ", connected server version=" << conn.connectedServerVersionString
                   << '}';

                return os;
            }

            bool Connection::operator<(const Connection &rhs) const {
                return connectionId < rhs.connectionId;
            }

            int64_t Connection::getStartTimeInMillis() const {
                return startTimeInMillis;
            }

            const Socket &Connection::getSocket() const {
                return *socket;
            }
        }
    }
}





//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {

            ClientConnectionStrategy::ClientConnectionStrategy(spi::ClientContext &clientContext, util::ILogger &logger,
                                                               const config::ClientConnectionStrategyConfig &clientConnectionStrategyConfig)
                    : clientContext(clientContext), logger(logger),
                      clientConnectionStrategyConfig(clientConnectionStrategyConfig) {
            }

            ClientConnectionStrategy::~ClientConnectionStrategy() {
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace connection {

            HeartbeatManager::HeartbeatManager(spi::ClientContext &client) : client(client), clientConnectionManager(
                    client.getConnectionManager()), logger(client.getLogger()) {
                ClientProperties &clientProperties = client.getClientProperties();
                int timeoutSeconds = clientProperties.getInteger(clientProperties.getHeartbeatTimeout());
                heartbeatTimeout = timeoutSeconds > 0 ? timeoutSeconds * 1000 : util::IOUtil::to_value<int>(
                        (std::string) ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT) * 1000;

                int intervalSeconds = clientProperties.getInteger(clientProperties.getHeartbeatInterval());
                heartbeatInterval = intervalSeconds > 0 ? intervalSeconds * 1000 : util::IOUtil::to_value<int>(
                        (std::string) ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT) * 1000;
            }

            void HeartbeatManager::start() {
                spi::impl::ClientExecutionServiceImpl &clientExecutionService = client.getClientExecutionService();

                clientExecutionService.scheduleWithRepetition(
                        std::shared_ptr<util::Runnable>(new util::RunnableDelegator(*this)), heartbeatInterval,
                        heartbeatInterval);
            }

            void HeartbeatManager::run() {
                if (!clientConnectionManager.isAlive()) {
                    return;
                }

                int64_t now = util::currentTimeMillis();
                for (std::shared_ptr<Connection> connection : clientConnectionManager.getActiveConnections()) {
                    checkConnection(now, connection);
                }
            }

            const std::string HeartbeatManager::getName() const {
                return "HeartbeatManager";
            }

            void HeartbeatManager::checkConnection(int64_t now, std::shared_ptr<Connection> &connection) {
                if (!connection->isAlive()) {
                    return;
                }

                if (now - connection->lastReadTimeMillis() > heartbeatTimeout) {
                    if (connection->isAlive()) {
                        logger.warning("Heartbeat failed over the connection: ", *connection);
                        onHeartbeatStopped(connection, "Heartbeat timed out");
                    }
                }

                if (now - connection->lastReadTimeMillis() > heartbeatInterval) {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ClientPingCodec::encodeRequest();
                    std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client, request, "", connection);
                    clientInvocation->invokeUrgent();
                }
            }

            void
            HeartbeatManager::onHeartbeatStopped(std::shared_ptr<Connection> &connection, const std::string &reason) {
                connection->close(reason.c_str(), (exception::ExceptionBuilder<exception::TargetDisconnectedException>(
                        "HeartbeatManager::onHeartbeatStopped") << "Heartbeat timed out to connection "
                                                                << *connection).buildShared());
            }

            void HeartbeatManager::shutdown() {
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        Member::Member() : liteMember(false) {
        }

        Member::Member(const Address &address, const std::string &uuid, bool lite,
                       const std::map<std::string, std::string> &attr) :
                address(address), uuid(uuid), liteMember(lite), attributes(attr) {
        }

        Member::Member(const Address &memberAddress) : address(memberAddress), liteMember(false) {
        }

        Member::Member(const std::string &uuid) : uuid(uuid), liteMember(false) {
        }

        bool Member::operator==(const Member &rhs) const {
            return address == rhs.address;
        }

        const Address &Member::getAddress() const {
            return address;
        }

        const std::string &Member::getUuid() const {
            return uuid;
        }

        bool Member::isLiteMember() const {
            return liteMember;
        }

        const std::map<std::string, std::string> &Member::getAttributes() const {
            return attributes;
        }

        std::ostream &operator<<(std::ostream &out, const Member &member) {
            const Address &address = member.getAddress();
            out << "Member[";
            out << address.getHost();
            out << "]";
            out << ":";
            out << address.getPort();
            out << " - " << member.getUuid();
            return out;
        }

        const std::string *Member::getAttribute(const std::string &key) const {
            std::map<std::string, std::string>::const_iterator it = attributes.find(key);
            if (attributes.end() != it) {
                return &(it->second);
            } else {
                return NULL;
            }
        }

        bool Member::lookupAttribute(const std::string &key) const {
            return attributes.find(key) != attributes.end();
        }

        bool Member::operator<(const Member &rhs) const {
            return uuid < rhs.uuid;
        }

        void Member::updateAttribute(Member::MemberAttributeOperationType operationType, const std::string &key,
                                     std::unique_ptr<std::string> &value) {
            switch (operationType) {
                case PUT:
                    attributes[key] = *value;
                    break;
                case REMOVE:
                    attributes.erase(key);
                    break;
                default:
                    throw (exception::ExceptionBuilder<exception::IllegalArgumentException>("Member::updateAttribute")
                            << "Not a known OperationType: " << operationType).build();
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace connection {

            DefaultClientConnectionStrategy::DefaultClientConnectionStrategy(spi::ClientContext &clientContext,
                                                                             util::ILogger &logger,
                                                                             const config::ClientConnectionStrategyConfig &clientConnectionStrategyConfig)
                    : ClientConnectionStrategy(clientContext, logger, clientConnectionStrategyConfig),
                      isShutdown(false) {
            }

            void DefaultClientConnectionStrategy::start() {
                clientStartAsync = clientConnectionStrategyConfig.isAsyncStart();
                reconnectMode = clientConnectionStrategyConfig.getReconnectMode();
                if (clientStartAsync) {
                    clientContext.getConnectionManager().connectToClusterAsync();
                } else {
                    clientContext.getConnectionManager().connectToCluster();
                }
            }

            void DefaultClientConnectionStrategy::beforeGetConnection(const Address &target) {
                checkShutdown("DefaultClientConnectionStrategy::beforeGetConnection");

                if (isClusterAvailable()) {
                    return;
                }
                if (clientStartAsync && !disconnectedFromCluster) {
                    throw exception::HazelcastClientOfflineException(
                            "DefaultClientConnectionStrategy::beforeGetConnection", "Client is connecting to cluster.");
                }
                if (reconnectMode == config::ClientConnectionStrategyConfig::ASYNC && disconnectedFromCluster) {
                    throw exception::HazelcastClientOfflineException(
                            "DefaultClientConnectionStrategy::beforeGetConnection", "Client is offline.");
                }
            }

            void DefaultClientConnectionStrategy::beforeOpenConnection(const Address &target) {
                checkShutdown("DefaultClientConnectionStrategy::beforeOpenConnection");

                if (isClusterAvailable()) {
                    return;
                }
                if (reconnectMode == config::ClientConnectionStrategyConfig::ASYNC && disconnectedFromCluster) {
                    throw exception::HazelcastClientOfflineException(
                            "DefaultClientConnectionStrategy::beforeGetConnection", "Client is offline");
                }
            }

            void DefaultClientConnectionStrategy::onConnectToCluster() {
                checkShutdown("DefaultClientConnectionStrategy::onConnectToCluster");

                disconnectedFromCluster.store(false);
            }

            void DefaultClientConnectionStrategy::onDisconnectFromCluster() {
                checkShutdown("DefaultClientConnectionStrategy::onDisconnectFromCluster");

                disconnectedFromCluster.store(true);
                if (reconnectMode == config::ClientConnectionStrategyConfig::OFF) {
                    shutdownWithExternalThread(clientContext.getHazelcastClientImplementation());
                    return;
                }
                if (clientContext.getLifecycleService().isRunning()) {
                    try {
                        clientContext.getConnectionManager().connectToClusterAsync();
                    } catch (exception::RejectedExecutionException &) {
                        shutdownWithExternalThread(clientContext.getHazelcastClientImplementation());
                    }
                }
            }

            void DefaultClientConnectionStrategy::onConnect(const std::shared_ptr<Connection> &connection) {
                checkShutdown("DefaultClientConnectionStrategy::onConnect");
            }

            void DefaultClientConnectionStrategy::onDisconnect(const std::shared_ptr<Connection> &connection) {
                checkShutdown("DefaultClientConnectionStrategy::onDisconnect");
            }

            void DefaultClientConnectionStrategy::shutdown() {
                isShutdown = true;
            }

            bool DefaultClientConnectionStrategy::isClusterAvailable() const {
                return clientContext.getConnectionManager().getOwnerConnectionAddress().get() != NULL;
            }

            void
            DefaultClientConnectionStrategy::shutdownWithExternalThread(
                    std::weak_ptr<client::impl::HazelcastClientInstanceImpl> clientImpl) {

                std::thread shutdownThread([=] {
                    std::shared_ptr<client::impl::HazelcastClientInstanceImpl> clientInstance = clientImpl.lock();
                    if (!clientInstance.get() || !clientInstance->getLifecycleService().isRunning()) {
                        return;
                    }

                    try {
                        clientInstance->getLifecycleService().shutdown();
                    } catch (exception::IException &exception) {
                        clientInstance->getLogger()->severe("Exception during client shutdown task ",
                                                            clientInstance->getName() + ".clientShutdown-", ":",
                                                            exception);
                    }
                });

                shutdownThread.detach();
            }

            void DefaultClientConnectionStrategy::checkShutdown(const std::string &methodName) {
                if (isShutdown) {
                    throw exception::IllegalStateException(methodName, "Client is shutdown.");
                }
            }
        }
    }
}


// Includes for parameters classes


namespace hazelcast {
    namespace client {

        ICountDownLatch::ICountDownLatch(const std::string &objectName, spi::ClientContext *context)
                : proxy::ProxyImpl("hz:impl:atomicLongService", objectName, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&objectName);
            partitionId = getPartitionId(keyData);
        }

        bool ICountDownLatch::await(long timeoutInMillis) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::CountDownLatchAwaitCodec::encodeRequest(getName(), timeoutInMillis);

            return invokeAndGetResult<bool, protocol::codec::CountDownLatchAwaitCodec::ResponseParameters>(request,
                                                                                                           partitionId);
        }

        void ICountDownLatch::countDown() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::CountDownLatchCountDownCodec::encodeRequest(getName());

            invokeOnPartition(request, partitionId);
        }

        int ICountDownLatch::getCount() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::CountDownLatchGetCountCodec::encodeRequest(getName());

            return invokeAndGetResult<int, protocol::codec::CountDownLatchGetCountCodec::ResponseParameters>(request,
                                                                                                             partitionId);
        }

        bool ICountDownLatch::trySetCount(int count) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::CountDownLatchTrySetCountCodec::encodeRequest(getName(), count);

            return invokeAndGetResult<bool, protocol::codec::CountDownLatchTrySetCountCodec::ResponseParameters>(
                    request, partitionId);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
