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
        namespace impl {
            AbstractLoadBalancer::AbstractLoadBalancer(const AbstractLoadBalancer &rhs) {
                *this = rhs;
            }

            void AbstractLoadBalancer::operator=(const AbstractLoadBalancer &rhs) {
                util::LockGuard lg(const_cast<util::Mutex &>(rhs.membersLock));
                util::LockGuard lg2(membersLock);
                membersRef = rhs.membersRef;
                cluster = rhs.cluster;
            }

            void AbstractLoadBalancer::init(Cluster &cluster) {
                this->cluster = &cluster;
                setMembersRef();
                cluster.addMembershipListener(this);
            }

            void AbstractLoadBalancer::setMembersRef() {
                util::LockGuard lg(membersLock);
                membersRef = cluster->getMembers();
            }

            void AbstractLoadBalancer::memberAdded(const MembershipEvent &membershipEvent) {
                setMembersRef();
            }

            void AbstractLoadBalancer::memberRemoved(const MembershipEvent &membershipEvent) {
                setMembersRef();
            }

            void AbstractLoadBalancer::memberAttributeChanged(const MemberAttributeEvent &memberAttributeEvent) {
            }

            std::vector<Member> AbstractLoadBalancer::getMembers() {
                util::LockGuard lg(membersLock);
                return membersRef;
            }

            AbstractLoadBalancer::~AbstractLoadBalancer() {
            }

            AbstractLoadBalancer::AbstractLoadBalancer() : cluster(NULL) {
            }

            void AbstractLoadBalancer::init(const InitialMembershipEvent &event) {
                setMembersRef();
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace cluster {
            namespace impl {

                VectorClock::VectorClock() {}

                VectorClock::VectorClock(const VectorClock::TimestampVector &replicaLogicalTimestamps)
                        : replicaTimestampEntries(replicaLogicalTimestamps) {
                    for (const VectorClock::TimestampVector::value_type &replicaTimestamp : replicaLogicalTimestamps) {
                        replicaTimestamps[replicaTimestamp.first] = replicaTimestamp.second;
                    }
                }

                VectorClock::TimestampVector VectorClock::entrySet() {
                    return replicaTimestampEntries;
                }

                bool VectorClock::isAfter(VectorClock &other) {
                    bool anyTimestampGreater = false;
                    for (const VectorClock::TimestampMap::value_type &otherEntry : other.replicaTimestamps) {
                        const std::string &replicaId = otherEntry.first;
                        int64_t otherReplicaTimestamp = otherEntry.second;
                        std::pair<bool, int64_t> localReplicaTimestamp = getTimestampForReplica(replicaId);

                        if (!localReplicaTimestamp.first ||
                            localReplicaTimestamp.second < otherReplicaTimestamp) {
                            return false;
                        } else if (localReplicaTimestamp.second > otherReplicaTimestamp) {
                            anyTimestampGreater = true;
                        }
                    }
                    // there is at least one local timestamp greater or local vector clock has additional timestamps
                    return anyTimestampGreater || other.replicaTimestamps.size() < replicaTimestamps.size();
                }

                std::pair<bool, int64_t> VectorClock::getTimestampForReplica(const std::string &replicaId) {
                    if (replicaTimestamps.count(replicaId) == 0) {
                        return std::make_pair(false, -1);
                    }
                    return std::make_pair(true, replicaTimestamps[replicaId]);
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace cluster {
            namespace memberselector {
                bool MemberSelectors::DataMemberSelector::select(const Member &member) const {
                    return !member.isLiteMember();
                }

                void MemberSelectors::DataMemberSelector::toString(std::ostream &os) const {
                    os << "Default DataMemberSelector";
                }

                const std::unique_ptr<MemberSelector> MemberSelectors::DATA_MEMBER_SELECTOR(
                        new MemberSelectors::DataMemberSelector());
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        Cluster::Cluster(spi::ClientClusterService &clusterService)
                : clusterService(clusterService) {
        }

        void Cluster::addMembershipListener(MembershipListener *listener) {
            clusterService.addMembershipListener(
                    std::shared_ptr<MembershipListener>(new MembershipListenerDelegator(listener)));
        }

        bool Cluster::removeMembershipListener(MembershipListener *listener) {
            return clusterService.removeMembershipListener(listener->getRegistrationId());
        }

        std::vector<Member> Cluster::getMembers() {
            return clusterService.getMemberList();
        }

        std::string Cluster::addMembershipListener(const std::shared_ptr<MembershipListener> &listener) {
            return clusterService.addMembershipListener(listener);
        }

        bool Cluster::removeMembershipListener(const std::string &registrationId) {
            return clusterService.removeMembershipListener(registrationId);
        }

        std::string Cluster::addMembershipListener(const std::shared_ptr<InitialMembershipListener> &listener) {
            return clusterService.addMembershipListener(listener);
        }

        std::string Cluster::addMembershipListener(InitialMembershipListener *listener) {
            return clusterService.addMembershipListener(
                    std::shared_ptr<MembershipListener>(new InitialMembershipListenerDelegator(listener)));

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace crdt {
            namespace pncounter {
                namespace impl {
                    PNCounterProxyFactory::PNCounterProxyFactory(spi::ClientContext *clientContext) : clientContext(
                            clientContext) {}

                    std::shared_ptr<spi::ClientProxy> PNCounterProxyFactory::create(const std::string &id) {
                        return std::shared_ptr<spi::ClientProxy>(
                                new proxy::ClientPNCounterProxy(proxy::ClientPNCounterProxy::SERVICE_NAME, id,
                                                                clientContext));
                    }
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        EntryEventType::EntryEventType() : value(UNDEFINED) {

        }

        EntryEventType::EntryEventType(Type value)
                : value(value) {

        }

        EntryEventType::operator int() const {
            return value;
        }

        void EntryEventType::operator=(int i) {
            value = (EntryEventType::Type) i;
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace monitor {
            namespace impl {
                LocalMapStatsImpl::LocalMapStatsImpl() : nearCacheStats(NULL) {}

                NearCacheStats *LocalMapStatsImpl::getNearCacheStats() {
                    return nearCacheStats;
                }

                void LocalMapStatsImpl::setNearCacheStats(NearCacheStats &stats) {
                    this->nearCacheStats = &stats;
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace monitor {
            namespace impl {
                NearCacheStatsImpl::NearCacheStatsImpl() : creationTime(util::currentTimeMillis()),
                                                           ownedEntryCount(0),
                                                           ownedEntryMemoryCost(0),
                                                           hits(0),
                                                           misses(0),
                                                           evictions(0),
                                                           expirations(0),
                                                           invalidations(0),
                                                           invalidationRequests(0),
                                                           persistenceCount(0),
                                                           lastPersistenceTime(0),
                                                           lastPersistenceDuration(0),
                                                           lastPersistenceWrittenBytes(0),
                                                           lastPersistenceKeyCount(0),
                                                           lastPersistenceFailure("") {
                }

                int64_t NearCacheStatsImpl::getCreationTime() {
                    return creationTime;
                }

                int64_t NearCacheStatsImpl::getOwnedEntryCount() {
                    return ownedEntryCount;
                }

                void NearCacheStatsImpl::setOwnedEntryCount(int64_t ownedEntryCount) {
                    this->ownedEntryCount = ownedEntryCount;
                }

                void NearCacheStatsImpl::incrementOwnedEntryCount() {
                    ++ownedEntryCount;
                }

                void NearCacheStatsImpl::decrementOwnedEntryCount() {
                    --ownedEntryCount;
                }

                int64_t NearCacheStatsImpl::getOwnedEntryMemoryCost() {
                    return ownedEntryMemoryCost;
                }

                void NearCacheStatsImpl::setOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost) {
                    this->ownedEntryMemoryCost = ownedEntryMemoryCost;
                }

                void NearCacheStatsImpl::incrementOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost) {
                    this->ownedEntryMemoryCost += ownedEntryMemoryCost;
                }

                void NearCacheStatsImpl::decrementOwnedEntryMemoryCost(int64_t ownedEntryMemoryCost) {
                    this->ownedEntryMemoryCost -= ownedEntryMemoryCost;
                }

                int64_t NearCacheStatsImpl::getHits() {
                    return hits;
                }

                // just for testing
                void NearCacheStatsImpl::setHits(int64_t hits) {
                    this->hits = hits;
                }

                void NearCacheStatsImpl::incrementHits() {
                    ++hits;
                }

                int64_t NearCacheStatsImpl::getMisses() {
                    return misses;
                }

                // just for testing
                void NearCacheStatsImpl::setMisses(int64_t misses) {
                    this->misses = misses;
                }

                void NearCacheStatsImpl::incrementMisses() {
                    ++misses;
                }

                double NearCacheStatsImpl::getRatio() {
                    if (misses == (int64_t) 0) {
                        if (hits == (int64_t) 0) {
                            return std::numeric_limits<double>::signaling_NaN();
                        } else {
                            return std::numeric_limits<double>::infinity();
                        }
                    } else {
                        return ((double) hits / misses) * PERCENTAGE;
                    }
                }

                int64_t NearCacheStatsImpl::getEvictions() {
                    return evictions;
                }

                void NearCacheStatsImpl::incrementEvictions() {
                    ++evictions;
                }

                int64_t NearCacheStatsImpl::getExpirations() {
                    return expirations;
                }

                void NearCacheStatsImpl::incrementExpirations() {
                    ++expirations;
                }

                int64_t NearCacheStatsImpl::getInvalidations() {
                    return invalidations.load();
                }

                void NearCacheStatsImpl::incrementInvalidations() {
                    ++invalidations;
                }

                int64_t NearCacheStatsImpl::getInvalidationRequests() {
                    return invalidationRequests.load();
                }

                void NearCacheStatsImpl::incrementInvalidationRequests() {
                    ++invalidationRequests;
                }

                void NearCacheStatsImpl::resetInvalidationEvents() {
                    invalidationRequests = 0;
                }

                int64_t NearCacheStatsImpl::getPersistenceCount() {
                    return persistenceCount;
                }

                void NearCacheStatsImpl::addPersistence(int64_t duration, int32_t writtenBytes, int32_t keyCount) {
                    ++persistenceCount;
                    lastPersistenceTime = util::currentTimeMillis();
                    lastPersistenceDuration = duration;
                    lastPersistenceWrittenBytes = writtenBytes;
                    lastPersistenceKeyCount = keyCount;
                    lastPersistenceFailure = "";
                }

                int64_t NearCacheStatsImpl::getLastPersistenceTime() {
                    return lastPersistenceTime;
                }

                int64_t NearCacheStatsImpl::getLastPersistenceDuration() {
                    return lastPersistenceDuration;
                }

                int64_t NearCacheStatsImpl::getLastPersistenceWrittenBytes() {
                    return lastPersistenceWrittenBytes;
                }

                int64_t NearCacheStatsImpl::getLastPersistenceKeyCount() {
                    return lastPersistenceKeyCount;
                }

                std::string NearCacheStatsImpl::getLastPersistenceFailure() {
                    return lastPersistenceFailure;
                }

                std::string NearCacheStatsImpl::toString() {
                    std::ostringstream out;
                    std::string failureString = lastPersistenceFailure;
                    out << "NearCacheStatsImpl{"
                        << "ownedEntryCount=" << ownedEntryCount
                        << ", ownedEntryMemoryCost=" << ownedEntryMemoryCost
                        << ", creationTime=" << creationTime
                        << ", hits=" << hits
                        << ", misses=" << misses
                        << ", ratio=" << std::setprecision(1) << getRatio()
                        << ", evictions=" << evictions
                        << ", expirations=" << expirations
                        << ", invalidations=" << invalidations.load()
                        << ", invalidationRequests=" << invalidationRequests.load()
                        << ", lastPersistenceTime=" << lastPersistenceTime
                        << ", persistenceCount=" << persistenceCount
                        << ", lastPersistenceDuration=" << lastPersistenceDuration
                        << ", lastPersistenceWrittenBytes=" << lastPersistenceWrittenBytes
                        << ", lastPersistenceKeyCount=" << lastPersistenceKeyCount
                        << ", lastPersistenceFailure='" << failureString << "'"
                        << '}';

                    return out.str();
                }

                const double NearCacheStatsImpl::PERCENTAGE = 100.0;
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace monitor {
            const int64_t LocalInstanceStats::STAT_NOT_AVAILABLE = -99L;
        }
    }
}




namespace hazelcast {
    namespace client {
        HazelcastClient::HazelcastClient() : clientImpl(new impl::HazelcastClientInstanceImpl(ClientConfig())) {
            clientImpl->start();
        }

        HazelcastClient::HazelcastClient(const ClientConfig &config) : clientImpl(
                new impl::HazelcastClientInstanceImpl(config)) {
            clientImpl->start();
        }

        const std::string &HazelcastClient::getName() const {
            return clientImpl->getName();
        }

        IdGenerator HazelcastClient::getIdGenerator(const std::string &name) {
            return clientImpl->getIdGenerator(name);
        }

        FlakeIdGenerator HazelcastClient::getFlakeIdGenerator(const std::string &name) {
            return clientImpl->getFlakeIdGenerator(name);
        }

        IAtomicLong HazelcastClient::getIAtomicLong(const std::string &name) {
            return clientImpl->getIAtomicLong(name);
        }

        std::shared_ptr<crdt::pncounter::PNCounter> HazelcastClient::getPNCounter(const std::string &name) {
            return clientImpl->getPNCounter(name);
        }

        ICountDownLatch HazelcastClient::getICountDownLatch(const std::string &name) {
            return clientImpl->getICountDownLatch(name);
        }

        ILock HazelcastClient::getILock(const std::string &name) {
            return clientImpl->getILock(name);
        }

        ISemaphore HazelcastClient::getISemaphore(const std::string &name) {
            return clientImpl->getISemaphore(name);
        }

        ClientConfig &HazelcastClient::getClientConfig() {
            return clientImpl->getClientConfig();
        }

        TransactionContext HazelcastClient::newTransactionContext() {
            return clientImpl->newTransactionContext();
        }

        TransactionContext HazelcastClient::newTransactionContext(const TransactionOptions &options) {
            return clientImpl->newTransactionContext(options);
        }

        Cluster &HazelcastClient::getCluster() {
            return clientImpl->getCluster();
        }

        void HazelcastClient::addLifecycleListener(LifecycleListener *lifecycleListener) {
            clientImpl->addLifecycleListener(lifecycleListener);
        }

        bool HazelcastClient::removeLifecycleListener(LifecycleListener *lifecycleListener) {
            return clientImpl->removeLifecycleListener(lifecycleListener);
        }

        void HazelcastClient::shutdown() {
            clientImpl->shutdown();
        }

        mixedtype::HazelcastClient &HazelcastClient::toMixedType() const {
            return clientImpl->toMixedType();
        }

        spi::LifecycleService &HazelcastClient::getLifecycleService() {
            return clientImpl->getLifecycleService();
        }

        std::shared_ptr<IExecutorService> HazelcastClient::getExecutorService(const std::string &name) {
            return clientImpl->getExecutorService(name);
        }

        Client HazelcastClient::getLocalEndpoint() const {
            return clientImpl->getLocalEndpoint();
        }

        HazelcastClient::~HazelcastClient() {
            clientImpl->shutdown();
        }
    }
}



namespace hazelcast {
    namespace client {
        namespace flakeidgen {
            namespace impl {
                FlakeIdGeneratorProxyFactory::FlakeIdGeneratorProxyFactory(spi::ClientContext *clientContext)
                        : clientContext(
                        clientContext) {}

                std::shared_ptr<spi::ClientProxy> FlakeIdGeneratorProxyFactory::create(const std::string &id) {
                    return std::shared_ptr<spi::ClientProxy>(
                            new proxy::ClientFlakeIdGeneratorProxy(id, clientContext));
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace flakeidgen {
            namespace impl {

                AutoBatcher::AutoBatcher(int32_t batchSize, int64_t validity,
                                         const std::shared_ptr<AutoBatcher::IdBatchSupplier> &batchIdSupplier)
                        : batchSize(batchSize), validity(validity), batchIdSupplier(batchIdSupplier),
                          block(std::shared_ptr<Block>(new Block(IdBatch(0, 0, 0), 0))) {}

                int64_t AutoBatcher::newId() {
                    for (;;) {
                        std::shared_ptr<Block> block = this->block;
                        int64_t res = block->next();
                        if (res != INT64_MIN) {
                            return res;
                        }

                        {
                            std::lock_guard<std::mutex> guard(lock);
                            if (block != this->block.get()) {
                                // new block was assigned in the meantime
                                continue;
                            }
                            this->block = std::shared_ptr<Block>(
                                    new Block(batchIdSupplier->newIdBatch(batchSize), validity));
                        }
                    }
                }

                AutoBatcher::Block::Block(const IdBatch &idBatch, int64_t validity) : idBatch(idBatch), numReturned(0) {
                    invalidSince = validity > 0 ? util::currentTimeMillis() + validity : INT64_MAX;
                }

                int64_t AutoBatcher::Block::next() {
                    if (invalidSince <= util::currentTimeMillis()) {
                        return INT64_MIN;
                    }
                    int32_t index;
                    do {
                        index = numReturned;
                        if (index == idBatch.getBatchSize()) {
                            return INT64_MIN;
                        }
                    } while (!numReturned.compare_exchange_strong(index, index + 1));

                    return idBatch.getBase() + index * idBatch.getIncrement();
                }
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace flakeidgen {
            namespace impl {
                IdBatch::IdIterator IdBatch::endOfBatch;

                const int64_t IdBatch::getBase() const {
                    return base;
                }

                const int64_t IdBatch::getIncrement() const {
                    return increment;
                }

                const int32_t IdBatch::getBatchSize() const {
                    return batchSize;
                }

                IdBatch::IdBatch(const int64_t base, const int64_t increment, const int32_t batchSize)
                        : base(base), increment(increment), batchSize(batchSize) {}

                IdBatch::IdIterator &IdBatch::end() {
                    return endOfBatch;
                }

                IdBatch::IdIterator IdBatch::iterator() {
                    return IdBatch::IdIterator(base, increment, batchSize);
                }

                IdBatch::IdIterator::IdIterator(int64_t base2, const int64_t increment, int32_t remaining) : base2(
                        base2), increment(increment), remaining(remaining) {}

                bool IdBatch::IdIterator::operator==(const IdBatch::IdIterator &rhs) const {
                    return base2 == rhs.base2 && increment == rhs.increment && remaining == rhs.remaining;
                }

                bool IdBatch::IdIterator::operator!=(const IdBatch::IdIterator &rhs) const {
                    return !(rhs == *this);
                }

                IdBatch::IdIterator::IdIterator() : base2(-1), increment(-1), remaining(-1) {
                }

                IdBatch::IdIterator &IdBatch::IdIterator::operator++() {
                    if (remaining == 0) {
                        return IdBatch::end();
                    }

                    --remaining;

                    base2 += increment;

                    return *this;
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace txn {
            const std::shared_ptr<util::ExceptionUtil::RuntimeExceptionFactory> ClientTransactionUtil::exceptionFactory(
                    new TransactionExceptionFactory());

            std::shared_ptr<protocol::ClientMessage>
            ClientTransactionUtil::invoke(std::unique_ptr<protocol::ClientMessage> &request,
                                          const std::string &objectName,
                                          spi::ClientContext &client,
                                          const std::shared_ptr<connection::Connection> &connection) {
                try {
                    std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client, request, objectName, connection);
                    std::shared_ptr<spi::impl::ClientInvocationFuture> future = clientInvocation->invoke();
                    return future->get();
                } catch (exception::IException &e) {
                    TRANSACTION_EXCEPTION_FACTORY()->rethrow(e, "ClientTransactionUtil::invoke failed");
                }
                return std::shared_ptr<protocol::ClientMessage>();
            }

            const std::shared_ptr<util::ExceptionUtil::RuntimeExceptionFactory> &
            ClientTransactionUtil::TRANSACTION_EXCEPTION_FACTORY() {
                return exceptionFactory;
            }

            void
            ClientTransactionUtil::TransactionExceptionFactory::rethrow(const client::exception::IException &throwable,
                                                                        const std::string &message) {
                throw TransactionException("TransactionExceptionFactory::create", message,
                                           std::shared_ptr<IException>(throwable.clone()));
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace txn {
#define MILLISECOND_IN_A_SECOND 1000

            TransactionProxy::TransactionProxy(TransactionOptions &txnOptions, spi::ClientContext &clientContext,
                                               std::shared_ptr<connection::Connection> connection)
                    : options(txnOptions), clientContext(clientContext), connection(connection),
                      threadId(util::getCurrentThreadId()), state(TxnState::NO_TXN), startTime(0) {
            }


            TransactionProxy::TransactionProxy(const TransactionProxy &rhs) : options(rhs.options),
                                                                              clientContext(rhs.clientContext),
                                                                              connection(rhs.connection),
                                                                              threadId(rhs.threadId), txnId(rhs.txnId),
                                                                              state(rhs.state),
                                                                              startTime(rhs.startTime) {
                TransactionProxy &nonConstRhs = const_cast<TransactionProxy &>(rhs);

                TRANSACTION_EXISTS.store(nonConstRhs.TRANSACTION_EXISTS.load());
            }

            const std::string &TransactionProxy::getTxnId() const {
                return txnId;
            }

            TxnState TransactionProxy::getState() const {
                return state;
            }

            int TransactionProxy::getTimeoutSeconds() const {
                return options.getTimeout();
            }


            void TransactionProxy::begin() {
                try {
                    if (clientContext.getConnectionManager().getOwnerConnection().get() == NULL) {
                        throw exception::TransactionException("TransactionProxy::begin()",
                                                              "Owner connection needs to be present to begin a transaction");
                    }
                    if (state == TxnState::ACTIVE) {
                        throw exception::IllegalStateException("TransactionProxy::begin()",
                                                               "Transaction is already active");
                    }
                    checkThread();
                    if (TRANSACTION_EXISTS) {
                        throw exception::IllegalStateException("TransactionProxy::begin()",
                                                               "Nested transactions are not allowed!");
                    }
                    TRANSACTION_EXISTS.store(true);
                    startTime = util::currentTimeMillis();
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::TransactionCreateCodec::encodeRequest(
                            options.getTimeout() * MILLISECOND_IN_A_SECOND, options.getDurability(),
                            options.getTransactionType(), threadId);

                    std::shared_ptr<protocol::ClientMessage> response = invoke(request);

                    protocol::codec::TransactionCreateCodec::ResponseParameters result =
                            protocol::codec::TransactionCreateCodec::ResponseParameters::decode(*response);
                    txnId = result.response;
                    state = TxnState::ACTIVE;
                } catch (exception::IException &) {
                    TRANSACTION_EXISTS.store(false);
                    throw;
                }
            }

            void TransactionProxy::commit() {
                try {
                    if (state != TxnState::ACTIVE) {
                        throw exception::IllegalStateException("TransactionProxy::commit()",
                                                               "Transaction is not active");
                    }
                    state = TxnState::COMMITTING;
                    checkThread();
                    checkTimeout();

                    std::unique_ptr<protocol::ClientMessage> request =
                            protocol::codec::TransactionCommitCodec::encodeRequest(txnId, threadId);

                    invoke(request);

                    state = TxnState::COMMITTED;
                } catch (exception::IException &e) {
                    state = TxnState::COMMIT_FAILED;
                    TRANSACTION_EXISTS.store(false);
                    util::ExceptionUtil::rethrow(e);
                }
            }

            void TransactionProxy::rollback() {
                try {
                    if (state == TxnState::NO_TXN || state == TxnState::ROLLED_BACK) {
                        throw exception::IllegalStateException("TransactionProxy::rollback()",
                                                               "Transaction is not active");
                    }
                    state = TxnState::ROLLING_BACK;
                    checkThread();
                    try {
                        std::unique_ptr<protocol::ClientMessage> request =
                                protocol::codec::TransactionRollbackCodec::encodeRequest(txnId, threadId);

                        invoke(request);
                    } catch (exception::IException &exception) {
                        clientContext.getLogger().warning("Exception while rolling back the transaction. Exception:",
                                                          exception);
                    }
                    state = TxnState::ROLLED_BACK;
                    TRANSACTION_EXISTS.store(false);
                } catch (exception::IException &) {
                    TRANSACTION_EXISTS.store(false);
                    throw;
                }
            }

            serialization::pimpl::SerializationService &TransactionProxy::getSerializationService() {
                return clientContext.getSerializationService();
            }

            std::shared_ptr<connection::Connection> TransactionProxy::getConnection() {
                return connection;
            }

            void TransactionProxy::checkThread() {
                if (threadId != util::getCurrentThreadId()) {
                    throw exception::IllegalStateException("TransactionProxy::checkThread()",
                                                           "Transaction cannot span multiple threads!");
                }
            }

            void TransactionProxy::checkTimeout() {
                if (startTime + options.getTimeoutMillis() < util::currentTimeMillis()) {
                    throw exception::TransactionException("TransactionProxy::checkTimeout()",
                                                          "Transaction is timed-out!");
                }
            }

            TxnState::TxnState(State value)
                    : value(value) {
                values.resize(9);
                values[0] = NO_TXN;
                values[1] = ACTIVE;
                values[2] = PREPARING;
                values[3] = PREPARED;
                values[4] = COMMITTING;
                values[5] = COMMITTED;
                values[6] = COMMIT_FAILED;
                values[7] = ROLLING_BACK;
                values[8] = ROLLED_BACK;
            }

            TxnState::operator int() const {
                return value;
            }

            void TxnState::operator=(int i) {
                value = values[i];
            }

            std::shared_ptr<protocol::ClientMessage> TransactionProxy::invoke(
                    std::unique_ptr<protocol::ClientMessage> &request) {
                return ClientTransactionUtil::invoke(request, getTxnId(), clientContext, connection);
            }

            spi::ClientContext &TransactionProxy::getClientContext() const {
                return clientContext;
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace proxy {
            ReliableTopicImpl::ReliableTopicImpl(const std::string &instanceName, spi::ClientContext *context,
                                                 std::shared_ptr<Ringbuffer<topic::impl::reliable::ReliableTopicMessage> > rb)
                    : proxy::ProxyImpl("hz:impl:topicService", instanceName, context), ringbuffer(rb),
                      logger(context->getLogger()),
                      config(context->getClientConfig().getReliableTopicConfig(instanceName)) {
            }

            void ReliableTopicImpl::publish(const serialization::pimpl::Data &data) {
                std::unique_ptr<Address> nullAddress;
                topic::impl::reliable::ReliableTopicMessage message(data, nullAddress);
                ringbuffer->add(message);
            }
        }
    }
}


namespace hazelcast {
    namespace client {
        namespace proxy {
            const std::string ClientPNCounterProxy::SERVICE_NAME = "hz:impl:PNCounterService";
            const std::shared_ptr<std::set<Address> > ClientPNCounterProxy::EMPTY_ADDRESS_LIST(
                    new std::set<Address>());

            ClientPNCounterProxy::ClientPNCounterProxy(const std::string &serviceName, const std::string &objectName,
                                                       spi::ClientContext *context)
                    : ProxyImpl(serviceName, objectName, context), maxConfiguredReplicaCount(0),
                      observedClock(std::shared_ptr<cluster::impl::VectorClock>(new cluster::impl::VectorClock())),
                      logger(context->getLogger()) {
            }

            std::ostream &operator<<(std::ostream &os, const ClientPNCounterProxy &proxy) {
                os << "PNCounter{name='" << proxy.getName() << "\'}";
                return os;
            }

            int64_t ClientPNCounterProxy::get() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::get",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeGetInternal(EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);
                protocol::codec::PNCounterGetCodec::ResponseParameters resultParameters = protocol::codec::PNCounterGetCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndAdd(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndAdd",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(delta, true, EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::addAndGet(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::addAndGet",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(delta, false,
                                                                                      EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndSubtract(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndSubtract",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(-delta, true,
                                                                                      EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::subtractAndGet(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::subtractAndGet",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(-delta, false,
                                                                                      EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::decrementAndGet() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::decrementAndGet",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(-1, false, EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::incrementAndGet() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::incrementAndGet",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(1, false, EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndDecrement() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndDecrement",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(-1, true, EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndIncrement() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    throw exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndIncrement",
                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members");
                }
                std::shared_ptr<protocol::ClientMessage> response = invokeAddInternal(1, true, EMPTY_ADDRESS_LIST,
                                                                                      std::unique_ptr<exception::HazelcastException>(),
                                                                                      target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        *response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            void ClientPNCounterProxy::reset() {
                observedClock = std::shared_ptr<cluster::impl::VectorClock>(new cluster::impl::VectorClock());
            }

            std::shared_ptr<Address>
            ClientPNCounterProxy::getCRDTOperationTarget(const std::set<Address> &excludedAddresses) {
                if (currentTargetReplicaAddress.get().get() != NULL &&
                    excludedAddresses.find(*currentTargetReplicaAddress.get()) == excludedAddresses.end()) {
                    return currentTargetReplicaAddress;
                }

                {
                    util::LockGuard guard(targetSelectionMutex);
                    if (currentTargetReplicaAddress.get() == NULL ||
                        excludedAddresses.find(*currentTargetReplicaAddress.get()) != excludedAddresses.end()) {
                        currentTargetReplicaAddress = chooseTargetReplica(excludedAddresses);
                    }
                }
                return currentTargetReplicaAddress;
            }

            std::shared_ptr<Address>
            ClientPNCounterProxy::chooseTargetReplica(const std::set<Address> &excludedAddresses) {
                std::vector<Address> replicaAddresses = getReplicaAddresses(excludedAddresses);
                if (replicaAddresses.empty()) {
                    return std::shared_ptr<Address>();
                }
                // TODO: Use a random generator as used in Java (ThreadLocalRandomProvider) which is per thread
                int randomReplicaIndex = std::abs(rand()) % (int) replicaAddresses.size();
                return std::shared_ptr<Address>(new Address(replicaAddresses[randomReplicaIndex]));
            }

            std::vector<Address> ClientPNCounterProxy::getReplicaAddresses(const std::set<Address> &excludedAddresses) {
                std::vector<Member> dataMembers = getContext().getClientClusterService().getMembers(
                        *cluster::memberselector::MemberSelectors::DATA_MEMBER_SELECTOR);
                int32_t maxConfiguredReplicaCount = getMaxConfiguredReplicaCount();
                int currentReplicaCount = util::min<int>(maxConfiguredReplicaCount, (int) dataMembers.size());

                std::vector<Address> replicaAddresses;
                for (int i = 0; i < currentReplicaCount; i++) {
                    const Address &dataMemberAddress = dataMembers[i].getAddress();
                    if (excludedAddresses.find(dataMemberAddress) == excludedAddresses.end()) {
                        replicaAddresses.push_back(dataMemberAddress);
                    }
                }
                return replicaAddresses;
            }

            int32_t ClientPNCounterProxy::getMaxConfiguredReplicaCount() {
                if (maxConfiguredReplicaCount > 0) {
                    return maxConfiguredReplicaCount;
                } else {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::PNCounterGetConfiguredReplicaCountCodec::encodeRequest(
                            getName());
                    maxConfiguredReplicaCount = invokeAndGetResult<int32_t, protocol::codec::PNCounterGetConfiguredReplicaCountCodec::ResponseParameters>(
                            request);
                }
                return maxConfiguredReplicaCount;
            }

            std::shared_ptr<protocol::ClientMessage>
            ClientPNCounterProxy::invokeGetInternal(std::shared_ptr<std::set<Address> > excludedAddresses,
                                                    const std::unique_ptr<exception::IException> &lastException,
                                                    const std::shared_ptr<Address> &target) {
                if (target.get() == NULL) {
                    if (lastException.get()) {
                        throw *lastException;
                    } else {
                        throw (exception::ExceptionBuilder<exception::NoDataMemberInClusterException>(
                                "ClientPNCounterProxy::invokeGetInternal") <<
                                                                           "Cannot invoke operations on a CRDT because the cluster does not contain any data members").build();
                    }
                }
                try {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::PNCounterGetCodec::encodeRequest(
                            getName(), observedClock.get()->entrySet(), *target);
                    return invokeOnAddress(request, *target);
                } catch (exception::HazelcastException &e) {
                    logger.finest("Exception occurred while invoking operation on target ", *target,
                                  ", choosing different target. Cause: ", e);
                    if (excludedAddresses == EMPTY_ADDRESS_LIST) {
                        // TODO: Make sure that this only affects the local variable of the method
                        excludedAddresses = std::shared_ptr<std::set<Address> >(new std::set<Address>());
                    }
                    excludedAddresses->insert(*target);
                    std::shared_ptr<Address> newTarget = getCRDTOperationTarget(*excludedAddresses);
                    std::unique_ptr<exception::IException> exception = e.clone();
                    return invokeGetInternal(excludedAddresses, exception, newTarget);
                }
            }


            std::shared_ptr<protocol::ClientMessage>
            ClientPNCounterProxy::invokeAddInternal(int64_t delta, bool getBeforeUpdate,
                                                    std::shared_ptr<std::set<Address> > excludedAddresses,
                                                    const std::unique_ptr<exception::IException> &lastException,
                                                    const std::shared_ptr<Address> &target) {
                if (target.get() == NULL) {
                    if (lastException.get()) {
                        throw *lastException;
                    } else {
                        throw (exception::ExceptionBuilder<exception::NoDataMemberInClusterException>(
                                "ClientPNCounterProxy::invokeAddInternal") <<
                                                                           "Cannot invoke operations on a CRDT because the cluster does not contain any data members").build();
                    }
                }

                try {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::PNCounterAddCodec::encodeRequest(
                            getName(), delta, getBeforeUpdate, observedClock.get()->entrySet(), *target);
                    return invokeOnAddress(request, *target);
                } catch (exception::HazelcastException &e) {
                    logger.finest("Unable to provide session guarantees when sending operations to ", *target,
                                  ", choosing different target. Cause: ", e);
                    if (excludedAddresses == EMPTY_ADDRESS_LIST) {
                        // TODO: Make sure that this only affects the local variable of the method
                        excludedAddresses = std::shared_ptr<std::set<Address> >(new std::set<Address>());
                    }
                    excludedAddresses->insert(*target);
                    std::shared_ptr<Address> newTarget = getCRDTOperationTarget(*excludedAddresses);
                    std::unique_ptr<exception::IException> exception = e.clone();
                    return invokeAddInternal(delta, getBeforeUpdate, excludedAddresses, exception, newTarget);
                }
            }

            void ClientPNCounterProxy::updateObservedReplicaTimestamps(
                    const std::vector<std::pair<std::string, int64_t> > &receivedLogicalTimestamps) {
                std::shared_ptr<cluster::impl::VectorClock> received = toVectorClock(receivedLogicalTimestamps);
                for (;;) {
                    std::shared_ptr<cluster::impl::VectorClock> currentClock = this->observedClock;
                    if (currentClock->isAfter(*received)) {
                        break;
                    }
                    if (observedClock.compareAndSet(currentClock, received)) {
                        break;
                    }
                }
            }

            std::shared_ptr<cluster::impl::VectorClock> ClientPNCounterProxy::toVectorClock(
                    const std::vector<std::pair<std::string, int64_t> > &replicaLogicalTimestamps) {
                return std::shared_ptr<cluster::impl::VectorClock>(
                        new cluster::impl::VectorClock(replicaLogicalTimestamps));
            }

            std::shared_ptr<Address> ClientPNCounterProxy::getCurrentTargetReplicaAddress() {
                return currentTargetReplicaAddress.get();
            }
        }
    }
}



