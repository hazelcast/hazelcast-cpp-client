/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include <unordered_set>
#include <atomic>

#include "hazelcast/client/impl/ClientLockReferenceIdGenerator.h"
#include "hazelcast/client/proxy/PNCounterImpl.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"
#include "hazelcast/client/proxy/FlakeIdGeneratorImpl.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/listener/listener_service_impl.h"
#include "hazelcast/client/proxy/ReliableTopicImpl.h"
#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicExecutor.h"
#include "hazelcast/client/proxy/RingbufferImpl.h"
#include "hazelcast/client/cluster/impl/VectorClock.h"
#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        constexpr std::chrono::milliseconds IMap::UNSET;

        namespace impl {
            ClientLockReferenceIdGenerator::ClientLockReferenceIdGenerator() : referenceIdCounter(0) {}

            int64_t ClientLockReferenceIdGenerator::getNextReferenceId() {
                return ++referenceIdCounter;
            }
        }

        namespace proxy {
            MultiMapImpl::MultiMapImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl(MultiMap::SERVICE_NAME, instanceName, context) {
                // TODO: remove this line once the client instance getDistributedObject works as expected in Java for this proxy type
                lockReferenceIdGenerator = getContext().getLockReferenceIdGenerator();
            }

            boost::future<bool> MultiMapImpl::put(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                auto request = protocol::codec::multimap_put_encode(getName(), key, value,
                                                                         util::getCurrentThreadId());
                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<std::vector<serialization::pimpl::Data>> MultiMapImpl::getData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_get_encode(getName(), key,
                                                                         util::getCurrentThreadId());
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request, key);
            }

            boost::future<bool> MultiMapImpl::remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                auto request = protocol::codec::multimap_removeentry_encode(getName(), key, value,
                                                                                 util::getCurrentThreadId());
                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<std::vector<serialization::pimpl::Data>> MultiMapImpl::removeData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_remove_encode(getName(), key,
                                                                            util::getCurrentThreadId());
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request, key);
            }

            boost::future<std::vector<serialization::pimpl::Data>> MultiMapImpl::keySetData() {
                auto request = protocol::codec::multimap_keyset_encode(getName());
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> MultiMapImpl::valuesData() {
                auto request = protocol::codec::multimap_values_encode(getName());
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request);
            }

            boost::future<EntryVector> MultiMapImpl::entrySetData() {
                auto request = protocol::codec::multimap_entryset_encode(getName());
                return invokeAndGetFuture<EntryVector>(request);
            }

            boost::future<bool> MultiMapImpl::containsKey(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_containskey_encode(getName(), key,
                                                                                 util::getCurrentThreadId());
                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<bool> MultiMapImpl::containsValue(const serialization::pimpl::Data &value) {
                auto request = protocol::codec::multimap_containsvalue_encode(getName(), value);
                return invokeAndGetFuture<bool>(request);
            }

            boost::future<bool> MultiMapImpl::containsEntry(const serialization::pimpl::Data &key,
                                             const serialization::pimpl::Data &value) {
                auto request = protocol::codec::multimap_containsentry_encode(getName(), key, value,
                                                                                   util::getCurrentThreadId());
                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<int> MultiMapImpl::size() {
                auto request = protocol::codec::multimap_size_encode(getName());
                return invokeAndGetFuture<int>(request);
            }

            boost::future<void> MultiMapImpl::clear() {
                auto request = protocol::codec::multimap_clear_encode(getName());
                return toVoidFuture(invoke(request));
            }

            boost::future<int> MultiMapImpl::valueCount(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_valuecount_encode(getName(), key,
                                                                                util::getCurrentThreadId());
                return invokeAndGetFuture<int>(request, key);
            }

            boost::future<boost::uuids::uuid>
            MultiMapImpl::addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler,
                                           bool includeValue) {
                return registerListener(createMultiMapEntryListenerCodec(includeValue), std::move(entryEventHandler));
            }

            boost::future<boost::uuids::uuid>
            MultiMapImpl::addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler,
                                           bool includeValue, Data &&key) {
                return registerListener(createMultiMapEntryListenerCodec(includeValue, std::move(key)),
                                        std::move(entryEventHandler));
            }

            boost::future<bool> MultiMapImpl::removeEntryListener(boost::uuids::uuid registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            boost::future<void> MultiMapImpl::lock(const serialization::pimpl::Data &key) {
                return lock(key, std::chrono::milliseconds(-1));
            }

            boost::future<void> MultiMapImpl::lock(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration leaseTime) {
                auto request = protocol::codec::multimap_lock_encode(getName(), key, util::getCurrentThreadId(),
                                                                          std::chrono::duration_cast<std::chrono::milliseconds>(leaseTime).count(),
                                                                          lockReferenceIdGenerator->getNextReferenceId());
                return toVoidFuture(invokeOnPartition(request, getPartitionId(key)));
            }

            boost::future<bool> MultiMapImpl::isLocked(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_islocked_encode(getName(), key);
                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<bool> MultiMapImpl::tryLock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_trylock_encode(getName(), key,
                                                                             util::getCurrentThreadId(), INT64_MAX,
                                                                             0,
                                                                             lockReferenceIdGenerator->getNextReferenceId());
                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<bool> MultiMapImpl::tryLock(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration timeout) {
                return tryLock(key, timeout, std::chrono::steady_clock::duration(INT64_MAX));
            }

            boost::future<bool> MultiMapImpl::tryLock(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration timeout, std::chrono::steady_clock::duration leaseTime) {
                auto request = protocol::codec::multimap_trylock_encode(getName(), key, util::getCurrentThreadId(),
                        std::chrono::duration_cast<std::chrono::milliseconds>(leaseTime).count(),
                        std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(),
                        lockReferenceIdGenerator->getNextReferenceId());
                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<void> MultiMapImpl::unlock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_unlock_encode(getName(), key, util::getCurrentThreadId(),
                                                                            lockReferenceIdGenerator->getNextReferenceId());
                return toVoidFuture(invokeOnPartition(request, getPartitionId(key)));
            }

            boost::future<void> MultiMapImpl::forceUnlock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_forceunlock_encode(getName(), key,
                                                                                 lockReferenceIdGenerator->getNextReferenceId());
                return toVoidFuture(invokeOnPartition(request, getPartitionId(key)));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            MultiMapImpl::createMultiMapEntryListenerCodec(bool includeValue) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MultiMapEntryListenerMessageCodec(getName(), includeValue));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            MultiMapImpl::createMultiMapEntryListenerCodec(bool includeValue, serialization::pimpl::Data &&key) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MultiMapEntryListenerToKeyCodec(getName(), includeValue, std::move(key)));
            }

            void MultiMapImpl::onInitialize() {
                ProxyImpl::onInitialize();
                lockReferenceIdGenerator = getContext().getLockReferenceIdGenerator();
            }

            MultiMapImpl::MultiMapEntryListenerMessageCodec::MultiMapEntryListenerMessageCodec(std::string name,
                                                                                               bool includeValue)
                    : name(std::move(name)), includeValue(includeValue) {}

            protocol::ClientMessage
            MultiMapImpl::MultiMapEntryListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::multimap_addentrylistener_encode(name, includeValue, localOnly);
            }

            protocol::ClientMessage
            MultiMapImpl::MultiMapEntryListenerMessageCodec::encodeRemoveRequest(
                    boost::uuids::uuid realRegistrationId) const {
                return protocol::codec::multimap_removeentrylistener_encode(name, realRegistrationId);
            }

            protocol::ClientMessage
            MultiMapImpl::MultiMapEntryListenerToKeyCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::multimap_addentrylistenertokey_encode(name, key, includeValue,
                                                                                          localOnly);
            }

            protocol::ClientMessage
            MultiMapImpl::MultiMapEntryListenerToKeyCodec::encodeRemoveRequest(
                    boost::uuids::uuid realRegistrationId) const {
                return protocol::codec::multimap_removeentrylistener_encode(name, realRegistrationId);
            }

            MultiMapImpl::MultiMapEntryListenerToKeyCodec::MultiMapEntryListenerToKeyCodec(std::string name,
                                                                                           bool includeValue,
                                                                                           serialization::pimpl::Data &&key)
                    : name(std::move(name)), includeValue(includeValue), key(key) {}


            ReliableTopicImpl::ReliableTopicImpl(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ProxyImpl(ReliableTopic::SERVICE_NAME, instanceName, context),
                      logger(context->getLogger()),
                      config(context->getClientConfig().getReliableTopicConfig(instanceName)) {
                ringbuffer = context->getHazelcastClientImplementation()->getDistributedObject<Ringbuffer>(
                        std::string(TOPIC_RB_PREFIX) + name);
            }

            boost::future<void> ReliableTopicImpl::publish(serialization::pimpl::Data &&data) {
                topic::impl::reliable::ReliableTopicMessage message(std::move(data), nullptr);
                return toVoidFuture(ringbuffer->add(message));
            }

            const std::shared_ptr<std::unordered_set<Member> > PNCounterImpl::EMPTY_ADDRESS_LIST(
                    new std::unordered_set<Member>());

            PNCounterImpl::PNCounterImpl(const std::string &serviceName, const std::string &objectName,
                                         spi::ClientContext *context)
                    : ProxyImpl(serviceName, objectName, context), maxConfiguredReplicaCount(0),
                      observedClock(std::shared_ptr<cluster::impl::VectorClock>(new cluster::impl::VectorClock())),
                      logger(context->getLogger()) {
            }

            std::ostream &operator<<(std::ostream &os, const PNCounterImpl &proxy) {
                os << "PNCounter{name='" << proxy.getName() << "\'}";
                return os;
            }

            boost::future<int64_t> PNCounterImpl::get() {
                boost::shared_ptr<Member> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(exception::NoDataMemberInClusterException("ClientPNCounterProxy::get",
                                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invokeGetInternal(EMPTY_ADDRESS_LIST, nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::getAndAdd(int64_t delta) {
                boost::shared_ptr<Member> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndAdd",
                                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invokeAddInternal(delta, true, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::addAndGet(int64_t delta) {
                boost::shared_ptr<Member> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(exception::NoDataMemberInClusterException("ClientPNCounterProxy::addAndGet",
                                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invokeAddInternal(delta, false, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::getAndSubtract(int64_t delta) {
                boost::shared_ptr<Member> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndSubtract",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invokeAddInternal(-delta, true, EMPTY_ADDRESS_LIST,nullptr, target);

            }

            boost::future<int64_t> PNCounterImpl::subtractAndGet(int64_t delta) {
                boost::shared_ptr<Member> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::subtractAndGet",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invokeAddInternal(-delta, false, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::decrementAndGet() {
                boost::shared_ptr<Member> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::decrementAndGet",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invokeAddInternal(-1, false, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::incrementAndGet() {
                boost::shared_ptr<Member> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::incrementAndGet",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invokeAddInternal(1, false, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::getAndDecrement() {
                boost::shared_ptr<Member> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndDecrement",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invokeAddInternal(-1, true, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::getAndIncrement() {
                boost::shared_ptr<Member> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndIncrement",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invokeAddInternal(1, true, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<void> PNCounterImpl::reset() {
                observedClock = std::shared_ptr<cluster::impl::VectorClock>(new cluster::impl::VectorClock());
                return boost::make_ready_future();
            }

            boost::shared_ptr<Member>
            PNCounterImpl::getCRDTOperationTarget(const std::unordered_set<Member> &excludedAddresses) {
                auto replicaAddress = currentTargetReplicaAddress.load();
                if (replicaAddress && excludedAddresses.find(*replicaAddress) == excludedAddresses.end()) {
                    return replicaAddress;
                }

                {
                    std::lock_guard<std::mutex> guard(targetSelectionMutex);
                    replicaAddress = currentTargetReplicaAddress.load();
                    if (!replicaAddress ||
                        excludedAddresses.find(*replicaAddress) != excludedAddresses.end()) {
                        currentTargetReplicaAddress = chooseTargetReplica(excludedAddresses);
                    }
                }
                return currentTargetReplicaAddress;
            }

            boost::shared_ptr<Member>
            PNCounterImpl::chooseTargetReplica(const std::unordered_set<Member> &excludedAddresses) {
                std::vector<Member> replicaAddresses = getReplicaAddresses(excludedAddresses);
                if (replicaAddresses.empty()) {
                    return nullptr;
                }
                // TODO: Use a random generator as used in Java (ThreadLocalRandomProvider) which is per thread
                int randomReplicaIndex = std::abs(rand()) % (int) replicaAddresses.size();
                return boost::make_shared<Member>(replicaAddresses[randomReplicaIndex]);
            }

            std::vector<Member> PNCounterImpl::getReplicaAddresses(const std::unordered_set<Member> &excludedMembers) {
                std::vector<Member> dataMembers = getContext().getClientClusterService().getMembers(
                        *cluster::memberselector::MemberSelectors::DATA_MEMBER_SELECTOR);
                int32_t replicaCount = getMaxConfiguredReplicaCount();
                int currentReplicaCount = util::min<int>(replicaCount, (int) dataMembers.size());

                std::vector<Member> replicaMembers;
                for (int i = 0; i < currentReplicaCount; i++) {
                    if (excludedMembers.find(dataMembers[i]) == excludedMembers.end()) {
                        replicaMembers.push_back(dataMembers[i]);
                    }
                }
                return replicaMembers;
            }

            int32_t PNCounterImpl::getMaxConfiguredReplicaCount() {
                if (maxConfiguredReplicaCount > 0) {
                    return maxConfiguredReplicaCount;
                } else {
                    auto request = protocol::codec::pncounter_getconfiguredreplicacount_encode(
                            getName());
                    maxConfiguredReplicaCount = invokeAndGetFuture<int32_t>(request).get();
                }
                return maxConfiguredReplicaCount;
            }

            boost::shared_ptr<Member>
            PNCounterImpl::tryChooseANewTarget(std::shared_ptr<std::unordered_set<Member>> excludedAddresses,
                                               boost::shared_ptr<Member> lastTarget,
                                               const exception::HazelcastException &lastException) {
                logger.finest("Exception occurred while invoking operation on target ", *lastTarget,
                              ", choosing different target. Cause: ", lastException);
                if (excludedAddresses == EMPTY_ADDRESS_LIST) {
                    // TODO: Make sure that this only affects the local variable of the method
                    excludedAddresses = std::make_shared<std::unordered_set<Member>>();
                }
                excludedAddresses->insert(*lastTarget);
                return getCRDTOperationTarget(*excludedAddresses);
            }

            boost::future<int64_t>
            PNCounterImpl::invokeGetInternal(std::shared_ptr<std::unordered_set<Member>> excludedAddresses,
                                             std::exception_ptr lastException,
                                             const boost::shared_ptr<Member> &target) {
                if (!target) {
                    if (lastException) {
                        std::rethrow_exception(lastException);
                    } else {
                        BOOST_THROW_EXCEPTION(
                                exception::NoDataMemberInClusterException("ClientPNCounterProxy::invokeGetInternal",
                                                                          "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                    }
                }
                try {
                    auto timestamps = observedClock.get()->entrySet();
                    auto request = protocol::codec::pncounter_get_encode(getName(), timestamps, target->getUuid());
                    return invokeOnMember(request, target->getUuid()).then(boost::launch::deferred, [=] (boost::future<protocol::ClientMessage> f) {
                        try {
                            return get_and_update_timestamps(std::move(f));
                        } catch (exception::HazelcastException &e) {
                            return invokeGetInternal(excludedAddresses, std::current_exception(),
                                                     tryChooseANewTarget(excludedAddresses, target, e)).get();
                        }
                    });
                } catch (exception::HazelcastException &e) {
                    return invokeGetInternal(excludedAddresses, std::current_exception(),
                                                    tryChooseANewTarget(excludedAddresses, target, e));
                }
            }

            boost::future<int64_t>
            PNCounterImpl::invokeAddInternal(int64_t delta, bool getBeforeUpdate,
                                             std::shared_ptr<std::unordered_set<Member> > excludedAddresses,
                                             std::exception_ptr lastException,
                                             const boost::shared_ptr<Member> &target) {
                if (!target) {
                    if (lastException) {
                        std::rethrow_exception(lastException);
                    } else {
                        BOOST_THROW_EXCEPTION(
                                exception::NoDataMemberInClusterException("ClientPNCounterProxy::invokeGetInternal",
                                                                          "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                    }
                }

                try {
                    auto request = protocol::codec::pncounter_add_encode(
                            getName(), delta, getBeforeUpdate, observedClock.get()->entrySet(), target->getUuid());
                    return invokeOnMember(request, target->getUuid()).then(boost::launch::deferred, [=] (boost::future<protocol::ClientMessage> f) {
                        try {
                            return get_and_update_timestamps(std::move(f));
                        } catch (exception::HazelcastException &e) {
                            return invokeAddInternal(delta, getBeforeUpdate, excludedAddresses, std::current_exception(),
                                                     tryChooseANewTarget(excludedAddresses, target, e)).get();
                        }
                    });
                } catch (exception::HazelcastException &e) {
                    return invokeAddInternal(delta, getBeforeUpdate, excludedAddresses, std::current_exception(),
                                             tryChooseANewTarget(excludedAddresses, target, e));
                }
            }

            int64_t PNCounterImpl::get_and_update_timestamps(boost::future<protocol::ClientMessage> f) {
                auto msg = f.get();
                auto value = msg.get_first_fixed_sized_field<int64_t>();
                // skip replica count
                msg.get<int32_t>();
                updateObservedReplicaTimestamps(msg.get<cluster::impl::VectorClock::TimestampVector>());
                return value;
            }

            void PNCounterImpl::updateObservedReplicaTimestamps(
                    const cluster::impl::VectorClock::TimestampVector &receivedLogicalTimestamps) {
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

            std::shared_ptr<cluster::impl::VectorClock> PNCounterImpl::toVectorClock(
                    const cluster::impl::VectorClock::TimestampVector &replicaLogicalTimestamps) {
                return std::shared_ptr<cluster::impl::VectorClock>(
                        new cluster::impl::VectorClock(replicaLogicalTimestamps));
            }

            boost::shared_ptr<Member> PNCounterImpl::getCurrentTargetReplicaAddress() {
                return currentTargetReplicaAddress.load();
            }

            IListImpl::IListImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:listService", instanceName, context) {
                serialization::pimpl::Data keyData = getContext().getSerializationService().toData<std::string>(
                        &instanceName);
                partitionId = getPartitionId(keyData);
            }

            boost::future<bool> IListImpl::removeItemListener(boost::uuids::uuid registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            boost::future<int> IListImpl::size() {
                auto request = protocol::codec::list_size_encode(getName());
                return invokeAndGetFuture<int>(request, partitionId);
            }

            boost::future<bool> IListImpl::isEmpty() {
                auto request = protocol::codec::list_isempty_encode(getName());
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> IListImpl::contains(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_contains_encode(getName(), element);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IListImpl::toArrayData() {
                auto request = protocol::codec::list_getall_encode(getName());
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request, partitionId);
            }

            boost::future<bool> IListImpl::add(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_add_encode(getName(), element);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> IListImpl::remove(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_remove_encode(getName(), element);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> IListImpl::containsAllData(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::list_containsall_encode(getName(), elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> IListImpl::addAllData(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::list_addall_encode(getName(), elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> IListImpl::addAllData(int index, const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::list_addallwithindex_encode(getName(), index,
                                                                                 elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> IListImpl::removeAllData(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::list_compareandremoveall_encode(getName(), elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> IListImpl::retainAllData(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::list_compareandretainall_encode(getName(), elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<void> IListImpl::clear() {
                auto request = protocol::codec::list_clear_encode(getName());
                return toVoidFuture(invokeOnPartition(request, partitionId));
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IListImpl::getData(int index) {
                auto request = protocol::codec::list_get_encode(getName(), index);
                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(request, partitionId);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IListImpl::setData(int index,
                                                                           const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_set_encode(getName(), index, element);
                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(request, partitionId);
            }

            boost::future<void> IListImpl::add(int index, const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_addwithindex_encode(getName(), index, element);
                return toVoidFuture(invokeOnPartition(request, partitionId));
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IListImpl::removeData(int index) {
                auto request = protocol::codec::list_removewithindex_encode(getName(), index);
                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(request, partitionId);
            }

            boost::future<int> IListImpl::indexOf(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_indexof_encode(getName(), element);
                return invokeAndGetFuture<int>(request, partitionId);
            }

            boost::future<int> IListImpl::lastIndexOf(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_lastindexof_encode(getName(), element);
                return invokeAndGetFuture<int>(request, partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IListImpl::subListData(int fromIndex, int toIndex) {
                auto request = protocol::codec::list_sub_encode(getName(), fromIndex, toIndex);
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request, partitionId);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec> IListImpl::createItemListenerCodec(bool includeValue) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new ListListenerMessageCodec(getName(), includeValue));
            }

            IListImpl::ListListenerMessageCodec::ListListenerMessageCodec(std::string name,
                                                                          bool includeValue) : name(std::move(name)),
                                                                                               includeValue(
                                                                                                       includeValue) {}

            protocol::ClientMessage
            IListImpl::ListListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::list_addlistener_encode(name, includeValue, localOnly);
            }

            protocol::ClientMessage
            IListImpl::ListListenerMessageCodec::encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const {
                return protocol::codec::list_removelistener_encode(name, realRegistrationId);
            }

            FlakeIdGeneratorImpl::Block::Block(IdBatch &&idBatch, std::chrono::steady_clock::duration validity)
                    : idBatch(idBatch), invalidSince(std::chrono::steady_clock::now() + validity), numReturned(0) {}

            int64_t FlakeIdGeneratorImpl::Block::next() {
                if (invalidSince <= std::chrono::steady_clock::now()) {
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

            FlakeIdGeneratorImpl::IdBatch::IdIterator FlakeIdGeneratorImpl::IdBatch::endOfBatch;

            const int64_t FlakeIdGeneratorImpl::IdBatch::getBase() const {
                return base;
            }

            const int64_t FlakeIdGeneratorImpl::IdBatch::getIncrement() const {
                return increment;
            }

            const int32_t FlakeIdGeneratorImpl::IdBatch::getBatchSize() const {
                return batchSize;
            }

            FlakeIdGeneratorImpl::IdBatch::IdBatch(int64_t base, int64_t increment, int32_t batchSize)
                    : base(base), increment(increment), batchSize(batchSize) {}

            FlakeIdGeneratorImpl::IdBatch::IdIterator &FlakeIdGeneratorImpl::IdBatch::end() {
                return endOfBatch;
            }

            FlakeIdGeneratorImpl::IdBatch::IdIterator FlakeIdGeneratorImpl::IdBatch::iterator() {
                return FlakeIdGeneratorImpl::IdBatch::IdIterator(base, increment, batchSize);
            }

            FlakeIdGeneratorImpl::IdBatch::IdIterator::IdIterator(int64_t base2, const int64_t increment, int32_t remaining) : base2(
                    base2), increment(increment), remaining(remaining) {}

            bool FlakeIdGeneratorImpl::IdBatch::IdIterator::operator==(const FlakeIdGeneratorImpl::IdBatch::IdIterator &rhs) const {
                return base2 == rhs.base2 && increment == rhs.increment && remaining == rhs.remaining;
            }

            bool FlakeIdGeneratorImpl::IdBatch::IdIterator::operator!=(const FlakeIdGeneratorImpl::IdBatch::IdIterator &rhs) const {
                return !(rhs == *this);
            }

            FlakeIdGeneratorImpl::IdBatch::IdIterator::IdIterator() : base2(-1), increment(-1), remaining(-1) {
            }

            FlakeIdGeneratorImpl::IdBatch::IdIterator &FlakeIdGeneratorImpl::IdBatch::IdIterator::operator++() {
                if (remaining == 0) {
                    return FlakeIdGeneratorImpl::IdBatch::end();
                }

                --remaining;

                base2 += increment;

                return *this;
            }


            FlakeIdGeneratorImpl::FlakeIdGeneratorImpl(const std::string &serviceName, const std::string &objectName,
                                                       spi::ClientContext *context)
                    : ProxyImpl(serviceName, objectName, context), block(nullptr) {
                auto config = context->getClientConfig().findFlakeIdGeneratorConfig(objectName);
                batchSize = config->getPrefetchCount();
                validity = config->getPrefetchValidityDuration();
            }

            int64_t FlakeIdGeneratorImpl::newIdInternal() {
                auto b = block.load();
                if (b) {
                    int64_t res = b->next();
                    if (res != INT64_MIN) {
                        return res;
                    }
                }

                throw std::overflow_error("");
            }

            boost::future<int64_t> FlakeIdGeneratorImpl::newId() {
                try {
                    return boost::make_ready_future(newIdInternal());
                } catch (std::overflow_error &) {
                    return newIdBatch(batchSize).then(boost::launch::deferred,
                                                      [=](boost::future<FlakeIdGeneratorImpl::IdBatch> f) {
                                                          auto newBlock = boost::make_shared<Block>(f.get(), validity);
                                                          auto value = newBlock->next();
                                                          auto b = block.load();
                                                          block.compare_exchange_strong(b, newBlock);
                                                          return value;
                                                      });
                }
            }

            boost::future<FlakeIdGeneratorImpl::IdBatch> FlakeIdGeneratorImpl::newIdBatch(int32_t size) {
                auto request = protocol::codec::flakeidgenerator_newidbatch_encode(
                        getName(), size);
                return invoke(request).then(boost::launch::deferred, [] (boost::future<protocol::ClientMessage> f) {
                    auto msg = f.get();
                    msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN);
                    return FlakeIdGeneratorImpl::IdBatch(msg.get<int64_t>(), msg.get<int64_t>(), msg.get<int32_t>());
                });
            }

            IQueueImpl::IQueueImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:queueService", instanceName, context) {
                serialization::pimpl::Data data = getContext().getSerializationService().toData<std::string>(
                        &instanceName);
                partitionId = getPartitionId(data);
            }

            boost::future<bool> IQueueImpl::removeItemListener(
                    boost::uuids::uuid registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            boost::future<bool> IQueueImpl::offer(const serialization::pimpl::Data &element, std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::queue_offer_encode(getName(), element,
                                                                        std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<void> IQueueImpl::put(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::queue_put_encode(getName(), element);
                return toVoidFuture(invokeOnPartition(request, partitionId));
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IQueueImpl::pollData(std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::queue_poll_encode(getName(), std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(request, partitionId);
            }

            boost::future<int> IQueueImpl::remainingCapacity() {
                auto request = protocol::codec::queue_remainingcapacity_encode(getName());
                return invokeAndGetFuture<int>(request, partitionId);
            }

            boost::future<bool> IQueueImpl::remove(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::queue_remove_encode(getName(), element);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> IQueueImpl::contains(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::queue_contains_encode(getName(), element);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IQueueImpl::drainToData(size_t maxElements) {
                auto request = protocol::codec::queue_draintomaxsize_encode(getName(), (int32_t) maxElements);

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request, partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IQueueImpl::drainToData() {
                auto request = protocol::codec::queue_drainto_encode(getName());
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request, partitionId);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IQueueImpl::peekData() {
                auto request = protocol::codec::queue_peek_encode(getName());
                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(request, partitionId);
            }

            boost::future<int> IQueueImpl::size() {
                auto request = protocol::codec::queue_size_encode(getName());
                return invokeAndGetFuture<int>(request, partitionId);
            }

            boost::future<bool> IQueueImpl::isEmpty() {
                auto request = protocol::codec::queue_isempty_encode(getName());
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IQueueImpl::toArrayData() {
                auto request = protocol::codec::queue_iterator_encode(getName());
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request, partitionId);
            }

            boost::future<bool> IQueueImpl::containsAllData(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::queue_containsall_encode(getName(), elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> IQueueImpl::addAllData(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::queue_addall_encode(getName(), elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> IQueueImpl::removeAllData(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::queue_compareandremoveall_encode(getName(), elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> IQueueImpl::retainAllData(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::queue_compareandretainall_encode(getName(), elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<void> IQueueImpl::clear() {
                auto request = protocol::codec::queue_clear_encode(getName());
                return toVoidFuture(invokeOnPartition(request, partitionId));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IQueueImpl::createItemListenerCodec(bool includeValue) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new QueueListenerMessageCodec(getName(), includeValue));
            }

            IQueueImpl::QueueListenerMessageCodec::QueueListenerMessageCodec(std::string name,
                                                                             bool includeValue) : name(std::move(name)),
                                                                                                  includeValue(
                                                                                                          includeValue) {}

            protocol::ClientMessage
            IQueueImpl::QueueListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::queue_addlistener_encode(name, includeValue, localOnly);
            }

            protocol::ClientMessage
            IQueueImpl::QueueListenerMessageCodec::encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const {
                return protocol::codec::queue_removelistener_encode(name, realRegistrationId);
            }

            ProxyImpl::ProxyImpl(const std::string &serviceName, const std::string &objectName,
                                 spi::ClientContext *context)
                    : ClientProxy(objectName, serviceName, *context), SerializingProxy(*context, objectName) {}

            ProxyImpl::~ProxyImpl() = default;

            SerializingProxy::SerializingProxy(spi::ClientContext &context, const std::string &objectName)
                    : serializationService_(context.getSerializationService()),
                      partitionService_(context.getPartitionService()), name_(objectName), context_(context) {}

            int SerializingProxy::getPartitionId(const serialization::pimpl::Data &key) {
                return partitionService_.getPartitionId(key);
            }

            boost::future<protocol::ClientMessage> SerializingProxy::invokeOnPartition(
                    protocol::ClientMessage &request, int partitionId) {
                try {
                    return spi::impl::ClientInvocation::create(context_, std::make_shared<protocol::ClientMessage>(std::move(request)), name_, partitionId)->invoke();
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                    return boost::make_ready_future(protocol::ClientMessage(0));
                }
            }

            boost::future<protocol::ClientMessage> SerializingProxy::invoke(protocol::ClientMessage &request) {
                try {
                    return spi::impl::ClientInvocation::create(context_, std::make_shared<protocol::ClientMessage>(std::move(request)), name_)->invoke();
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                    return boost::make_ready_future(protocol::ClientMessage(0));
                }
            }

            boost::future<protocol::ClientMessage> SerializingProxy::invokeOnConnection(protocol::ClientMessage &request,
                                                                      std::shared_ptr<connection::Connection> connection) {
                try {
                    return spi::impl::ClientInvocation::create(context_, std::make_shared<protocol::ClientMessage>(std::move(request)), name_, connection)->invoke();
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                    return boost::make_ready_future(protocol::ClientMessage(0));
                }
            }

            boost::future<protocol::ClientMessage>
            SerializingProxy::invokeOnKeyOwner(protocol::ClientMessage &request,
                                        const serialization::pimpl::Data &keyData) {
                try {
                    return invokeOnPartition(request, getPartitionId(keyData));
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                    return boost::make_ready_future(protocol::ClientMessage(0));
                }
            }

            boost::future<protocol::ClientMessage>
            SerializingProxy::invokeOnMember(protocol::ClientMessage &request, boost::uuids::uuid uuid) {
                try {
                    auto invocation = spi::impl::ClientInvocation::create(context_, std::make_shared<protocol::ClientMessage>(std::move(request)), name_, uuid);
                    return invocation->invoke();
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                    return boost::make_ready_future(protocol::ClientMessage(0));
                }
            }

            template<>
            boost::future<boost::optional<serialization::pimpl::Data>>
            SerializingProxy::invokeAndGetFuture(protocol::ClientMessage &request) {
                return decode_optional_var_sized<serialization::pimpl::Data>(invoke(request));
            }

            template<>
            boost::future<boost::optional<map::DataEntryView>>
            SerializingProxy::invokeAndGetFuture(protocol::ClientMessage &request, const serialization::pimpl::Data &key) {
                return decode_optional_var_sized<map::DataEntryView>(invokeOnKeyOwner(request, key));
            }

            template<>
            boost::future<boost::optional<serialization::pimpl::Data>>
            SerializingProxy::invokeAndGetFuture(protocol::ClientMessage &request, int partitionId) {
                return decode_optional_var_sized<serialization::pimpl::Data>(invokeOnPartition(request, partitionId));
            }

            template<>
            boost::future<boost::optional<serialization::pimpl::Data>>
            SerializingProxy::invokeAndGetFuture(protocol::ClientMessage &request, const serialization::pimpl::Data &key) {
                return decode_optional_var_sized<serialization::pimpl::Data>(invokeOnKeyOwner(request, key));
            }

            PartitionSpecificClientProxy::PartitionSpecificClientProxy(const std::string &serviceName,
                                                                       const std::string &objectName,
                                                                       spi::ClientContext *context) : ProxyImpl(
                    serviceName, objectName, context), partitionId(-1) {}

            void PartitionSpecificClientProxy::onInitialize() {
                std::string partitionKey = internal::partition::strategy::StringPartitioningStrategy::getPartitionKey(
                        name);
                partitionId = getContext().getPartitionService().getPartitionId(toData<std::string>(partitionKey));
            }
            
            IMapImpl::IMapImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:mapService", instanceName, context) {}

            boost::future<bool> IMapImpl::containsKey(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_containskey_encode(getName(), key, util::getCurrentThreadId());
                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<bool> IMapImpl::containsValue(const serialization::pimpl::Data &value) {
                auto request = protocol::codec::map_containsvalue_encode(getName(), value);
                return invokeAndGetFuture<bool>(request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IMapImpl::getData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_get_encode(getName(), key, util::getCurrentThreadId());
                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(request, key);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IMapImpl::removeData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_remove_encode(getName(), key, util::getCurrentThreadId());
                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(request, key);
            }

            boost::future<bool> IMapImpl::remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                auto request = protocol::codec::map_removeifsame_encode(getName(), key, value,
                                                                             util::getCurrentThreadId());
                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<protocol::ClientMessage> IMapImpl::removeAll(const serialization::pimpl::Data &predicateData) {
                auto request = protocol::codec::map_removeall_encode(getName(), predicateData);
                return invoke(request);
            }

            boost::future<protocol::ClientMessage> IMapImpl::deleteEntry(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_delete_encode(getName(), key, util::getCurrentThreadId());
                return invokeOnPartition(request, getPartitionId(key));
            }

            boost::future<protocol::ClientMessage> IMapImpl::flush() {
                auto request = protocol::codec::map_flush_encode(getName());
                return invoke(request);
            }

            boost::future<bool> IMapImpl::tryRemove(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::map_tryremove_encode(getName(), key,
                                                                          util::getCurrentThreadId(),
                                                                          std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<bool> IMapImpl::tryPut(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                  std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::map_tryput_encode(getName(), key, value,
                                                                       util::getCurrentThreadId(),
                                                                       std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IMapImpl::putData(const serialization::pimpl::Data &key,
                                                                          const serialization::pimpl::Data &value,
                                                                          std::chrono::steady_clock::duration ttl) {
                auto request = protocol::codec::map_put_encode(getName(), key, value,
                                                                    util::getCurrentThreadId(),
                                                                    std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(request, key);
            }

            boost::future<protocol::ClientMessage> IMapImpl::putTransient(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                        std::chrono::steady_clock::duration ttl) {
                auto request = protocol::codec::map_puttransient_encode(getName(), key, value,
                                                                             util::getCurrentThreadId(),
                                                                             std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
                return invokeOnPartition(request, getPartitionId(key));
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IMapImpl::putIfAbsentData(const serialization::pimpl::Data &key,
                                                                                  const serialization::pimpl::Data &value,
                                                                                  std::chrono::steady_clock::duration ttl) {
                auto request = protocol::codec::map_putifabsent_encode(getName(), key, value,
                                                                            util::getCurrentThreadId(),
                                                                            std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(request, key);
            }

            boost::future<bool> IMapImpl::replace(const serialization::pimpl::Data &key, const serialization::pimpl::Data &oldValue,
                                   const serialization::pimpl::Data &newValue) {
                auto request = protocol::codec::map_replaceifsame_encode(getName(), key, oldValue,
                                                                              newValue,
                                                                              util::getCurrentThreadId());

                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IMapImpl::replaceData(const serialization::pimpl::Data &key,
                                                                              const serialization::pimpl::Data &value) {
                auto request = protocol::codec::map_replace_encode(getName(), key, value,
                                                                        util::getCurrentThreadId());

                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(request, key);
            }

            boost::future<protocol::ClientMessage>
            IMapImpl::set(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                          std::chrono::steady_clock::duration ttl) {
                auto request = protocol::codec::map_set_encode(getName(), key, value,
                                                                           util::getCurrentThreadId(),
                                                                           std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                                   ttl).count());
                return invokeOnPartition(request, getPartitionId(key));
            }

            boost::future<protocol::ClientMessage> IMapImpl::lock(const serialization::pimpl::Data &key) {
                return lock(key, std::chrono::milliseconds(-1));
            }

            boost::future<protocol::ClientMessage> IMapImpl::lock(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration leaseTime) {
                auto request = protocol::codec::map_lock_encode(getName(), key, util::getCurrentThreadId(),
                                                                     std::chrono::duration_cast<std::chrono::milliseconds>(leaseTime).count(),
                                                                     lockReferenceIdGenerator->getNextReferenceId());
                return invokeOnPartition(request, getPartitionId(key));
            }

            boost::future<bool> IMapImpl::isLocked(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_islocked_encode(getName(), key);

                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<bool> IMapImpl::tryLock(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration timeout) {
                return tryLock(key, timeout, std::chrono::milliseconds(-1));
            }

            boost::future<bool>
            IMapImpl::tryLock(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration timeout,
                              std::chrono::steady_clock::duration leaseTime) {
                auto request = protocol::codec::map_trylock_encode(getName(), key, util::getCurrentThreadId(),
                        std::chrono::duration_cast<std::chrono::milliseconds>(leaseTime).count(),
                        std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(),
                        lockReferenceIdGenerator->getNextReferenceId());
                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<protocol::ClientMessage> IMapImpl::unlock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_unlock_encode(getName(), key, util::getCurrentThreadId(),
                                                                       lockReferenceIdGenerator->getNextReferenceId());
                return invokeOnPartition(request, getPartitionId(key));
            }

            boost::future<protocol::ClientMessage> IMapImpl::forceUnlock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_forceunlock_encode(getName(), key,
                                                                            lockReferenceIdGenerator->getNextReferenceId());
                return invokeOnPartition(request, getPartitionId(key));
            }

            boost::future<std::string> IMapImpl::addInterceptor(const serialization::pimpl::Data &interceptor) {
                auto request = protocol::codec::map_addinterceptor_encode(getName(), interceptor);
                return invokeAndGetFuture<std::string>(request);
            }

            // TODO: We can use generic template Listener instead of impl::BaseEventHandler to prevent the virtual function calls
            boost::future<boost::uuids::uuid> IMapImpl::addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler, bool includeValue, int32_t listener_flags) {
                return registerListener(createMapEntryListenerCodec(includeValue, listener_flags), std::move(entryEventHandler));
            }

            boost::future<boost::uuids::uuid>
            IMapImpl::addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler,
                    Data &&predicate, bool includeValue, int32_t listener_flags) {
                return registerListener(createMapEntryListenerCodec(includeValue, std::move(predicate), listener_flags), std::move(entryEventHandler));
            }

            boost::future<bool> IMapImpl::removeEntryListener(boost::uuids::uuid registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            boost::future<boost::uuids::uuid> IMapImpl::addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler,
                                                   bool includeValue, Data &&key, int32_t listener_flags) {
                return registerListener(createMapEntryListenerCodec(includeValue, listener_flags, std::move(key)),
                                        std::move(entryEventHandler));
            }

            boost::future<boost::optional<map::DataEntryView>> IMapImpl::getEntryViewData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_getentryview_encode(getName(), key,
                                                                             util::getCurrentThreadId());
                return invokeAndGetFuture<boost::optional<map::DataEntryView>>(request, key);
            }

            boost::future<bool> IMapImpl::evict(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_evict_encode(getName(), key, util::getCurrentThreadId());
                return invokeAndGetFuture<bool>(request, key);
            }

            boost::future<protocol::ClientMessage> IMapImpl::evictAll() {
                auto request = protocol::codec::map_evictall_encode(getName());
                return invoke(request);
            }

            boost::future<EntryVector>
            IMapImpl::getAllData(int partitionId, const std::vector<serialization::pimpl::Data> &keys) {
                auto request = protocol::codec::map_getall_encode(getName(), keys);
                return invokeAndGetFuture<EntryVector>(request, partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::keySetData() {
                auto request = protocol::codec::map_keyset_encode(getName());
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::keySetData(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::map_keysetwithpredicate_encode(getName(), predicate);
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request);
            }

            boost::future<std::pair<std::vector<serialization::pimpl::Data>, query::anchor_data_list>> IMapImpl::keySetForPagingPredicateData(
                    protocol::codec::holder::paging_predicate_holder const & predicate) {
                auto request = protocol::codec::map_keysetwithpagingpredicate_encode(getName(), predicate);
                return invoke(request).then(boost::launch::deferred, [=](boost::future<protocol::ClientMessage> f) {
                    return get_paging_predicate_response<std::vector<serialization::pimpl::Data>>(std::move(f));
                });
            }

            boost::future<EntryVector> IMapImpl::entrySetData() {
                auto request = protocol::codec::map_entryset_encode(getName());
                return invokeAndGetFuture<EntryVector>(request);
            }

            boost::future<EntryVector> IMapImpl::entrySetData(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::map_entrieswithpredicate_encode(getName(), predicate);
                return invokeAndGetFuture<EntryVector>(request);
            }

            boost::future<std::pair<EntryVector, query::anchor_data_list>> IMapImpl::entrySetForPagingPredicateData(
                    protocol::codec::holder::paging_predicate_holder const & predicate) {
                auto request = protocol::codec::map_entrieswithpagingpredicate_encode(getName(), predicate);
                return invoke(request).then(boost::launch::deferred, [=](boost::future<protocol::ClientMessage> f) {
                    return get_paging_predicate_response<EntryVector>(std::move(f));
                });
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::valuesData() {
                auto request = protocol::codec::map_values_encode(getName());
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::valuesData(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::map_valueswithpredicate_encode(getName(), predicate);
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request);
            }

            boost::future<std::pair<std::vector<serialization::pimpl::Data>, query::anchor_data_list>>
            IMapImpl::valuesForPagingPredicateData(protocol::codec::holder::paging_predicate_holder const & predicate) {
                auto request = protocol::codec::map_valueswithpagingpredicate_encode(getName(), predicate);
                return invoke(request).then(boost::launch::deferred, [=](boost::future<protocol::ClientMessage> f) {
                    return get_paging_predicate_response<std::vector<serialization::pimpl::Data>>(std::move(f));
                });
            }

            boost::future<protocol::ClientMessage> IMapImpl::addIndexData(const config::index_config &config) {
                auto request = protocol::codec::map_addindex_encode(getName(), config);
                return invoke(request);
            }

            boost::future<int> IMapImpl::size() {
                auto request = protocol::codec::map_size_encode(getName());
                return invokeAndGetFuture<int>(request);
            }

            boost::future<bool> IMapImpl::isEmpty() {
                auto request = protocol::codec::map_isempty_encode(getName());
                return invokeAndGetFuture<bool>(request);
            }

            boost::future<protocol::ClientMessage> IMapImpl::putAllData(int partitionId, const EntryVector &entries) {
                auto request = protocol::codec::map_putall_encode(getName(), entries, true);
                return invokeOnPartition(request, partitionId);
            }

            boost::future<protocol::ClientMessage> IMapImpl::clearData() {
                auto request = protocol::codec::map_clear_encode(getName());
                return invoke(request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>>
            IMapImpl::executeOnKeyData(const serialization::pimpl::Data &key,
                                       const serialization::pimpl::Data &processor) {
                auto request = protocol::codec::map_executeonkey_encode(getName(),
                                                                             processor,
                                                                             key,
                                                                             util::getCurrentThreadId());
                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(request, getPartitionId(key));
            }

            boost::future<boost::optional<serialization::pimpl::Data>>
            IMapImpl::submitToKeyData(const serialization::pimpl::Data &key,
                                       const serialization::pimpl::Data &processor) {
                auto request = protocol::codec::map_submittokey_encode(getName(),
                                                                                   processor,
                                                                                   key,
                                                                                   util::getCurrentThreadId());
                return invokeOnPartition(request, getPartitionId(key)).then([] (boost::future<protocol::ClientMessage> f) {
                    auto msg = f.get();
                    msg.skip_frame();
                    return msg.getNullable<serialization::pimpl::Data>();
                });
            }

            boost::future<EntryVector> IMapImpl::executeOnKeysData(const std::vector<serialization::pimpl::Data> &keys,
                                                    const serialization::pimpl::Data &processor) {
                auto request = protocol::codec::map_executeonkeys_encode(getName(), processor, keys);
                return invokeAndGetFuture<EntryVector>(request);
            }

            boost::future<protocol::ClientMessage> IMapImpl::removeInterceptor(const std::string &id) {
                auto request = protocol::codec::map_removeinterceptor_encode(getName(), id);
                return invoke(request);
            }

            boost::future<EntryVector> IMapImpl::executeOnEntriesData(const serialization::pimpl::Data &entryProcessor) {
                auto request = protocol::codec::map_executeonallkeys_encode(
                        getName(), entryProcessor);
                return invokeAndGetFuture<EntryVector>(request);

            }

            boost::future<EntryVector>
            IMapImpl::executeOnEntriesData(const serialization::pimpl::Data &entryProcessor, const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::map_executewithpredicate_encode(getName(),
                                                                                     entryProcessor,
                                                                                     predicate);
                return invokeAndGetFuture<EntryVector>(request);
            }


            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::createMapEntryListenerCodec(bool includeValue, serialization::pimpl::Data &&predicate,
                                                  int32_t listenerFlags) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerWithPredicateMessageCodec(getName(), includeValue, listenerFlags, std::move(predicate)));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::createMapEntryListenerCodec(bool includeValue, int32_t listenerFlags) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerMessageCodec(getName(), includeValue, listenerFlags));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::createMapEntryListenerCodec(bool includeValue, int32_t listenerFlags,
                                                  serialization::pimpl::Data &&key) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerToKeyCodec(getName(), includeValue, listenerFlags, key));
            }

            void IMapImpl::onInitialize() {
                ProxyImpl::onInitialize();
                lockReferenceIdGenerator = getContext().getLockReferenceIdGenerator();
            }

            IMapImpl::MapEntryListenerMessageCodec::MapEntryListenerMessageCodec(std::string name,
                                                                                 bool includeValue,
                                                                                 int32_t listenerFlags) : name(std::move(name)),
                                                                                                          includeValue(
                                                                                                                  includeValue),
                                                                                                          listenerFlags(
                                                                                                                  listenerFlags) {}

            protocol::ClientMessage
            IMapImpl::MapEntryListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::map_addentrylistener_encode(name, includeValue,
                                                                                static_cast<int32_t>(listenerFlags),
                                                                                localOnly);
            }

            protocol::ClientMessage
            IMapImpl::MapEntryListenerMessageCodec::encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const {
                return protocol::codec::map_removeentrylistener_encode(name, realRegistrationId);
            }

            protocol::ClientMessage
            IMapImpl::MapEntryListenerToKeyCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::map_addentrylistenertokey_encode(name, key, includeValue,
                                                                                     static_cast<int32_t>(listenerFlags), localOnly);
            }

            protocol::ClientMessage
            IMapImpl::MapEntryListenerToKeyCodec::encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const {
                return protocol::codec::map_removeentrylistener_encode(name, realRegistrationId);
            }

            IMapImpl::MapEntryListenerToKeyCodec::MapEntryListenerToKeyCodec(std::string name, bool includeValue,
                                                                             int32_t listenerFlags,
                                                                             serialization::pimpl::Data key)
                    : name(std::move(name)), includeValue(includeValue), listenerFlags(listenerFlags), key(std::move(key)) {}

            IMapImpl::MapEntryListenerWithPredicateMessageCodec::MapEntryListenerWithPredicateMessageCodec(
                    std::string name, bool includeValue, int32_t listenerFlags,
                    serialization::pimpl::Data &&predicate) : name(std::move(name)), includeValue(includeValue),
                                                             listenerFlags(listenerFlags), predicate(predicate) {}

            protocol::ClientMessage
            IMapImpl::MapEntryListenerWithPredicateMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::map_addentrylistenerwithpredicate_encode(name, predicate,
                                                                                             includeValue,
                                                                                             static_cast<int32_t>(listenerFlags), localOnly);
            }

            protocol::ClientMessage
            IMapImpl::MapEntryListenerWithPredicateMessageCodec::encodeRemoveRequest(
                    boost::uuids::uuid realRegistrationId) const {
                return protocol::codec::map_removeentrylistener_encode(name, realRegistrationId);
            }

            TransactionalQueueImpl::TransactionalQueueImpl(const std::string &name,
                                                           txn::TransactionProxy &transactionProxy)
                    : TransactionalObject(IQueue::SERVICE_NAME, name, transactionProxy) {}

            boost::future<bool> TransactionalQueueImpl::offer(const serialization::pimpl::Data &e, std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::transactionalqueue_offer_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e, std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invokeAndGetFuture<bool>(request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> TransactionalQueueImpl::pollData(std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::transactionalqueue_poll_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(request);
            }

            boost::future<int> TransactionalQueueImpl::size() {
                auto request = protocol::codec::transactionalqueue_size_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetFuture<int>(request);
            }

            ISetImpl::ISetImpl(const std::string &instanceName, spi::ClientContext *clientContext)
                    : ProxyImpl(ISet::SERVICE_NAME, instanceName, clientContext) {
                serialization::pimpl::Data keyData = getContext().getSerializationService().toData<std::string>(
                        &instanceName);
                partitionId = getPartitionId(keyData);
            }

            boost::future<bool> ISetImpl::removeItemListener(boost::uuids::uuid registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            boost::future<int> ISetImpl::size() {
                auto request = protocol::codec::set_size_encode(getName());
                return invokeAndGetFuture<int>(request, partitionId);
            }

            boost::future<bool> ISetImpl::isEmpty() {
                auto request = protocol::codec::set_isempty_encode(getName());
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> ISetImpl::contains(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::set_contains_encode(getName(), element);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> ISetImpl::toArrayData() {
                auto request = protocol::codec::set_getall_encode(getName());
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(request, partitionId);
            }

            boost::future<bool> ISetImpl::add(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::set_add_encode(getName(), element);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> ISetImpl::remove(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::set_remove_encode(getName(), element);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> ISetImpl::containsAll(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::set_containsall_encode(getName(), elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> ISetImpl::addAll(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::set_addall_encode(getName(), elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> ISetImpl::removeAll(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::set_compareandremoveall_encode(getName(), elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<bool> ISetImpl::retainAll(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::set_compareandretainall_encode(getName(), elements);
                return invokeAndGetFuture<bool>(request, partitionId);
            }

            boost::future<void> ISetImpl::clear() {
                auto request = protocol::codec::set_clear_encode(getName());
                return toVoidFuture(invokeOnPartition(request, partitionId));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            ISetImpl::createItemListenerCodec(bool includeValue) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new SetListenerMessageCodec(getName(), includeValue));
            }

            ISetImpl::SetListenerMessageCodec::SetListenerMessageCodec(std::string name, bool includeValue)
                    : name(std::move(name)), includeValue(includeValue) {}

            protocol::ClientMessage
            ISetImpl::SetListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::set_addlistener_encode(name, includeValue, localOnly);
            }

            protocol::ClientMessage
            ISetImpl::SetListenerMessageCodec::encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const {
                return protocol::codec::set_removelistener_encode(name, realRegistrationId);
            }

            ITopicImpl::ITopicImpl(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ProxyImpl("hz:impl:topicService", instanceName, context),
                    partitionId(getPartitionId(toData(instanceName))) {}

            boost::future<void> ITopicImpl::publish(const serialization::pimpl::Data &data) {
                auto request = protocol::codec::topic_publish_encode(getName(), data);
                return toVoidFuture(invokeOnPartition(request, partitionId));
            }

            boost::future<boost::uuids::uuid> ITopicImpl::addMessageListener(std::shared_ptr<impl::BaseEventHandler> topicEventHandler) {
                return registerListener(createItemListenerCodec(), std::move(topicEventHandler));
            }

            boost::future<bool> ITopicImpl::removeMessageListener(boost::uuids::uuid registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec> ITopicImpl::createItemListenerCodec() {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(new TopicListenerMessageCodec(getName()));
            }

            ITopicImpl::TopicListenerMessageCodec::TopicListenerMessageCodec(std::string name) : name(std::move(name)) {}

            protocol::ClientMessage
            ITopicImpl::TopicListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::topic_addmessagelistener_encode(name, localOnly);
            }

            protocol::ClientMessage
            ITopicImpl::TopicListenerMessageCodec::encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const {
                return protocol::codec::topic_removemessagelistener_encode(name, realRegistrationId);
            }

            ReplicatedMapImpl::ReplicatedMapImpl(const std::string &serviceName, const std::string &objectName,
                                                 spi::ClientContext *context) : ProxyImpl(serviceName, objectName,
                                                                                          context),
                                                                                targetPartitionId(-1) {}

            constexpr int32_t RingbufferImpl::MAX_BATCH_SIZE;
        }

        namespace map {
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

            int64_t DataEntryView::getTtl() const {
                return ttl;
            }

            int64_t DataEntryView::getMaxIdle() const {
                return maxIdle;
            }

            DataEntryView::DataEntryView(Data &&key, Data &&value, int64_t cost, int64_t creationTime,
                                         int64_t expirationTime, int64_t hits, int64_t lastAccessTime,
                                         int64_t lastStoredTime, int64_t lastUpdateTime, int64_t version, int64_t ttl,
                                         int64_t maxIdle) : key(key), value(value), cost(cost),
                                                            creationTime(creationTime), expirationTime(expirationTime),
                                                            hits(hits), lastAccessTime(lastAccessTime),
                                                            lastStoredTime(lastStoredTime),
                                                            lastUpdateTime(lastUpdateTime), version(version), ttl(ttl),
                                                            maxIdle(maxIdle) {}
        }

        namespace topic {
            namespace impl {
                namespace reliable {
                    ReliableTopicExecutor::ReliableTopicExecutor(std::shared_ptr<Ringbuffer> rb,
                                                                 util::ILogger &logger)
                            : ringbuffer(std::move(rb)), q(10), shutdown(false) {
                        runnerThread = std::thread([&]() { Task(ringbuffer, q, shutdown).run(); });
                    }

                    ReliableTopicExecutor::~ReliableTopicExecutor() {
                        stop();
                    }

                    void ReliableTopicExecutor::start() {}

                    bool ReliableTopicExecutor::stop() {
                        bool expected = false;
                        if (!shutdown.compare_exchange_strong(expected, true)) {
                            return false;
                        }

                        topic::impl::reliable::ReliableTopicExecutor::Message m;
                        m.type = topic::impl::reliable::ReliableTopicExecutor::CANCEL;
                        m.callback = nullptr;
                        m.sequence = -1;
                        execute(m);
                        runnerThread.join();
                        return true;
                    }

                    void ReliableTopicExecutor::execute(Message m) {
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
                                auto f = rb->readMany(m.sequence, 1, m.maxCount);
                                while (!shutdown && f.wait_for(boost::chrono::seconds(1)) != boost::future_status::ready) {}
                                if (f.is_ready()) {
                                    m.callback->onResponse(boost::make_optional<ringbuffer::ReadResultSet>(f.get()));
                                }
                            } catch (exception::IException &) {
                                m.callback->onFailure(std::current_exception());
                            }
                        }
                    }

                    std::string ReliableTopicExecutor::Task::getName() const {
                        return "ReliableTopicExecutor Task";
                    }

                    ReliableTopicExecutor::Task::Task(std::shared_ptr<Ringbuffer> rb,
                                                      util::BlockingConcurrentQueue<ReliableTopicExecutor::Message> &q,
                                                      std::atomic<bool> &shutdown) : rb(std::move(rb)), q(q),
                                                                                       shutdown(shutdown) {}

                    ReliableTopicMessage::ReliableTopicMessage() : publishTime(std::chrono::system_clock::now()) {}

                    ReliableTopicMessage::ReliableTopicMessage(
                            hazelcast::client::serialization::pimpl::Data &&payloadData,
                            std::unique_ptr<Address> address)
                            : publishTime(std::chrono::system_clock::now()), payload(payloadData) {
                        if (address) {
                            publisherAddress = boost::make_optional(*address);
                        }
                    }

                    std::chrono::system_clock::time_point ReliableTopicMessage::getPublishTime() const {
                        return publishTime;
                    }

                    const boost::optional<Address> &ReliableTopicMessage::getPublisherAddress() const {
                        return publisherAddress;
                    }

                    serialization::pimpl::Data &ReliableTopicMessage::getPayload() {
                        return payload;
                    }
                }
            }
        }

        namespace serialization {
            int32_t hz_serializer<topic::impl::reliable::ReliableTopicMessage>::getFactoryId() {
                return F_ID;
            }

            int hz_serializer<topic::impl::reliable::ReliableTopicMessage>::getClassId() {
                return RELIABLE_TOPIC_MESSAGE;
            }

            void hz_serializer<topic::impl::reliable::ReliableTopicMessage>::writeData(
                    const topic::impl::reliable::ReliableTopicMessage &object, ObjectDataOutput &out) {
                out.write<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(object.publishTime.time_since_epoch()).count());
                out.writeObject(object.publisherAddress);
                out.write(object.payload.toByteArray());
            }

            topic::impl::reliable::ReliableTopicMessage
            hz_serializer<topic::impl::reliable::ReliableTopicMessage>::readData(ObjectDataInput &in) {
                topic::impl::reliable::ReliableTopicMessage message;
                auto now = std::chrono::system_clock::now();
                message.publishTime = now + std::chrono::milliseconds(in.read<int64_t>()) - now.time_since_epoch();
                message.publisherAddress = in.readObject<Address>();
                message.payload = serialization::pimpl::Data(in.read<std::vector<byte>>().value());
                return message;
            }
        }

        EntryEvent::EntryEvent(const std::string &name, const Member &member, type eventType,
                   TypedData &&key, TypedData &&value)
                : name(name), member(member), eventType(eventType), key(key), value(value) {
        }

        EntryEvent::EntryEvent(const std::string &name, const Member &member, type eventType,
                   TypedData &&key, TypedData &&value, TypedData &&oldValue, TypedData &&mergingValue)
                : name(name), member(member), eventType(eventType), key(key), value(value), oldValue(oldValue),
                  mergingValue(mergingValue) {}

        EntryEvent::EntryEvent(const std::string &name, const Member &member, type eventType)
                : name(name), member(member), eventType(eventType) {}

        const TypedData &EntryEvent::getKey() const {
            return key;
        }

        const TypedData &EntryEvent::getOldValue() const {
            return oldValue;
        }

        const TypedData &EntryEvent::getValue() const {
            return value;
        }

        const TypedData &EntryEvent::getMergingValue() const {
            return mergingValue;
        }

        const Member &EntryEvent::getMember() const {
            return member;
        }

        EntryEvent::type EntryEvent::getEventType() const {
            return eventType;
        }

        const std::string &EntryEvent::getName() const {
            return name;
        }

        std::ostream &operator<<(std::ostream &os, const EntryEvent &event) {
            os << "name: " << event.name << " member: " << event.member << " eventType: " <<
               static_cast<int>(event.eventType) << " key: " << event.key.getType() << " value: " << event.value.getType() <<
               " oldValue: " << event.oldValue.getType() << " mergingValue: " << event.mergingValue.getType();
            return os;
        }

        MapEvent::MapEvent(Member &&member, EntryEvent::type eventType, const std::string &name,
                           int numberOfEntriesAffected)
                : member(member), eventType(eventType), name(name), numberOfEntriesAffected(numberOfEntriesAffected) {}

        const Member &MapEvent::getMember() const {
            return member;
        }

        EntryEvent::type MapEvent::getEventType() const {
            return eventType;
        }

        const std::string &MapEvent::getName() const {
            return name;
        }

        int MapEvent::getNumberOfEntriesAffected() const {
            return numberOfEntriesAffected;
        }

        std::ostream &operator<<(std::ostream &os, const MapEvent &event) {
            os << "MapEvent{member: " << event.member << " eventType: " << static_cast<int>(event.eventType) << " name: " << event.name
               << " numberOfEntriesAffected: " << event.numberOfEntriesAffected;
            return os;
        }

        ItemEventBase::ItemEventBase(const std::string &name, const Member &member, const ItemEventType &eventType)
                : name(name), member(member), eventType(eventType) {}

        const Member &ItemEventBase::getMember() const {
            return member;
        }

        ItemEventType ItemEventBase::getEventType() const {
            return eventType;
        }

        const std::string &ItemEventBase::getName() const {
            return name;
        }

        ItemEventBase::~ItemEventBase() = default;

        FlakeIdGenerator::FlakeIdGenerator(const std::string &objectName, spi::ClientContext *context)
                : FlakeIdGeneratorImpl(SERVICE_NAME, objectName, context) {}
    }
}
