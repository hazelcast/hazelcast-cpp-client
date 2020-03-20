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
        namespace map {
            namespace impl {
                namespace nearcache {
                    bool TrueMarkerImpl::tryMark(const serialization::pimpl::Data &key) {
                        return true;
                    }

                    bool TrueMarkerImpl::tryUnmark(const serialization::pimpl::Data &key) {
                        return true;
                    }

                    bool TrueMarkerImpl::tryRemove(const serialization::pimpl::Data &key) {
                        return true;
                    }

                    void TrueMarkerImpl::forceUnmark(const serialization::pimpl::Data &key) {
                    }

                    void TrueMarkerImpl::init() {
                    }

                    const std::unique_ptr<KeyStateMarker> KeyStateMarker::TRUE_MARKER =
                            std::unique_ptr<KeyStateMarker>(new TrueMarkerImpl());
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace map {
            DataEntryView::DataEntryView(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                         int64_t cost,
                                         int64_t creationTime, int64_t expirationTime, int64_t hits,
                                         int64_t lastAccessTime,
                                         int64_t lastStoredTime, int64_t lastUpdateTime, int64_t version,
                                         int64_t evictionCriteriaNumber,
                                         int64_t ttl) : key(key), value(value), cost(cost), creationTime(creationTime),
                                                        expirationTime(expirationTime), hits(hits),
                                                        lastAccessTime(lastAccessTime),
                                                        lastStoredTime(lastStoredTime), lastUpdateTime(lastUpdateTime),
                                                        version(version),
                                                        evictionCriteriaNumber(evictionCriteriaNumber), ttl(ttl) {}


            const serialization::pimpl::Data &DataEntryView::getKey() const {
                return key;
            }

            const serialization::pimpl::Data &DataEntryView::getValue() const {
                return value;
            }

            int64_t DataEntryView::getCost() const {
                return cost;
            }

            int64_t DataEntryView::getCreationTime() const {
                return creationTime;
            }

            int64_t DataEntryView::getExpirationTime() const {
                return expirationTime;
            }

            int64_t DataEntryView::getHits() const {
                return hits;
            }

            int64_t DataEntryView::getLastAccessTime() const {
                return lastAccessTime;
            }

            int64_t DataEntryView::getLastStoredTime() const {
                return lastStoredTime;
            }

            int64_t DataEntryView::getLastUpdateTime() const {
                return lastUpdateTime;
            }

            int64_t DataEntryView::getVersion() const {
                return version;
            }

            int64_t DataEntryView::getEvictionCriteriaNumber() const {
                return evictionCriteriaNumber;
            }

            int64_t DataEntryView::getTtl() const {
                return ttl;
            }

        }
    }
}


namespace hazelcast {
    namespace client {

        int64_t IAtomicLong::addAndGet(int64_t delta) {
            return impl->addAndGet(delta);
        }

        bool IAtomicLong::compareAndSet(int64_t expect, int64_t update) {
            return impl->compareAndSet(expect, update);
        }

        int64_t IAtomicLong::decrementAndGet() {
            return impl->decrementAndGet();
        }

        int64_t IAtomicLong::get() {
            return impl->get();
        }

        int64_t IAtomicLong::getAndAdd(int64_t delta) {
            return impl->getAndAdd(delta);
        }

        int64_t IAtomicLong::getAndSet(int64_t newValue) {
            return impl->getAndSet(newValue);
        }

        int64_t IAtomicLong::incrementAndGet() {
            return impl->incrementAndGet();
        }

        int64_t IAtomicLong::getAndIncrement() {
            return impl->getAndIncrement();
        }

        void IAtomicLong::set(int64_t newValue) {
            impl->set(newValue);
        }

        IAtomicLong::IAtomicLong(const std::shared_ptr<impl::AtomicLongInterface> &impl) : impl(impl) {}

        const std::string &IAtomicLong::getServiceName() const {
            return impl->getServiceName();
        }

        const std::string &IAtomicLong::getName() const {
            return impl->getName();
        }

        void IAtomicLong::destroy() {
            impl->destroy();
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::addAndGetAsync(int64_t delta) {
            return impl->addAndGetAsync(delta);
        }

        std::shared_ptr<ICompletableFuture<bool> > IAtomicLong::compareAndSetAsync(int64_t expect, int64_t update) {
            return impl->compareAndSetAsync(expect, update);
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::decrementAndGetAsync() {
            return impl->decrementAndGetAsync();
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::getAsync() {
            return impl->getAsync();
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::getAndAddAsync(int64_t delta) {
            return impl->getAndAddAsync(delta);
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::getAndSetAsync(int64_t newValue) {
            return impl->getAndSetAsync(newValue);
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::incrementAndGetAsync() {
            return impl->incrementAndGetAsync();
        }

        std::shared_ptr<ICompletableFuture<int64_t> > IAtomicLong::getAndIncrementAsync() {
            return impl->getAndIncrementAsync();
        }

        std::shared_ptr<ICompletableFuture<void> > IAtomicLong::setAsync(int64_t newValue) {
            return impl->setAsync(newValue);
        }
    }
}


namespace hazelcast {
    namespace client {
        const std::string IExecutorService::SERVICE_NAME = "hz:impl:executorService";

        IExecutorService::IExecutorService(const std::string &name, spi::ClientContext *context) : ProxyImpl(
                SERVICE_NAME, name, context), consecutiveSubmits(0), lastSubmitTime(0) {
        }

        std::vector<Member>
        IExecutorService::selectMembers(const cluster::memberselector::MemberSelector &memberSelector) {
            std::vector<Member> selected;
            std::vector<Member> members = getContext().getClientClusterService().getMemberList();
            for (const Member &member : members) {
                if (memberSelector.select(member)) {
                    selected.push_back(member);
                }
            }
            if (selected.empty()) {
                throw (exception::ExceptionBuilder<exception::RejectedExecutionException>(
                        "IExecutorService::selectMembers") << "No member selected with memberSelector["
                                                           << memberSelector << "]").build();
            }
            return selected;
        }

        std::shared_ptr<spi::impl::ClientInvocationFuture>
        IExecutorService::invokeOnTarget(std::unique_ptr<protocol::ClientMessage> &request, const Address &target) {
            try {
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), target);
                return clientInvocation->invoke();
            } catch (exception::IException &e) {
                util::ExceptionUtil::rethrow(e);
            }
            return std::shared_ptr<spi::impl::ClientInvocationFuture>();
        }

        std::shared_ptr<spi::impl::ClientInvocationFuture>
        IExecutorService::invokeOnPartitionOwner(std::unique_ptr<protocol::ClientMessage> &request, int partitionId) {
            try {
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), partitionId);
                return clientInvocation->invoke();
            } catch (exception::IException &e) {
                util::ExceptionUtil::rethrow(e);
            }
            return std::shared_ptr<spi::impl::ClientInvocationFuture>();
        }

        bool IExecutorService::isSyncComputation(bool preventSync) {
            int64_t now = util::currentTimeMillis();

            int64_t last = lastSubmitTime;
            lastSubmitTime = now;

            if (last + MIN_TIME_RESOLUTION_OF_CONSECUTIVE_SUBMITS < now) {
                consecutiveSubmits = 0;
                return false;
            }

            return !preventSync && (consecutiveSubmits++ % MAX_CONSECUTIVE_SUBMITS == 0);
        }

        Address IExecutorService::getMemberAddress(const Member &member) {
            std::shared_ptr<Member> m = getContext().getClientClusterService().getMember(member.getUuid());
            if (m.get() == NULL) {
                throw (exception::ExceptionBuilder<exception::HazelcastException>(
                        "IExecutorService::getMemberAddress(Member)") << member << " is not available!").build();
            }
            return m->getAddress();
        }

        int IExecutorService::randomPartitionId() {
            spi::ClientPartitionService &partitionService = getContext().getPartitionService();
            return rand() % partitionService.getPartitionCount();
        }

        void IExecutorService::shutdown() {
            std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ExecutorServiceShutdownCodec::encodeRequest(
                    getName());
            invoke(request);
        }

        bool IExecutorService::isShutdown() {
            std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ExecutorServiceIsShutdownCodec::encodeRequest(
                    getName());
            return invokeAndGetResult<bool, protocol::codec::ExecutorServiceIsShutdownCodec::ResponseParameters>(
                    request);
        }

        bool IExecutorService::isTerminated() {
            return isShutdown();
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace atomiclong {
            namespace impl {
                AtomicLongProxyFactory::AtomicLongProxyFactory(spi::ClientContext *clientContext) : clientContext(
                        clientContext) {}

                std::shared_ptr<spi::ClientProxy> AtomicLongProxyFactory::create(const std::string &id) {
                    return std::shared_ptr<spi::ClientProxy>(new proxy::ClientAtomicLongProxy(id, clientContext));
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        ILock::ILock(const std::string &instanceName, spi::ClientContext *context)
                : proxy::ProxyImpl("hz:impl:lockService", instanceName, context),
                  key(toData<std::string>(instanceName)) {
            partitionId = getPartitionId(key);

            // TODO: remove this line once the client instance getDistributedObject works as expected in Java for this proxy type
            referenceIdGenerator = context->getLockReferenceIdGenerator();
        }

        void ILock::lock() {
            lock(-1);
        }

        void ILock::lock(long leaseTimeInMillis) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockLockCodec::encodeRequest(getName(), leaseTimeInMillis,
                                                                  util::getCurrentThreadId(),
                                                                  referenceIdGenerator->getNextReferenceId());

            invokeOnPartition(request, partitionId);
        }

        void ILock::unlock() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockUnlockCodec::encodeRequest(getName(), util::getCurrentThreadId(),
                                                                    referenceIdGenerator->getNextReferenceId());

            invokeOnPartition(request, partitionId);
        }

        void ILock::forceUnlock() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockForceUnlockCodec::encodeRequest(getName(),
                                                                         referenceIdGenerator->getNextReferenceId());

            invokeOnPartition(request, partitionId);
        }

        bool ILock::isLocked() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockIsLockedCodec::encodeRequest(getName());

            return invokeAndGetResult<bool, protocol::codec::LockIsLockedCodec::ResponseParameters>(request,
                                                                                                    partitionId);
        }

        bool ILock::isLockedByCurrentThread() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockIsLockedByCurrentThreadCodec::encodeRequest(getName(),
                                                                                     util::getCurrentThreadId());

            return invokeAndGetResult<bool, protocol::codec::LockIsLockedByCurrentThreadCodec::ResponseParameters>(
                    request, partitionId);
        }

        int ILock::getLockCount() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockGetLockCountCodec::encodeRequest(getName());

            return invokeAndGetResult<int, protocol::codec::LockGetLockCountCodec::ResponseParameters>(request,
                                                                                                       partitionId);
        }

        long ILock::getRemainingLeaseTime() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockGetRemainingLeaseTimeCodec::encodeRequest(getName());

            return invokeAndGetResult<long, protocol::codec::LockGetRemainingLeaseTimeCodec::ResponseParameters>(
                    request, partitionId);
        }

        bool ILock::tryLock() {
            return tryLock(0);
        }

        bool ILock::tryLock(long timeInMillis) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::LockTryLockCodec::encodeRequest(getName(), util::getCurrentThreadId(), LONG_MAX,
                                                                     timeInMillis,
                                                                     referenceIdGenerator->getNextReferenceId());

            return invokeAndGetResult<bool, protocol::codec::LockTryLockCodec::ResponseParameters>(request,
                                                                                                   partitionId);
        }

        void ILock::onInitialize() {
            ProxyImpl::onInitialize();

            referenceIdGenerator = getContext().getLockReferenceIdGenerator();
        }
    }
}

namespace hazelcast {
    namespace client {
        FlakeIdGenerator::FlakeIdGenerator(const std::shared_ptr<impl::IdGeneratorInterface> &impl) : IdGenerator(
                impl) {}

        int64_t FlakeIdGenerator::newId() {
            return IdGenerator::newId();
        }

        bool FlakeIdGenerator::init(int64_t id) {
            return IdGenerator::init(id);
        }
    }
}


namespace hazelcast {
    namespace client {

        SerializationConfig::SerializationConfig() : version(0) {
        }

        int SerializationConfig::getPortableVersion() const {
            return version;
        }

        SerializationConfig &SerializationConfig::setPortableVersion(int version) {
            this->version = version;
            return *this;
        }

        std::vector<std::shared_ptr<serialization::SerializerBase> > const &
        SerializationConfig::getSerializers() const {
            return serializers;
        }

        SerializationConfig &
        SerializationConfig::registerSerializer(std::shared_ptr<serialization::SerializerBase> serializer) {
            serializers.push_back(serializer);
            return *this;
        }

        SerializationConfig &
        SerializationConfig::registerSerializer(std::shared_ptr<serialization::StreamSerializer> serializer) {
            serializers.push_back(serializer);
            return *this;
        }

        SerializationConfig &SerializationConfig::addDataSerializableFactory(int32_t factoryId,
                                                                             std::shared_ptr<serialization::DataSerializableFactory> dataSerializableFactory) {
            dataSerializableFactories[factoryId] = dataSerializableFactory;
            return *this;
        }

        SerializationConfig &SerializationConfig::addPortableFactory(int32_t factoryId,
                                                                     std::shared_ptr<serialization::PortableFactory> portableFactory) {
            portableFactories[factoryId] = portableFactory;
            return *this;
        }

        const std::map<int32_t, std::shared_ptr<serialization::DataSerializableFactory> > &
        SerializationConfig::getDataSerializableFactories() const {
            return dataSerializableFactories;
        }

        const std::map<int32_t, std::shared_ptr<serialization::PortableFactory> > &
        SerializationConfig::getPortableFactories() const {
            return portableFactories;
        }

        SerializationConfig &
        SerializationConfig::setGlobalSerializer(const std::shared_ptr<serialization::StreamSerializer> &serializer) {
            globalSerializer = serializer;
            return *this;
        }

        const std::shared_ptr<serialization::StreamSerializer> &SerializationConfig::getGlobalSerializer() const {
            return globalSerializer;
        }
    }
}


// Includes for parameters classes

namespace hazelcast {
    namespace client {

        ISemaphore::ISemaphore(const std::string &name, spi::ClientContext *context)
                : proxy::ProxyImpl("hz:impl:semaphoreService", name, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&name);
            partitionId = getPartitionId(keyData);
        }

        bool ISemaphore::init(int permits) {
            checkNegative(permits);
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreInitCodec::encodeRequest(getName(), permits);

            return invokeAndGetResult<bool, protocol::codec::SemaphoreInitCodec::ResponseParameters>(request,
                                                                                                     partitionId);
        }

        void ISemaphore::acquire() {
            acquire(1);
        }

        void ISemaphore::acquire(int permits) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreAcquireCodec::encodeRequest(getName(), permits);

            invokeOnPartition(request, partitionId);
        }

        int ISemaphore::availablePermits() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreAvailablePermitsCodec::encodeRequest(getName());

            return invokeAndGetResult<int, protocol::codec::SemaphoreAvailablePermitsCodec::ResponseParameters>(request,
                                                                                                                partitionId);
        }

        int ISemaphore::drainPermits() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreDrainPermitsCodec::encodeRequest(getName());

            return invokeAndGetResult<int, protocol::codec::SemaphoreDrainPermitsCodec::ResponseParameters>(request,
                                                                                                            partitionId);
        }

        void ISemaphore::reducePermits(int reduction) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreReducePermitsCodec::encodeRequest(getName(), reduction);

            invokeOnPartition(request, partitionId);
        }

        void ISemaphore::release() {
            release(1);
        }

        void ISemaphore::release(int permits) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreReleaseCodec::encodeRequest(getName(), permits);

            invokeOnPartition(request, partitionId);
        }

        bool ISemaphore::tryAcquire() {
            return tryAcquire(int(1));
        }

        bool ISemaphore::tryAcquire(int permits) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreTryAcquireCodec::encodeRequest(getName(), permits, 0);

            return invokeAndGetResult<bool, protocol::codec::SemaphoreTryAcquireCodec::ResponseParameters>(request,
                                                                                                           partitionId);
        }

        bool ISemaphore::tryAcquire(long timeoutInMillis) {
            return tryAcquire(1, timeoutInMillis);
        }

        bool ISemaphore::tryAcquire(int permits, long timeoutInMillis) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreTryAcquireCodec::encodeRequest(getName(), permits, timeoutInMillis);

            return invokeAndGetResult<bool, protocol::codec::SemaphoreTryAcquireCodec::ResponseParameters>(request,
                                                                                                           partitionId);
        }

        void ISemaphore::increasePermits(int32_t increase) {
            checkNegative(increase);
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreIncreasePermitsCodec::encodeRequest(getName(), increase);
            invokeOnPartition(request, partitionId);
        }

        void ISemaphore::checkNegative(int32_t permits) const {
            if (permits < 0) {
                throw exception::IllegalArgumentException("ISemaphore::checkNegative", "Permits cannot be negative!");
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        InitialMembershipListener::~InitialMembershipListener() {
        }

        bool InitialMembershipListener::shouldRequestInitialMembers() const {
            return true;
        }

        void InitialMembershipListenerDelegator::init(
                const InitialMembershipEvent &event) {
            listener->init(event);
        }

        void InitialMembershipListenerDelegator::memberRemoved(
                const MembershipEvent &membershipEvent) {
            listener->memberRemoved(membershipEvent);
        }

        void InitialMembershipListenerDelegator::memberAdded(
                const MembershipEvent &membershipEvent) {
            listener->memberAdded(membershipEvent);
        }

        void InitialMembershipListenerDelegator::memberAttributeChanged(
                const MemberAttributeEvent &memberAttributeEvent) {
            listener->memberAttributeChanged(memberAttributeEvent);
        }

        InitialMembershipListenerDelegator::InitialMembershipListenerDelegator(
                InitialMembershipListener *listener) : listener(listener) {}

        bool InitialMembershipListenerDelegator::shouldRequestInitialMembers() const {
            return listener->shouldRequestInitialMembers();
        }

        const std::string &InitialMembershipListenerDelegator::getRegistrationId() const {
            return listener->getRegistrationId();
        }

        void InitialMembershipListenerDelegator::setRegistrationId(const std::string &registrationId) {
            listener->setRegistrationId(registrationId);
        }

    }
}

namespace hazelcast {
    namespace client {
        namespace executor {
            namespace impl {
                ExecutorServiceProxyFactory::ExecutorServiceProxyFactory(spi::ClientContext *clientContext)
                        : clientContext(
                        clientContext) {}

                std::shared_ptr<spi::ClientProxy> ExecutorServiceProxyFactory::create(const std::string &id) {
                    return std::shared_ptr<spi::ClientProxy>(
                            new IExecutorService(id, clientContext));
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace impl {
                Filter::Filter() {
                }

                /**
                 *
                 * Add a new filter with the given name and value to the query.
                 *
                 * @param name Filter name
                 * @param value Filter value
                 *
                 */
                void Filter::addFilter(const std::string &name, const std::string &value) {
                    std::stringstream out;
                    unsigned long index = filters.size() + 1;
                    out << "Filter." << index << ".Name";
                    filters[out.str()] = name;
                    out.str("");
                    out.clear();
                    out << "Filter." << index << ".Value.1";
                    filters[out.str()] = value;
                }

                const std::map<std::string, std::string> &Filter::getFilters() {
                    return filters;
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace impl {
                AwsAddressTranslator::AwsAddressTranslator(config::ClientAwsConfig &awsConfig, util::ILogger &logger)
                        : logger(logger) {
                    if (awsConfig.isEnabled() && !awsConfig.isInsideAws()) {
                        awsClient = std::unique_ptr<AWSClient>(new AWSClient(awsConfig, logger));
                    }
                }

                Address AwsAddressTranslator::translate(const Address &address) {
                    // if no translation is needed just return the address as it is
                    if (NULL == awsClient.get()) {
                        return address;
                    }

                    Address translatedAddress = address;

                    if (findFromCache(address, translatedAddress)) {
                        return translatedAddress;
                    }

                    refresh();

                    if (findFromCache(address, translatedAddress)) {
                        return translatedAddress;
                    }

                    std::stringstream out;
                    out << "No translation is found for private ip:" << address;
                    throw exception::IOException("AwsAddressTranslator::translate", out.str());
                }

                void AwsAddressTranslator::refresh() {
                    try {
                        privateToPublic = std::shared_ptr<std::map<std::string, std::string> >(
                                new std::map<std::string, std::string>(awsClient->getAddresses()));
                    } catch (exception::IException &e) {
                        logger.warning(std::string("AWS addresses failed to load: ") + e.what());
                    }
                }

                bool AwsAddressTranslator::findFromCache(const Address &address, Address &translatedAddress) {
                    std::shared_ptr<std::map<std::string, std::string> > mapping = privateToPublic;
                    if (mapping.get() == NULL) {
                        return false;
                    }

                    std::map<std::string, std::string>::const_iterator publicAddressIt = mapping->find(
                            address.getHost());
                    if (publicAddressIt != mapping->end()) {
                        const std::string &publicIp = (*publicAddressIt).second;
                        if (!publicIp.empty()) {
                            translatedAddress = Address((*publicAddressIt).second, address.getPort());
                            return true;
                        }
                    }

                    return false;
                }
            }
        }
    }
}


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace impl {
                const std::string DescribeInstances::QUERY_PREFIX = "/?";
                const std::string DescribeInstances::IAM_ROLE_ENDPOINT = "169.254.169.254";
                const std::string DescribeInstances::IAM_ROLE_QUERY = "/latest/meta-data/iam/security-credentials/";
                const std::string DescribeInstances::IAM_TASK_ROLE_ENDPOINT = "169.254.170.2";

                DescribeInstances::DescribeInstances(config::ClientAwsConfig &awsConfig, const std::string &endpoint,
                                                     util::ILogger &logger) : awsConfig(awsConfig), endpoint(endpoint),
                                                                              logger(logger) {
                    checkKeysFromIamRoles();

                    std::string timeStamp = getFormattedTimestamp();
                    rs = std::unique_ptr<security::EC2RequestSigner>(
                            new security::EC2RequestSigner(awsConfig, timeStamp, endpoint));
                    attributes["Action"] = "DescribeInstances";
                    attributes["Version"] = impl::Constants::DOC_VERSION;
                    attributes["X-Amz-Algorithm"] = impl::Constants::SIGNATURE_METHOD_V4;
                    attributes["X-Amz-Credential"] = rs->createFormattedCredential();
                    attributes["X-Amz-Date"] = timeStamp;
                    attributes["X-Amz-SignedHeaders"] = "host";
                    attributes["X-Amz-Expires"] = "30";
                    addFilters();
                }

                DescribeInstances::~DescribeInstances() {
                }

                std::map<std::string, std::string> DescribeInstances::execute() {
                    std::string signature = rs->sign(attributes);
                    attributes["X-Amz-Signature"] = signature;

                    std::istream &stream = callService();
                    return utility::CloudUtility::unmarshalTheResponse(stream, logger);
                }

                std::string DescribeInstances::getFormattedTimestamp() {
                    using namespace boost::posix_time;
                    ptime now = second_clock::universal_time();

                    std::ostringstream out;
                    std::locale timeLocale(out.getloc(), new time_facet(impl::Constants::DATE_FORMAT));
                    out.imbue(timeLocale);
                    out << now;
                    return out.str();
                }

                std::istream &DescribeInstances::callService() {
                    std::string query = rs->getCanonicalizedQueryString(attributes);
                    httpsClient = std::unique_ptr<util::SyncHttpsClient>(
                            new util::SyncHttpsClient(endpoint.c_str(), QUERY_PREFIX + query));
                    return httpsClient->openConnection();
                }

                void DescribeInstances::checkKeysFromIamRoles() {
                    if (awsConfig.getAccessKey().empty() || !awsConfig.getIamRole().empty()) {
                        tryGetDefaultIamRole();
                        if (!awsConfig.getIamRole().empty()) {
                            getKeysFromIamRole();
                        } else {
                            getKeysFromIamTaskRole();
                        }
                    }
                }

                void DescribeInstances::tryGetDefaultIamRole() {
                    // if none of the below are true
                    if (!(awsConfig.getIamRole().empty() || awsConfig.getIamRole() == "DEFAULT")) {
                        // stop here. No point looking up the default role.
                        return;
                    }
                    try {
                        util::SyncHttpClient httpClient(IAM_ROLE_ENDPOINT, IAM_ROLE_QUERY);
                        std::string roleName;
                        std::istream &responseStream = httpClient.openConnection();
                        responseStream >> roleName;
                        awsConfig.setIamRole(roleName);
                    } catch (exception::IOException &e) {
                        throw exception::InvalidConfigurationException("tryGetDefaultIamRole",
                                                                       std::string("Invalid Aws Configuration. ") +
                                                                       e.what());
                    }
                }

                void DescribeInstances::getKeysFromIamTaskRole() {
                    // before giving up, attempt to discover whether we're running in an ECS Container,
                    // in which case, AWS_CONTAINER_CREDENTIALS_RELATIVE_URI will exist as an env var.
                    const char *uri = getenv(Constants::ECS_CREDENTIALS_ENV_VAR_NAME);
                    if (!uri) {
                        throw exception::IllegalArgumentException("getKeysFromIamTaskRole",
                                                                  "Could not acquire credentials! Did not find declared AWS access key or IAM Role, and could not discover IAM Task Role or default role.");
                    }

                    util::SyncHttpClient httpClient(IAM_TASK_ROLE_ENDPOINT, uri);

                    try {
                        std::istream &istream = httpClient.openConnection();
                        parseAndStoreRoleCreds(istream);
                    } catch (exception::IException &e) {
                        std::stringstream out;
                        out << "Unable to retrieve credentials from IAM Task Role. URI: " << uri << ". \n " << e.what();
                        throw exception::InvalidConfigurationException("getKeysFromIamTaskRole", out.str());
                    }
                }

                void DescribeInstances::getKeysFromIamRole() {
                    std::string query = "/latest/meta-data/iam/security-credentials/" + awsConfig.getIamRole();

                    util::SyncHttpClient httpClient(IAM_ROLE_ENDPOINT, query);

                    try {
                        std::istream &istream = httpClient.openConnection();
                        parseAndStoreRoleCreds(istream);
                    } catch (exception::IException &e) {
                        std::stringstream out;
                        out << "Unable to retrieve credentials from IAM Task Role. URI: " << query << ". \n "
                            << e.what();
                        throw exception::InvalidConfigurationException("getKeysFromIamRole", out.str());
                    }
                }

                void DescribeInstances::parseAndStoreRoleCreds(std::istream &in) {
                    utility::CloudUtility::unmarshalJsonResponse(in, awsConfig, attributes);
                }

                /**
                 * Add available filters to narrow down the scope of the query
                 */
                void DescribeInstances::addFilters() {
                    Filter filter;
                    if (!awsConfig.getTagKey().empty()) {
                        if (!awsConfig.getTagValue().empty()) {
                            filter.addFilter(std::string("tag:") + awsConfig.getTagKey(), awsConfig.getTagValue());
                        } else {
                            filter.addFilter("tag-key", awsConfig.getTagKey());
                        }
                    } else if (!awsConfig.getTagValue().empty()) {
                        filter.addFilter("tag-value", awsConfig.getTagValue());
                    }

                    if (!awsConfig.getSecurityGroupName().empty()) {
                        filter.addFilter("instance.group-name", awsConfig.getSecurityGroupName());
                    }

                    filter.addFilter("instance-state-name", "running");
                    const std::map<std::string, std::string> &filters = filter.getFilters();
                    attributes.insert(filters.begin(), filters.end());
                }
            }
        }
    }
}


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
