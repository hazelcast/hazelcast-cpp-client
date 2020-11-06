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
            ClientLockReferenceIdGenerator::ClientLockReferenceIdGenerator() : reference_id_counter_(0) {}

            int64_t ClientLockReferenceIdGenerator::get_next_reference_id() {
                return ++reference_id_counter_;
            }
        }

        namespace proxy {
            MultiMapImpl::MultiMapImpl(const std::string &instance_name, spi::ClientContext *context)
                    : ProxyImpl(MultiMap::SERVICE_NAME, instance_name, context) {
                // TODO: remove this line once the client instance get_distributed_object works as expected in Java for this proxy type
                lock_reference_id_generator_ = get_context().get_lock_reference_id_generator();
            }

            boost::future<bool> MultiMapImpl::put(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                auto request = protocol::codec::multimap_put_encode(get_name(), key, value,
                                                                         util::get_current_thread_id());
                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<std::vector<serialization::pimpl::Data>> MultiMapImpl::get_data(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_get_encode(get_name(), key,
                                                                         util::get_current_thread_id());
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request, key);
            }

            boost::future<bool> MultiMapImpl::remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                auto request = protocol::codec::multimap_removeentry_encode(get_name(), key, value,
                                                                                 util::get_current_thread_id());
                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<std::vector<serialization::pimpl::Data>> MultiMapImpl::remove_data(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_remove_encode(get_name(), key,
                                                                            util::get_current_thread_id());
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request, key);
            }

            boost::future<std::vector<serialization::pimpl::Data>> MultiMapImpl::key_set_data() {
                auto request = protocol::codec::multimap_keyset_encode(get_name());
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> MultiMapImpl::values_data() {
                auto request = protocol::codec::multimap_values_encode(get_name());
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request);
            }

            boost::future<EntryVector> MultiMapImpl::entry_set_data() {
                auto request = protocol::codec::multimap_entryset_encode(get_name());
                return invoke_and_get_future<EntryVector>(request);
            }

            boost::future<bool> MultiMapImpl::contains_key(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_containskey_encode(get_name(), key,
                                                                                 util::get_current_thread_id());
                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<bool> MultiMapImpl::contains_value(const serialization::pimpl::Data &value) {
                auto request = protocol::codec::multimap_containsvalue_encode(get_name(), value);
                return invoke_and_get_future<bool>(request);
            }

            boost::future<bool> MultiMapImpl::contains_entry(const serialization::pimpl::Data &key,
                                             const serialization::pimpl::Data &value) {
                auto request = protocol::codec::multimap_containsentry_encode(get_name(), key, value,
                                                                                   util::get_current_thread_id());
                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<int> MultiMapImpl::size() {
                auto request = protocol::codec::multimap_size_encode(get_name());
                return invoke_and_get_future<int>(request);
            }

            boost::future<void> MultiMapImpl::clear() {
                auto request = protocol::codec::multimap_clear_encode(get_name());
                return to_void_future(invoke(request));
            }

            boost::future<int> MultiMapImpl::value_count(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_valuecount_encode(get_name(), key,
                                                                                util::get_current_thread_id());
                return invoke_and_get_future<int>(request, key);
            }

            boost::future<boost::uuids::uuid>
            MultiMapImpl::add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entry_event_handler,
                                           bool include_value) {
                return register_listener(create_multi_map_entry_listener_codec(include_value), std::move(entry_event_handler));
            }

            boost::future<boost::uuids::uuid>
            MultiMapImpl::add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entry_event_handler,
                                           bool include_value, Data &&key) {
                return register_listener(create_multi_map_entry_listener_codec(include_value, std::move(key)),
                                        std::move(entry_event_handler));
            }

            boost::future<bool> MultiMapImpl::remove_entry_listener(boost::uuids::uuid registration_id) {
                return get_context().get_client_listener_service().deregister_listener(registration_id);
            }

            boost::future<void> MultiMapImpl::lock(const serialization::pimpl::Data &key) {
                return lock(key, std::chrono::milliseconds(-1));
            }

            boost::future<void> MultiMapImpl::lock(const serialization::pimpl::Data &key, std::chrono::milliseconds lease_time) {
                auto request = protocol::codec::multimap_lock_encode(get_name(), key, util::get_current_thread_id(),
                                                                          std::chrono::duration_cast<std::chrono::milliseconds>(lease_time).count(),
                                                                          lock_reference_id_generator_->get_next_reference_id());
                return to_void_future(invoke_on_partition(request, get_partition_id(key)));
            }

            boost::future<bool> MultiMapImpl::is_locked(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_islocked_encode(get_name(), key);
                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<bool> MultiMapImpl::try_lock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_trylock_encode(get_name(), key,
                                                                             util::get_current_thread_id(), INT64_MAX,
                                                                             0,
                                                                             lock_reference_id_generator_->get_next_reference_id());
                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<bool> MultiMapImpl::try_lock(const serialization::pimpl::Data &key, std::chrono::milliseconds timeout) {
                return try_lock(key, timeout, std::chrono::milliseconds(INT64_MAX));
            }

            boost::future<bool> MultiMapImpl::try_lock(const serialization::pimpl::Data &key, std::chrono::milliseconds timeout, std::chrono::milliseconds lease_time) {
                auto request = protocol::codec::multimap_trylock_encode(get_name(), key, util::get_current_thread_id(),
                        std::chrono::duration_cast<std::chrono::milliseconds>(lease_time).count(),
                        std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(),
                        lock_reference_id_generator_->get_next_reference_id());
                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<void> MultiMapImpl::unlock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_unlock_encode(get_name(), key, util::get_current_thread_id(),
                                                                            lock_reference_id_generator_->get_next_reference_id());
                return to_void_future(invoke_on_partition(request, get_partition_id(key)));
            }

            boost::future<void> MultiMapImpl::force_unlock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::multimap_forceunlock_encode(get_name(), key,
                                                                                 lock_reference_id_generator_->get_next_reference_id());
                return to_void_future(invoke_on_partition(request, get_partition_id(key)));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            MultiMapImpl::create_multi_map_entry_listener_codec(bool include_value) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MultiMapEntryListenerMessageCodec(get_name(), include_value));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            MultiMapImpl::create_multi_map_entry_listener_codec(bool include_value, serialization::pimpl::Data &&key) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MultiMapEntryListenerToKeyCodec(get_name(), include_value, std::move(key)));
            }

            void MultiMapImpl::on_initialize() {
                ProxyImpl::on_initialize();
                lock_reference_id_generator_ = get_context().get_lock_reference_id_generator();
            }

            MultiMapImpl::MultiMapEntryListenerMessageCodec::MultiMapEntryListenerMessageCodec(std::string name,
                                                                                               bool include_value)
                    : name_(std::move(name)), include_value_(include_value) {}

            protocol::ClientMessage
            MultiMapImpl::MultiMapEntryListenerMessageCodec::encode_add_request(bool local_only) const {
                return protocol::codec::multimap_addentrylistener_encode(name_, include_value_, local_only);
            }

            protocol::ClientMessage
            MultiMapImpl::MultiMapEntryListenerMessageCodec::encode_remove_request(
                    boost::uuids::uuid real_registration_id) const {
                return protocol::codec::multimap_removeentrylistener_encode(name_, real_registration_id);
            }

            protocol::ClientMessage
            MultiMapImpl::MultiMapEntryListenerToKeyCodec::encode_add_request(bool local_only) const {
                return protocol::codec::multimap_addentrylistenertokey_encode(name_, key_, include_value_,
                                                                                          local_only);
            }

            protocol::ClientMessage
            MultiMapImpl::MultiMapEntryListenerToKeyCodec::encode_remove_request(
                    boost::uuids::uuid real_registration_id) const {
                return protocol::codec::multimap_removeentrylistener_encode(name_, real_registration_id);
            }

            MultiMapImpl::MultiMapEntryListenerToKeyCodec::MultiMapEntryListenerToKeyCodec(std::string name,
                                                                                           bool include_value,
                                                                                           serialization::pimpl::Data &&key)
                    : name_(std::move(name)), include_value_(include_value), key_(std::move(key)) {}


            ReliableTopicImpl::ReliableTopicImpl(const std::string &instance_name, spi::ClientContext *context)
                    : proxy::ProxyImpl(ReliableTopic::SERVICE_NAME, instance_name, context),
                      logger_(context->get_logger()),
                      config_(context->get_client_config().get_reliable_topic_config(instance_name)) {
                ringbuffer_ = context->get_hazelcast_client_implementation()->get_distributed_object<Ringbuffer>(
                        std::string(TOPIC_RB_PREFIX) + name_);
            }

            boost::future<void> ReliableTopicImpl::publish(serialization::pimpl::Data &&data) {
                topic::impl::reliable::ReliableTopicMessage message(std::move(data), nullptr);
                return to_void_future(ringbuffer_->add(message));
            }

            const std::shared_ptr<std::unordered_set<Member> > PNCounterImpl::EMPTY_ADDRESS_LIST(
                    new std::unordered_set<Member>());

            PNCounterImpl::PNCounterImpl(const std::string &service_name, const std::string &object_name,
                                         spi::ClientContext *context)
                    : ProxyImpl(service_name, object_name, context), maxConfiguredReplicaCount_(0),
                      observedClock_(std::shared_ptr<cluster::impl::VectorClock>(new cluster::impl::VectorClock())),
                      logger_(context->get_logger()) {
            }

            std::ostream &operator<<(std::ostream &os, const PNCounterImpl &proxy) {
                os << "PNCounter{name='" << proxy.get_name() << "\'}";
                return os;
            }

            boost::future<int64_t> PNCounterImpl::get() {
                boost::shared_ptr<Member> target = get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(exception::NoDataMemberInClusterException("ClientPNCounterProxy::get",
                                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invoke_get_internal(EMPTY_ADDRESS_LIST, nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::get_and_add(int64_t delta) {
                boost::shared_ptr<Member> target = get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndAdd",
                                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invoke_add_internal(delta, true, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::add_and_get(int64_t delta) {
                boost::shared_ptr<Member> target = get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(exception::NoDataMemberInClusterException("ClientPNCounterProxy::addAndGet",
                                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invoke_add_internal(delta, false, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::get_and_subtract(int64_t delta) {
                boost::shared_ptr<Member> target = get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndSubtract",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invoke_add_internal(-delta, true, EMPTY_ADDRESS_LIST,nullptr, target);

            }

            boost::future<int64_t> PNCounterImpl::subtract_and_get(int64_t delta) {
                boost::shared_ptr<Member> target = get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::subtractAndGet",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invoke_add_internal(-delta, false, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::decrement_and_get() {
                boost::shared_ptr<Member> target = get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::decrementAndGet",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invoke_add_internal(-1, false, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::increment_and_get() {
                boost::shared_ptr<Member> target = get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::incrementAndGet",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invoke_add_internal(1, false, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::get_and_decrement() {
                boost::shared_ptr<Member> target = get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndDecrement",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invoke_add_internal(-1, true, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<int64_t> PNCounterImpl::get_and_increment() {
                boost::shared_ptr<Member> target = get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
                if (!target) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndIncrement",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                return invoke_add_internal(1, true, EMPTY_ADDRESS_LIST,nullptr, target);
            }

            boost::future<void> PNCounterImpl::reset() {
                observedClock_ = std::shared_ptr<cluster::impl::VectorClock>(new cluster::impl::VectorClock());
                return boost::make_ready_future();
            }

            boost::shared_ptr<Member>
            PNCounterImpl::get_crdt_operation_target(const std::unordered_set<Member> &excluded_addresses) {
                auto replicaAddress = currentTargetReplicaAddress_.load();
                if (replicaAddress && excluded_addresses.find(*replicaAddress) == excluded_addresses.end()) {
                    return replicaAddress;
                }

                {
                    std::lock_guard<std::mutex> guard(targetSelectionMutex_);
                    replicaAddress = currentTargetReplicaAddress_.load();
                    if (!replicaAddress ||
                        excluded_addresses.find(*replicaAddress) != excluded_addresses.end()) {
                        currentTargetReplicaAddress_ = choose_target_replica(excluded_addresses);
                    }
                }
                return currentTargetReplicaAddress_;
            }

            boost::shared_ptr<Member>
            PNCounterImpl::choose_target_replica(const std::unordered_set<Member> &excluded_addresses) {
                std::vector<Member> replicaAddresses = get_replica_addresses(excluded_addresses);
                if (replicaAddresses.empty()) {
                    return nullptr;
                }
                // TODO: Use a random generator as used in Java (ThreadLocalRandomProvider) which is per thread
                int randomReplicaIndex = std::abs(rand()) % (int) replicaAddresses.size();
                return boost::make_shared<Member>(replicaAddresses[randomReplicaIndex]);
            }

            std::vector<Member> PNCounterImpl::get_replica_addresses(const std::unordered_set<Member> &excluded_members) {
                std::vector<Member> dataMembers = get_context().get_client_cluster_service().get_members(
                        *cluster::memberselector::MemberSelectors::DATA_MEMBER_SELECTOR);
                int32_t replicaCount = get_max_configured_replica_count();
                int currentReplicaCount = util::min<int>(replicaCount, (int) dataMembers.size());

                std::vector<Member> replicaMembers;
                for (int i = 0; i < currentReplicaCount; i++) {
                    if (excluded_members.find(dataMembers[i]) == excluded_members.end()) {
                        replicaMembers.push_back(dataMembers[i]);
                    }
                }
                return replicaMembers;
            }

            int32_t PNCounterImpl::get_max_configured_replica_count() {
                if (maxConfiguredReplicaCount_ > 0) {
                    return maxConfiguredReplicaCount_;
                } else {
                    auto request = protocol::codec::pncounter_getconfiguredreplicacount_encode(
                            get_name());
                    maxConfiguredReplicaCount_ = invoke_and_get_future<int32_t>(request).get();
                }
                return maxConfiguredReplicaCount_;
            }

            boost::shared_ptr<Member>
            PNCounterImpl::try_choose_a_new_target(std::shared_ptr<std::unordered_set<Member>> excluded_addresses,
                                               boost::shared_ptr<Member> last_target,
                                               const exception::HazelcastException &last_exception) {
                HZ_LOG(logger_, finest,
                    boost::str(boost::format("Exception occurred while invoking operation on target %1%, "
                                             "choosing different target. Cause: %2%")
                                             % last_target % last_exception)
                );
                if (excluded_addresses == EMPTY_ADDRESS_LIST) {
                    // TODO: Make sure that this only affects the local variable of the method
                    excluded_addresses = std::make_shared<std::unordered_set<Member>>();
                }
                excluded_addresses->insert(*last_target);
                return get_crdt_operation_target(*excluded_addresses);
            }

            boost::future<int64_t>
            PNCounterImpl::invoke_get_internal(std::shared_ptr<std::unordered_set<Member>> excluded_addresses,
                                             std::exception_ptr last_exception,
                                             const boost::shared_ptr<Member> &target) {
                if (!target) {
                    if (last_exception) {
                        std::rethrow_exception(last_exception);
                    } else {
                        BOOST_THROW_EXCEPTION(
                                exception::NoDataMemberInClusterException("ClientPNCounterProxy::invokeGetInternal",
                                                                          "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                    }
                }
                try {
                    auto timestamps = observedClock_.get()->entry_set();
                    auto request = protocol::codec::pncounter_get_encode(get_name(), timestamps, target->get_uuid());
                    return invoke_on_member(request, target->get_uuid()).then(boost::launch::deferred, [=] (boost::future<protocol::ClientMessage> f) {
                        try {
                            return get_and_update_timestamps(std::move(f));
                        } catch (exception::HazelcastException &e) {
                            return invoke_get_internal(excluded_addresses, std::current_exception(),
                                                     try_choose_a_new_target(excluded_addresses, target, e)).get();
                        }
                    });
                } catch (exception::HazelcastException &e) {
                    return invoke_get_internal(excluded_addresses, std::current_exception(),
                                                    try_choose_a_new_target(excluded_addresses, target, e));
                }
            }

            boost::future<int64_t>
            PNCounterImpl::invoke_add_internal(int64_t delta, bool getBeforeUpdate,
                                             std::shared_ptr<std::unordered_set<Member> > excluded_addresses,
                                             std::exception_ptr last_exception,
                                             const boost::shared_ptr<Member> &target) {
                if (!target) {
                    if (last_exception) {
                        std::rethrow_exception(last_exception);
                    } else {
                        BOOST_THROW_EXCEPTION(
                                exception::NoDataMemberInClusterException("ClientPNCounterProxy::invokeGetInternal",
                                                                          "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                    }
                }

                try {
                    auto request = protocol::codec::pncounter_add_encode(
                            get_name(), delta, getBeforeUpdate, observedClock_.get()->entry_set(), target->get_uuid());
                    return invoke_on_member(request, target->get_uuid()).then(boost::launch::deferred, [=] (boost::future<protocol::ClientMessage> f) {
                        try {
                            return get_and_update_timestamps(std::move(f));
                        } catch (exception::HazelcastException &e) {
                            return invoke_add_internal(delta, getBeforeUpdate, excluded_addresses, std::current_exception(),
                                                     try_choose_a_new_target(excluded_addresses, target, e)).get();
                        }
                    });
                } catch (exception::HazelcastException &e) {
                    return invoke_add_internal(delta, getBeforeUpdate, excluded_addresses, std::current_exception(),
                                             try_choose_a_new_target(excluded_addresses, target, e));
                }
            }

            int64_t PNCounterImpl::get_and_update_timestamps(boost::future<protocol::ClientMessage> f) {
                auto msg = f.get();
                auto value = msg.get_first_fixed_sized_field<int64_t>();
                // skip replica count
                msg.get<int32_t>();
                update_observed_replica_timestamps(msg.get<cluster::impl::VectorClock::TimestampVector>());
                return value;
            }

            void PNCounterImpl::update_observed_replica_timestamps(
                    const cluster::impl::VectorClock::TimestampVector &received_logical_timestamps) {
                std::shared_ptr<cluster::impl::VectorClock> received = to_vector_clock(received_logical_timestamps);
                for (;;) {
                    std::shared_ptr<cluster::impl::VectorClock> currentClock = this->observedClock_;
                    if (currentClock->is_after(*received)) {
                        break;
                    }
                    if (observedClock_.compare_and_set(currentClock, received)) {
                        break;
                    }
                }
            }

            std::shared_ptr<cluster::impl::VectorClock> PNCounterImpl::to_vector_clock(
                    const cluster::impl::VectorClock::TimestampVector &replica_logical_timestamps) {
                return std::shared_ptr<cluster::impl::VectorClock>(
                        new cluster::impl::VectorClock(replica_logical_timestamps));
            }

            boost::shared_ptr<Member> PNCounterImpl::get_current_target_replica_address() {
                return currentTargetReplicaAddress_.load();
            }

            IListImpl::IListImpl(const std::string &instance_name, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:listService", instance_name, context) {
                serialization::pimpl::Data keyData = get_context().get_serialization_service().to_data<std::string>(
                        &instance_name);
                partition_id_ = get_partition_id(keyData);
            }

            boost::future<bool> IListImpl::remove_item_listener(boost::uuids::uuid registration_id) {
                return get_context().get_client_listener_service().deregister_listener(registration_id);
            }

            boost::future<int> IListImpl::size() {
                auto request = protocol::codec::list_size_encode(get_name());
                return invoke_and_get_future<int>(request, partition_id_);
            }

            boost::future<bool> IListImpl::is_empty() {
                auto request = protocol::codec::list_isempty_encode(get_name());
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> IListImpl::contains(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_contains_encode(get_name(), element);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IListImpl::to_array_data() {
                auto request = protocol::codec::list_getall_encode(get_name());
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request, partition_id_);
            }

            boost::future<bool> IListImpl::add(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_add_encode(get_name(), element);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> IListImpl::remove(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_remove_encode(get_name(), element);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> IListImpl::contains_all_data(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::list_containsall_encode(get_name(), elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> IListImpl::add_all_data(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::list_addall_encode(get_name(), elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> IListImpl::add_all_data(int index, const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::list_addallwithindex_encode(get_name(), index,
                                                                                 elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> IListImpl::remove_all_data(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::list_compareandremoveall_encode(get_name(), elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> IListImpl::retain_all_data(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::list_compareandretainall_encode(get_name(), elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<void> IListImpl::clear() {
                auto request = protocol::codec::list_clear_encode(get_name());
                return to_void_future(invoke_on_partition(request, partition_id_));
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IListImpl::get_data(int index) {
                auto request = protocol::codec::list_get_encode(get_name(), index);
                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(request, partition_id_);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IListImpl::set_data(int index,
                                                                           const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_set_encode(get_name(), index, element);
                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(request, partition_id_);
            }

            boost::future<void> IListImpl::add(int index, const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_addwithindex_encode(get_name(), index, element);
                return to_void_future(invoke_on_partition(request, partition_id_));
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IListImpl::remove_data(int index) {
                auto request = protocol::codec::list_removewithindex_encode(get_name(), index);
                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(request, partition_id_);
            }

            boost::future<int> IListImpl::index_of(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_indexof_encode(get_name(), element);
                return invoke_and_get_future<int>(request, partition_id_);
            }

            boost::future<int> IListImpl::last_index_of(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::list_lastindexof_encode(get_name(), element);
                return invoke_and_get_future<int>(request, partition_id_);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IListImpl::sub_list_data(int from_index, int to_index) {
                auto request = protocol::codec::list_sub_encode(get_name(), from_index, to_index);
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request, partition_id_);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec> IListImpl::create_item_listener_codec(bool include_value) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new ListListenerMessageCodec(get_name(), include_value));
            }

            IListImpl::ListListenerMessageCodec::ListListenerMessageCodec(std::string name,
                                                                          bool include_value) : name_(std::move(name)),
                                                                                               include_value_(
                                                                                                       include_value) {}

            protocol::ClientMessage
            IListImpl::ListListenerMessageCodec::encode_add_request(bool local_only) const {
                return protocol::codec::list_addlistener_encode(name_, include_value_, local_only);
            }

            protocol::ClientMessage
            IListImpl::ListListenerMessageCodec::encode_remove_request(boost::uuids::uuid real_registration_id) const {
                return protocol::codec::list_removelistener_encode(name_, real_registration_id);
            }

            FlakeIdGeneratorImpl::Block::Block(IdBatch &&id_batch, std::chrono::milliseconds validity)
                    : id_batch_(id_batch), invalid_since_(std::chrono::steady_clock::now() + validity), num_returned_(0) {}

            int64_t FlakeIdGeneratorImpl::Block::next() {
                if (invalid_since_ <= std::chrono::steady_clock::now()) {
                    return INT64_MIN;
                }
                int32_t index;
                do {
                    index = num_returned_;
                    if (index == id_batch_.get_batch_size()) {
                        return INT64_MIN;
                    }
                } while (!num_returned_.compare_exchange_strong(index, index + 1));

                return id_batch_.get_base() + index * id_batch_.get_increment();
            }

            FlakeIdGeneratorImpl::IdBatch::IdIterator FlakeIdGeneratorImpl::IdBatch::endOfBatch;

            const int64_t FlakeIdGeneratorImpl::IdBatch::get_base() const {
                return base_;
            }

            const int64_t FlakeIdGeneratorImpl::IdBatch::get_increment() const {
                return increment_;
            }

            const int32_t FlakeIdGeneratorImpl::IdBatch::get_batch_size() const {
                return batch_size_;
            }

            FlakeIdGeneratorImpl::IdBatch::IdBatch(int64_t base, int64_t increment, int32_t batch_size)
                    : base_(base), increment_(increment), batch_size_(batch_size) {}

            FlakeIdGeneratorImpl::IdBatch::IdIterator &FlakeIdGeneratorImpl::IdBatch::end() {
                return endOfBatch;
            }

            FlakeIdGeneratorImpl::IdBatch::IdIterator FlakeIdGeneratorImpl::IdBatch::iterator() {
                return FlakeIdGeneratorImpl::IdBatch::IdIterator(base_, increment_, batch_size_);
            }

            FlakeIdGeneratorImpl::IdBatch::IdIterator::IdIterator(int64_t base2, const int64_t increment, int32_t remaining) : base2_(
                    base2), increment_(increment), remaining_(remaining) {}

            bool FlakeIdGeneratorImpl::IdBatch::IdIterator::operator==(const FlakeIdGeneratorImpl::IdBatch::IdIterator &rhs) const {
                return base2_ == rhs.base2_ && increment_ == rhs.increment_ && remaining_ == rhs.remaining_;
            }

            bool FlakeIdGeneratorImpl::IdBatch::IdIterator::operator!=(const FlakeIdGeneratorImpl::IdBatch::IdIterator &rhs) const {
                return !(rhs == *this);
            }

            FlakeIdGeneratorImpl::IdBatch::IdIterator::IdIterator() : base2_(-1), increment_(-1), remaining_(-1) {
            }

            FlakeIdGeneratorImpl::IdBatch::IdIterator &FlakeIdGeneratorImpl::IdBatch::IdIterator::operator++() {
                if (remaining_ == 0) {
                    return FlakeIdGeneratorImpl::IdBatch::end();
                }

                --remaining_;

                base2_ += increment_;

                return *this;
            }


            FlakeIdGeneratorImpl::FlakeIdGeneratorImpl(const std::string &service_name, const std::string &object_name,
                                                       spi::ClientContext *context)
                    : ProxyImpl(service_name, object_name, context), block_(nullptr) {
                auto config = context->get_client_config().find_flake_id_generator_config(object_name);
                batch_size_ = config->get_prefetch_count();
                validity_ = config->get_prefetch_validity_duration();
            }

            int64_t FlakeIdGeneratorImpl::new_id_internal() {
                auto b = block_.load();
                if (b) {
                    int64_t res = b->next();
                    if (res != INT64_MIN) {
                        return res;
                    }
                }

                throw std::overflow_error("");
            }

            boost::future<int64_t> FlakeIdGeneratorImpl::new_id() {
                try {
                    return boost::make_ready_future(new_id_internal());
                } catch (std::overflow_error &) {
                    return new_id_batch(batch_size_).then(boost::launch::deferred,
                                                      [=](boost::future<FlakeIdGeneratorImpl::IdBatch> f) {
                                                          auto newBlock = boost::make_shared<Block>(f.get(), validity_);
                                                          auto value = newBlock->next();
                                                          auto b = block_.load();
                                                          block_.compare_exchange_strong(b, newBlock);
                                                          return value;
                                                      });
                }
            }

            boost::future<FlakeIdGeneratorImpl::IdBatch> FlakeIdGeneratorImpl::new_id_batch(int32_t size) {
                auto request = protocol::codec::flakeidgenerator_newidbatch_encode(
                        get_name(), size);
                return invoke(request).then(boost::launch::deferred, [] (boost::future<protocol::ClientMessage> f) {
                    auto msg = f.get();
                    msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN);

                    auto base = msg.get<int64_t>();
                    auto increment = msg.get<int64_t>();
                    auto batch_size = msg.get<int32_t>();
                    return FlakeIdGeneratorImpl::IdBatch(base, increment, batch_size);
                });
            }

            IQueueImpl::IQueueImpl(const std::string &instance_name, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:queueService", instance_name, context) {
                serialization::pimpl::Data data = get_context().get_serialization_service().to_data<std::string>(
                        &instance_name);
                partition_id_ = get_partition_id(data);
            }

            boost::future<bool> IQueueImpl::remove_item_listener(
                    boost::uuids::uuid registration_id) {
                return get_context().get_client_listener_service().deregister_listener(registration_id);
            }

            boost::future<bool> IQueueImpl::offer(const serialization::pimpl::Data &element, std::chrono::milliseconds timeout) {
                auto request = protocol::codec::queue_offer_encode(get_name(), element,
                                                                        std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<void> IQueueImpl::put(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::queue_put_encode(get_name(), element);
                return to_void_future(invoke_on_partition(request, partition_id_));
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IQueueImpl::poll_data(std::chrono::milliseconds timeout) {
                auto request = protocol::codec::queue_poll_encode(get_name(), std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(request, partition_id_);
            }

            boost::future<int> IQueueImpl::remaining_capacity() {
                auto request = protocol::codec::queue_remainingcapacity_encode(get_name());
                return invoke_and_get_future<int>(request, partition_id_);
            }

            boost::future<bool> IQueueImpl::remove(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::queue_remove_encode(get_name(), element);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> IQueueImpl::contains(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::queue_contains_encode(get_name(), element);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IQueueImpl::drain_to_data(size_t max_elements) {
                auto request = protocol::codec::queue_draintomaxsize_encode(get_name(), (int32_t) max_elements);

                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request, partition_id_);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IQueueImpl::drain_to_data() {
                auto request = protocol::codec::queue_drainto_encode(get_name());
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request, partition_id_);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IQueueImpl::peek_data() {
                auto request = protocol::codec::queue_peek_encode(get_name());
                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(request, partition_id_);
            }

            boost::future<int> IQueueImpl::size() {
                auto request = protocol::codec::queue_size_encode(get_name());
                return invoke_and_get_future<int>(request, partition_id_);
            }

            boost::future<bool> IQueueImpl::is_empty() {
                auto request = protocol::codec::queue_isempty_encode(get_name());
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IQueueImpl::to_array_data() {
                auto request = protocol::codec::queue_iterator_encode(get_name());
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request, partition_id_);
            }

            boost::future<bool> IQueueImpl::contains_all_data(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::queue_containsall_encode(get_name(), elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> IQueueImpl::add_all_data(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::queue_addall_encode(get_name(), elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> IQueueImpl::remove_all_data(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::queue_compareandremoveall_encode(get_name(), elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> IQueueImpl::retain_all_data(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::queue_compareandretainall_encode(get_name(), elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<void> IQueueImpl::clear() {
                auto request = protocol::codec::queue_clear_encode(get_name());
                return to_void_future(invoke_on_partition(request, partition_id_));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IQueueImpl::create_item_listener_codec(bool include_value) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new QueueListenerMessageCodec(get_name(), include_value));
            }

            IQueueImpl::QueueListenerMessageCodec::QueueListenerMessageCodec(std::string name,
                                                                             bool include_value) : name_(std::move(name)),
                                                                                                  include_value_(
                                                                                                          include_value) {}

            protocol::ClientMessage
            IQueueImpl::QueueListenerMessageCodec::encode_add_request(bool local_only) const {
                return protocol::codec::queue_addlistener_encode(name_, include_value_, local_only);
            }

            protocol::ClientMessage
            IQueueImpl::QueueListenerMessageCodec::encode_remove_request(boost::uuids::uuid real_registration_id) const {
                return protocol::codec::queue_removelistener_encode(name_, real_registration_id);
            }

            ProxyImpl::ProxyImpl(const std::string &service_name, const std::string &object_name,
                                 spi::ClientContext *context)
                    : ClientProxy(object_name, service_name, *context), SerializingProxy(*context, object_name) {}

            ProxyImpl::~ProxyImpl() = default;

            SerializingProxy::SerializingProxy(spi::ClientContext &context, const std::string &object_name)
                    : serializationService_(context.get_serialization_service()),
                      partitionService_(context.get_partition_service()), object_name_(object_name), client_context_(context) {}

            int SerializingProxy::get_partition_id(const serialization::pimpl::Data &key) {
                return partitionService_.get_partition_id(key);
            }

            boost::future<protocol::ClientMessage> SerializingProxy::invoke_on_partition(
                    protocol::ClientMessage &request, int partition_id) {
                try {
                    return spi::impl::ClientInvocation::create(client_context_, std::make_shared<protocol::ClientMessage>(std::move(request)), object_name_, partition_id)->invoke();
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                    return boost::make_ready_future(protocol::ClientMessage(0));
                }
            }

            boost::future<protocol::ClientMessage> SerializingProxy::invoke(protocol::ClientMessage &request) {
                try {
                    return spi::impl::ClientInvocation::create(client_context_, std::make_shared<protocol::ClientMessage>(std::move(request)), object_name_)->invoke();
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                    return boost::make_ready_future(protocol::ClientMessage(0));
                }
            }

            boost::future<protocol::ClientMessage> SerializingProxy::invoke_on_connection(protocol::ClientMessage &request,
                                                                      std::shared_ptr<connection::Connection> connection) {
                try {
                    return spi::impl::ClientInvocation::create(client_context_, std::make_shared<protocol::ClientMessage>(std::move(request)), object_name_, connection)->invoke();
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                    return boost::make_ready_future(protocol::ClientMessage(0));
                }
            }

            boost::future<protocol::ClientMessage>
            SerializingProxy::invoke_on_key_owner(protocol::ClientMessage &request,
                                        const serialization::pimpl::Data &key_data) {
                try {
                    return invoke_on_partition(request, get_partition_id(key_data));
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                    return boost::make_ready_future(protocol::ClientMessage(0));
                }
            }

            boost::future<protocol::ClientMessage>
            SerializingProxy::invoke_on_member(protocol::ClientMessage &request, boost::uuids::uuid uuid) {
                try {
                    auto invocation = spi::impl::ClientInvocation::create(client_context_, std::make_shared<protocol::ClientMessage>(std::move(request)), object_name_, uuid);
                    return invocation->invoke();
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                    return boost::make_ready_future(protocol::ClientMessage(0));
                }
            }

            template<>
            boost::future<boost::optional<serialization::pimpl::Data>>
            SerializingProxy::invoke_and_get_future(protocol::ClientMessage &request) {
                return decode_optional_var_sized<serialization::pimpl::Data>(invoke(request));
            }

            template<>
            boost::future<boost::optional<map::DataEntryView>>
            SerializingProxy::invoke_and_get_future(protocol::ClientMessage &request, const serialization::pimpl::Data &key) {
                return decode_optional_var_sized<map::DataEntryView>(invoke_on_key_owner(request, key));
            }

            template<>
            boost::future<boost::optional<serialization::pimpl::Data>>
            SerializingProxy::invoke_and_get_future(protocol::ClientMessage &request, int partition_id) {
                return decode_optional_var_sized<serialization::pimpl::Data>(invoke_on_partition(request, partition_id));
            }

            template<>
            boost::future<boost::optional<serialization::pimpl::Data>>
            SerializingProxy::invoke_and_get_future(protocol::ClientMessage &request, const serialization::pimpl::Data &key) {
                return decode_optional_var_sized<serialization::pimpl::Data>(invoke_on_key_owner(request, key));
            }

            PartitionSpecificClientProxy::PartitionSpecificClientProxy(const std::string &service_name,
                                                                       const std::string &object_name,
                                                                       spi::ClientContext *context) : ProxyImpl(
                    service_name, object_name, context), partitionId_(-1) {}

            void PartitionSpecificClientProxy::on_initialize() {
                std::string partitionKey = internal::partition::strategy::StringPartitioningStrategy::get_partition_key(
                        name_);
                partitionId_ = get_context().get_partition_service().get_partition_id(to_data<std::string>(partitionKey));
            }
            
            IMapImpl::IMapImpl(const std::string &instance_name, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:mapService", instance_name, context) {}

            boost::future<bool> IMapImpl::contains_key(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_containskey_encode(get_name(), key, util::get_current_thread_id());
                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<bool> IMapImpl::contains_value(const serialization::pimpl::Data &value) {
                auto request = protocol::codec::map_containsvalue_encode(get_name(), value);
                return invoke_and_get_future<bool>(request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IMapImpl::get_data(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_get_encode(get_name(), key, util::get_current_thread_id());
                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(request, key);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IMapImpl::remove_data(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_remove_encode(get_name(), key, util::get_current_thread_id());
                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(request, key);
            }

            boost::future<bool> IMapImpl::remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                auto request = protocol::codec::map_removeifsame_encode(get_name(), key, value,
                                                                             util::get_current_thread_id());
                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<protocol::ClientMessage> IMapImpl::remove_all(const serialization::pimpl::Data &predicate_data) {
                auto request = protocol::codec::map_removeall_encode(get_name(), predicate_data);
                return invoke(request);
            }

            boost::future<protocol::ClientMessage> IMapImpl::delete_entry(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_delete_encode(get_name(), key, util::get_current_thread_id());
                return invoke_on_partition(request, get_partition_id(key));
            }

            boost::future<protocol::ClientMessage> IMapImpl::flush() {
                auto request = protocol::codec::map_flush_encode(get_name());
                return invoke(request);
            }

            boost::future<bool> IMapImpl::try_remove(const serialization::pimpl::Data &key, std::chrono::milliseconds timeout) {
                auto request = protocol::codec::map_tryremove_encode(get_name(), key,
                                                                          util::get_current_thread_id(),
                                                                          std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<bool> IMapImpl::try_put(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                  std::chrono::milliseconds timeout) {
                auto request = protocol::codec::map_tryput_encode(get_name(), key, value,
                                                                       util::get_current_thread_id(),
                                                                       std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IMapImpl::put_data(const serialization::pimpl::Data &key,
                                                                          const serialization::pimpl::Data &value,
                                                                          std::chrono::milliseconds ttl) {
                auto request = protocol::codec::map_put_encode(get_name(), key, value,
                                                                    util::get_current_thread_id(),
                                                                    std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(request, key);
            }

            boost::future<protocol::ClientMessage> IMapImpl::put_transient(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                        std::chrono::milliseconds ttl) {
                auto request = protocol::codec::map_puttransient_encode(get_name(), key, value,
                                                                             util::get_current_thread_id(),
                                                                             std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
                return invoke_on_partition(request, get_partition_id(key));
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IMapImpl::put_if_absent_data(const serialization::pimpl::Data &key,
                                                                                  const serialization::pimpl::Data &value,
                                                                                  std::chrono::milliseconds ttl) {
                auto request = protocol::codec::map_putifabsent_encode(get_name(), key, value,
                                                                            util::get_current_thread_id(),
                                                                            std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(request, key);
            }

            boost::future<bool> IMapImpl::replace(const serialization::pimpl::Data &key, const serialization::pimpl::Data &old_value,
                                   const serialization::pimpl::Data &new_value) {
                auto request = protocol::codec::map_replaceifsame_encode(get_name(), key, old_value,
                                                                              new_value,
                                                                              util::get_current_thread_id());

                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> IMapImpl::replace_data(const serialization::pimpl::Data &key,
                                                                              const serialization::pimpl::Data &value) {
                auto request = protocol::codec::map_replace_encode(get_name(), key, value,
                                                                        util::get_current_thread_id());

                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(request, key);
            }

            boost::future<protocol::ClientMessage>
            IMapImpl::set(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                          std::chrono::milliseconds ttl) {
                auto request = protocol::codec::map_set_encode(get_name(), key, value,
                                                                           util::get_current_thread_id(),
                                                                           std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                                   ttl).count());
                return invoke_on_partition(request, get_partition_id(key));
            }

            boost::future<protocol::ClientMessage> IMapImpl::lock(const serialization::pimpl::Data &key) {
                return lock(key, std::chrono::milliseconds(-1));
            }

            boost::future<protocol::ClientMessage> IMapImpl::lock(const serialization::pimpl::Data &key, std::chrono::milliseconds lease_time) {
                auto request = protocol::codec::map_lock_encode(get_name(), key, util::get_current_thread_id(),
                                                                     std::chrono::duration_cast<std::chrono::milliseconds>(lease_time).count(),
                                                                     lock_reference_id_generator_->get_next_reference_id());
                return invoke_on_partition(request, get_partition_id(key));
            }

            boost::future<bool> IMapImpl::is_locked(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_islocked_encode(get_name(), key);

                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<bool> IMapImpl::try_lock(const serialization::pimpl::Data &key, std::chrono::milliseconds timeout) {
                return try_lock(key, timeout, std::chrono::milliseconds(-1));
            }

            boost::future<bool>
            IMapImpl::try_lock(const serialization::pimpl::Data &key, std::chrono::milliseconds timeout,
                              std::chrono::milliseconds lease_time) {
                auto request = protocol::codec::map_trylock_encode(get_name(), key, util::get_current_thread_id(),
                        std::chrono::duration_cast<std::chrono::milliseconds>(lease_time).count(),
                        std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(),
                        lock_reference_id_generator_->get_next_reference_id());
                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<protocol::ClientMessage> IMapImpl::unlock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_unlock_encode(get_name(), key, util::get_current_thread_id(),
                                                                       lock_reference_id_generator_->get_next_reference_id());
                return invoke_on_partition(request, get_partition_id(key));
            }

            boost::future<protocol::ClientMessage> IMapImpl::force_unlock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_forceunlock_encode(get_name(), key,
                                                                            lock_reference_id_generator_->get_next_reference_id());
                return invoke_on_partition(request, get_partition_id(key));
            }

            boost::future<std::string> IMapImpl::add_interceptor(const serialization::pimpl::Data &interceptor) {
                auto request = protocol::codec::map_addinterceptor_encode(get_name(), interceptor);
                return invoke_and_get_future<std::string>(request);
            }

            // TODO: We can use generic template Listener instead of impl::BaseEventHandler to prevent the virtual function calls
            boost::future<boost::uuids::uuid> IMapImpl::add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entry_event_handler, bool include_value, int32_t listener_flags) {
                return register_listener(create_map_entry_listener_codec(include_value, listener_flags), std::move(entry_event_handler));
            }

            boost::future<boost::uuids::uuid>
            IMapImpl::add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entry_event_handler,
                    Data &&predicate, bool include_value, int32_t listener_flags) {
                return register_listener(create_map_entry_listener_codec(include_value, std::move(predicate), listener_flags), std::move(entry_event_handler));
            }

            boost::future<bool> IMapImpl::remove_entry_listener(boost::uuids::uuid registration_id) {
                return get_context().get_client_listener_service().deregister_listener(registration_id);
            }

            boost::future<boost::uuids::uuid> IMapImpl::add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entry_event_handler,
                                                   bool include_value, Data &&key, int32_t listener_flags) {
                return register_listener(create_map_entry_listener_codec(include_value, listener_flags, std::move(key)),
                                        std::move(entry_event_handler));
            }

            boost::future<boost::optional<map::DataEntryView>> IMapImpl::get_entry_view_data(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_getentryview_encode(get_name(), key,
                                                                             util::get_current_thread_id());
                return invoke_and_get_future<boost::optional<map::DataEntryView>>(request, key);
            }

            boost::future<bool> IMapImpl::evict(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::map_evict_encode(get_name(), key, util::get_current_thread_id());
                return invoke_and_get_future<bool>(request, key);
            }

            boost::future<protocol::ClientMessage> IMapImpl::evict_all() {
                auto request = protocol::codec::map_evictall_encode(get_name());
                return invoke(request);
            }

            boost::future<EntryVector>
            IMapImpl::get_all_data(int partition_id, const std::vector<serialization::pimpl::Data> &keys) {
                auto request = protocol::codec::map_getall_encode(get_name(), keys);
                return invoke_and_get_future<EntryVector>(request, partition_id);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::key_set_data() {
                auto request = protocol::codec::map_keyset_encode(get_name());
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::key_set_data(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::map_keysetwithpredicate_encode(get_name(), predicate);
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request);
            }

            boost::future<std::pair<std::vector<serialization::pimpl::Data>, query::anchor_data_list>> IMapImpl::key_set_for_paging_predicate_data(
                    protocol::codec::holder::paging_predicate_holder const & predicate) {
                auto request = protocol::codec::map_keysetwithpagingpredicate_encode(get_name(), predicate);
                return invoke(request).then(boost::launch::deferred, [=](boost::future<protocol::ClientMessage> f) {
                    return get_paging_predicate_response<std::vector<serialization::pimpl::Data>>(std::move(f));
                });
            }

            boost::future<EntryVector> IMapImpl::entry_set_data() {
                auto request = protocol::codec::map_entryset_encode(get_name());
                return invoke_and_get_future<EntryVector>(request);
            }

            boost::future<EntryVector> IMapImpl::entry_set_data(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::map_entrieswithpredicate_encode(get_name(), predicate);
                return invoke_and_get_future<EntryVector>(request);
            }

            boost::future<std::pair<EntryVector, query::anchor_data_list>> IMapImpl::entry_set_for_paging_predicate_data(
                    protocol::codec::holder::paging_predicate_holder const & predicate) {
                auto request = protocol::codec::map_entrieswithpagingpredicate_encode(get_name(), predicate);
                return invoke(request).then(boost::launch::deferred, [=](boost::future<protocol::ClientMessage> f) {
                    return get_paging_predicate_response<EntryVector>(std::move(f));
                });
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::values_data() {
                auto request = protocol::codec::map_values_encode(get_name());
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::values_data(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::map_valueswithpredicate_encode(get_name(), predicate);
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request);
            }

            boost::future<std::pair<std::vector<serialization::pimpl::Data>, query::anchor_data_list>>
            IMapImpl::values_for_paging_predicate_data(protocol::codec::holder::paging_predicate_holder const & predicate) {
                auto request = protocol::codec::map_valueswithpagingpredicate_encode(get_name(), predicate);
                return invoke(request).then(boost::launch::deferred, [=](boost::future<protocol::ClientMessage> f) {
                    return get_paging_predicate_response<std::vector<serialization::pimpl::Data>>(std::move(f));
                });
            }

            boost::future<protocol::ClientMessage> IMapImpl::add_index_data(const config::index_config &config) {
                auto request = protocol::codec::map_addindex_encode(get_name(), config);
                return invoke(request);
            }

            boost::future<int> IMapImpl::size() {
                auto request = protocol::codec::map_size_encode(get_name());
                return invoke_and_get_future<int>(request);
            }

            boost::future<bool> IMapImpl::is_empty() {
                auto request = protocol::codec::map_isempty_encode(get_name());
                return invoke_and_get_future<bool>(request);
            }

            boost::future<protocol::ClientMessage> IMapImpl::put_all_data(int partition_id, const EntryVector &entries) {
                auto request = protocol::codec::map_putall_encode(get_name(), entries, true);
                return invoke_on_partition(request, partition_id);
            }

            boost::future<protocol::ClientMessage> IMapImpl::clear_data() {
                auto request = protocol::codec::map_clear_encode(get_name());
                return invoke(request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>>
            IMapImpl::execute_on_key_data(const serialization::pimpl::Data &key,
                                       const serialization::pimpl::Data &processor) {
                auto request = protocol::codec::map_executeonkey_encode(get_name(),
                                                                             processor,
                                                                             key,
                                                                             util::get_current_thread_id());
                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(request, get_partition_id(key));
            }

            boost::future<boost::optional<serialization::pimpl::Data>>
            IMapImpl::submit_to_key_data(const serialization::pimpl::Data &key,
                                       const serialization::pimpl::Data &processor) {
                auto request = protocol::codec::map_submittokey_encode(get_name(),
                                                                                   processor,
                                                                                   key,
                                                                                   util::get_current_thread_id());
                return invoke_on_partition(request, get_partition_id(key)).then([] (boost::future<protocol::ClientMessage> f) {
                    auto msg = f.get();
                    msg.skip_frame();
                    return msg.get_nullable<serialization::pimpl::Data>();
                });
            }

            boost::future<EntryVector> IMapImpl::execute_on_keys_data(const std::vector<serialization::pimpl::Data> &keys,
                                                    const serialization::pimpl::Data &processor) {
                auto request = protocol::codec::map_executeonkeys_encode(get_name(), processor, keys);
                return invoke_and_get_future<EntryVector>(request);
            }

            boost::future<protocol::ClientMessage> IMapImpl::remove_interceptor(const std::string &id) {
                auto request = protocol::codec::map_removeinterceptor_encode(get_name(), id);
                return invoke(request);
            }

            boost::future<EntryVector> IMapImpl::execute_on_entries_data(const serialization::pimpl::Data &entry_processor) {
                auto request = protocol::codec::map_executeonallkeys_encode(
                        get_name(), entry_processor);
                return invoke_and_get_future<EntryVector>(request);

            }

            boost::future<EntryVector>
            IMapImpl::execute_on_entries_data(const serialization::pimpl::Data &entry_processor, const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::map_executewithpredicate_encode(get_name(),
                                                                                     entry_processor,
                                                                                     predicate);
                return invoke_and_get_future<EntryVector>(request);
            }


            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::create_map_entry_listener_codec(bool include_value, serialization::pimpl::Data &&predicate,
                                                  int32_t listener_flags) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerWithPredicateMessageCodec(get_name(), include_value, listener_flags, std::move(predicate)));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::create_map_entry_listener_codec(bool include_value, int32_t listener_flags) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerMessageCodec(get_name(), include_value, listener_flags));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::create_map_entry_listener_codec(bool include_value, int32_t listener_flags,
                                                  serialization::pimpl::Data &&key) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerToKeyCodec(get_name(), include_value, listener_flags, std::move(key)));
            }

            void IMapImpl::on_initialize() {
                ProxyImpl::on_initialize();
                lock_reference_id_generator_ = get_context().get_lock_reference_id_generator();
            }

            IMapImpl::MapEntryListenerMessageCodec::MapEntryListenerMessageCodec(std::string name,
                                                                                 bool include_value,
                                                                                 int32_t listener_flags) : name_(std::move(name)),
                                                                                                          include_value_(
                                                                                                                  include_value),
                                                                                                          listener_flags_(
                                                                                                                  listener_flags) {}

            protocol::ClientMessage
            IMapImpl::MapEntryListenerMessageCodec::encode_add_request(bool local_only) const {
                return protocol::codec::map_addentrylistener_encode(name_, include_value_,
                                                                                static_cast<int32_t>(listener_flags_),
                                                                                local_only);
            }

            protocol::ClientMessage
            IMapImpl::MapEntryListenerMessageCodec::encode_remove_request(boost::uuids::uuid real_registration_id) const {
                return protocol::codec::map_removeentrylistener_encode(name_, real_registration_id);
            }

            protocol::ClientMessage
            IMapImpl::MapEntryListenerToKeyCodec::encode_add_request(bool local_only) const {
                return protocol::codec::map_addentrylistenertokey_encode(name_, key_, include_value_,
                                                                                     static_cast<int32_t>(listener_flags_), local_only);
            }

            protocol::ClientMessage
            IMapImpl::MapEntryListenerToKeyCodec::encode_remove_request(boost::uuids::uuid real_registration_id) const {
                return protocol::codec::map_removeentrylistener_encode(name_, real_registration_id);
            }

            IMapImpl::MapEntryListenerToKeyCodec::MapEntryListenerToKeyCodec(std::string name, bool include_value,
                                                                             int32_t listener_flags,
                                                                             serialization::pimpl::Data key)
                    : name_(std::move(name)), include_value_(include_value), listener_flags_(listener_flags), key_(std::move(key)) {}

            IMapImpl::MapEntryListenerWithPredicateMessageCodec::MapEntryListenerWithPredicateMessageCodec(
                    std::string name, bool include_value, int32_t listener_flags,
                    serialization::pimpl::Data &&predicate) : name_(std::move(name)), include_value_(include_value),
                                                             listener_flags_(listener_flags), predicate_(std::move(predicate)) {}

            protocol::ClientMessage
            IMapImpl::MapEntryListenerWithPredicateMessageCodec::encode_add_request(bool local_only) const {
                return protocol::codec::map_addentrylistenerwithpredicate_encode(name_, predicate_,
                                                                                             include_value_,
                                                                                             static_cast<int32_t>(listener_flags_), local_only);
            }

            protocol::ClientMessage
            IMapImpl::MapEntryListenerWithPredicateMessageCodec::encode_remove_request(
                    boost::uuids::uuid real_registration_id) const {
                return protocol::codec::map_removeentrylistener_encode(name_, real_registration_id);
            }

            TransactionalQueueImpl::TransactionalQueueImpl(const std::string &name,
                                                           txn::TransactionProxy &transaction_proxy)
                    : TransactionalObject(IQueue::SERVICE_NAME, name, transaction_proxy) {}

            boost::future<bool> TransactionalQueueImpl::offer(const serialization::pimpl::Data &e, std::chrono::milliseconds timeout) {
                auto request = protocol::codec::transactionalqueue_offer_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), e, std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invoke_and_get_future<bool>(request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> TransactionalQueueImpl::poll_data(std::chrono::milliseconds timeout) {
                auto request = protocol::codec::transactionalqueue_poll_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(request);
            }

            boost::future<int> TransactionalQueueImpl::size() {
                auto request = protocol::codec::transactionalqueue_size_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id());

                return invoke_and_get_future<int>(request);
            }

            ISetImpl::ISetImpl(const std::string &instance_name, spi::ClientContext *client_context)
                    : ProxyImpl(ISet::SERVICE_NAME, instance_name, client_context) {
                serialization::pimpl::Data keyData = get_context().get_serialization_service().to_data<std::string>(
                        &instance_name);
                partition_id_ = get_partition_id(keyData);
            }

            boost::future<bool> ISetImpl::remove_item_listener(boost::uuids::uuid registration_id) {
                return get_context().get_client_listener_service().deregister_listener(registration_id);
            }

            boost::future<int> ISetImpl::size() {
                auto request = protocol::codec::set_size_encode(get_name());
                return invoke_and_get_future<int>(request, partition_id_);
            }

            boost::future<bool> ISetImpl::is_empty() {
                auto request = protocol::codec::set_isempty_encode(get_name());
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> ISetImpl::contains(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::set_contains_encode(get_name(), element);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<std::vector<serialization::pimpl::Data>> ISetImpl::to_array_data() {
                auto request = protocol::codec::set_getall_encode(get_name());
                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(request, partition_id_);
            }

            boost::future<bool> ISetImpl::add(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::set_add_encode(get_name(), element);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> ISetImpl::remove(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::set_remove_encode(get_name(), element);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> ISetImpl::contains_all(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::set_containsall_encode(get_name(), elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> ISetImpl::add_all(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::set_addall_encode(get_name(), elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> ISetImpl::remove_all(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::set_compareandremoveall_encode(get_name(), elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<bool> ISetImpl::retain_all(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::set_compareandretainall_encode(get_name(), elements);
                return invoke_and_get_future<bool>(request, partition_id_);
            }

            boost::future<void> ISetImpl::clear() {
                auto request = protocol::codec::set_clear_encode(get_name());
                return to_void_future(invoke_on_partition(request, partition_id_));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            ISetImpl::create_item_listener_codec(bool include_value) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new SetListenerMessageCodec(get_name(), include_value));
            }

            ISetImpl::SetListenerMessageCodec::SetListenerMessageCodec(std::string name, bool include_value)
                    : name_(std::move(name)), include_value_(include_value) {}

            protocol::ClientMessage
            ISetImpl::SetListenerMessageCodec::encode_add_request(bool local_only) const {
                return protocol::codec::set_addlistener_encode(name_, include_value_, local_only);
            }

            protocol::ClientMessage
            ISetImpl::SetListenerMessageCodec::encode_remove_request(boost::uuids::uuid real_registration_id) const {
                return protocol::codec::set_removelistener_encode(name_, real_registration_id);
            }

            ITopicImpl::ITopicImpl(const std::string &instance_name, spi::ClientContext *context)
                    : proxy::ProxyImpl("hz:impl:topicService", instance_name, context),
                    partition_id_(get_partition_id(to_data(instance_name))) {}

            boost::future<void> ITopicImpl::publish(const serialization::pimpl::Data &data) {
                auto request = protocol::codec::topic_publish_encode(get_name(), data);
                return to_void_future(invoke_on_partition(request, partition_id_));
            }

            boost::future<boost::uuids::uuid> ITopicImpl::add_message_listener(std::shared_ptr<impl::BaseEventHandler> topic_event_handler) {
                return register_listener(create_item_listener_codec(), std::move(topic_event_handler));
            }

            boost::future<bool> ITopicImpl::remove_message_listener(boost::uuids::uuid registration_id) {
                return get_context().get_client_listener_service().deregister_listener(registration_id);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec> ITopicImpl::create_item_listener_codec() {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(new TopicListenerMessageCodec(get_name()));
            }

            ITopicImpl::TopicListenerMessageCodec::TopicListenerMessageCodec(std::string name) : name_(std::move(name)) {}

            protocol::ClientMessage
            ITopicImpl::TopicListenerMessageCodec::encode_add_request(bool local_only) const {
                return protocol::codec::topic_addmessagelistener_encode(name_, local_only);
            }

            protocol::ClientMessage
            ITopicImpl::TopicListenerMessageCodec::encode_remove_request(boost::uuids::uuid real_registration_id) const {
                return protocol::codec::topic_removemessagelistener_encode(name_, real_registration_id);
            }

            ReplicatedMapImpl::ReplicatedMapImpl(const std::string &service_name, const std::string &object_name,
                                                 spi::ClientContext *context) : ProxyImpl(service_name, object_name,
                                                                                          context),
                                                                                target_partition_id_(-1) {}

            constexpr int32_t RingbufferImpl::MAX_BATCH_SIZE;
        }

        namespace map {
            const serialization::pimpl::Data &DataEntryView::get_key() const {
                return key_;
            }

            const serialization::pimpl::Data &DataEntryView::get_value() const {
                return value_;
            }

            int64_t DataEntryView::get_cost() const {
                return cost_;
            }

            int64_t DataEntryView::get_creation_time() const {
                return creation_time_;
            }

            int64_t DataEntryView::get_expiration_time() const {
                return expiration_time_;
            }

            int64_t DataEntryView::get_hits() const {
                return hits_;
            }

            int64_t DataEntryView::get_last_access_time() const {
                return last_access_time_;
            }

            int64_t DataEntryView::get_last_stored_time() const {
                return last_stored_time_;
            }

            int64_t DataEntryView::get_last_update_time() const {
                return last_update_time_;
            }

            int64_t DataEntryView::get_version() const {
                return version_;
            }

            int64_t DataEntryView::get_ttl() const {
                return ttl_;
            }

            int64_t DataEntryView::get_max_idle() const {
                return max_idle_;
            }

            DataEntryView::DataEntryView(Data &&key, Data &&value, int64_t cost, int64_t creation_time,
                                         int64_t expiration_time, int64_t hits, int64_t last_access_time,
                                         int64_t last_stored_time, int64_t last_update_time, int64_t version, int64_t ttl,
                                         int64_t max_idle) : key_(std::move(key)), value_(std::move(value)), cost_(cost),
                                                            creation_time_(creation_time), expiration_time_(expiration_time),
                                                            hits_(hits), last_access_time_(last_access_time),
                                                            last_stored_time_(last_stored_time),
                                                            last_update_time_(last_update_time), version_(version), ttl_(ttl),
                                                            max_idle_(max_idle) {}
        }

        namespace topic {
            namespace impl {
                namespace reliable {
                    ReliableTopicExecutor::ReliableTopicExecutor(std::shared_ptr<Ringbuffer> rb,
                                                                 logger &lg)
                            : ringbuffer_(std::move(rb)), q_(10), shutdown_(false) {
                        runner_thread_ = std::thread([&]() { Task(ringbuffer_, q_, shutdown_).run(); });
                    }

                    ReliableTopicExecutor::~ReliableTopicExecutor() {
                        stop();
                    }

                    void ReliableTopicExecutor::start() {}

                    bool ReliableTopicExecutor::stop() {
                        bool expected = false;
                        if (!shutdown_.compare_exchange_strong(expected, true)) {
                            return false;
                        }

                        topic::impl::reliable::ReliableTopicExecutor::Message m;
                        m.type = topic::impl::reliable::ReliableTopicExecutor::CANCEL;
                        m.callback = nullptr;
                        m.sequence = -1;
                        execute(m);
                        runner_thread_.join();
                        return true;
                    }

                    void ReliableTopicExecutor::execute(Message m) {
                        q_.push(m);
                    }

                    void ReliableTopicExecutor::Task::run() {
                        while (!shutdown_) {
                            Message m = q_.pop();
                            if (CANCEL == m.type) {
                                // exit the thread
                                return;
                            }
                            try {
                                auto f = rb_->read_many(m.sequence, 1, m.max_count);
                                while (!shutdown_ && f.wait_for(boost::chrono::seconds(1)) != boost::future_status::ready) {}
                                if (f.is_ready()) {
                                    m.callback->on_response(boost::make_optional<ringbuffer::ReadResultSet>(f.get()));
                                }
                            } catch (exception::IException &) {
                                m.callback->on_failure(std::current_exception());
                            }
                        }
                    }

                    std::string ReliableTopicExecutor::Task::get_name() const {
                        return "ReliableTopicExecutor Task";
                    }

                    ReliableTopicExecutor::Task::Task(std::shared_ptr<Ringbuffer> rb,
                                                      util::BlockingConcurrentQueue<ReliableTopicExecutor::Message> &q,
                                                      std::atomic<bool> &shutdown) : rb_(std::move(rb)), q_(q),
                                                                                       shutdown_(shutdown) {}

                    ReliableTopicMessage::ReliableTopicMessage() : publish_time_(std::chrono::system_clock::now()) {}

                    ReliableTopicMessage::ReliableTopicMessage(
                            hazelcast::client::serialization::pimpl::Data &&payload_data,
                            std::unique_ptr<Address> address)
                            : publish_time_(std::chrono::system_clock::now())
                            , payload_(std::move(payload_data)) {
                        if (address) {
                            publisher_address_ = boost::make_optional(*address);
                        }
                    }

                    std::chrono::system_clock::time_point ReliableTopicMessage::get_publish_time() const {
                        return publish_time_;
                    }

                    const boost::optional<Address> &ReliableTopicMessage::get_publisher_address() const {
                        return publisher_address_;
                    }

                    serialization::pimpl::Data &ReliableTopicMessage::get_payload() {
                        return payload_;
                    }
                }
            }
        }

        namespace serialization {
            int32_t hz_serializer<topic::impl::reliable::ReliableTopicMessage>::get_factory_id() {
                return F_ID;
            }

            int hz_serializer<topic::impl::reliable::ReliableTopicMessage>::get_class_id() {
                return RELIABLE_TOPIC_MESSAGE;
            }

            void hz_serializer<topic::impl::reliable::ReliableTopicMessage>::write_data(
                    const topic::impl::reliable::ReliableTopicMessage &object, ObjectDataOutput &out) {
                out.write<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(object.publish_time_.time_since_epoch()).count());
                out.write_object(object.publisher_address_);
                out.write(object.payload_.to_byte_array());
            }

            topic::impl::reliable::ReliableTopicMessage
            hz_serializer<topic::impl::reliable::ReliableTopicMessage>::read_data(ObjectDataInput &in) {
                topic::impl::reliable::ReliableTopicMessage message;
                auto now = std::chrono::system_clock::now();
                message.publish_time_ = now + std::chrono::milliseconds(in.read<int64_t>()) - now.time_since_epoch();
                message.publisher_address_ = in.read_object<Address>();
                message.payload_ = serialization::pimpl::Data(in.read<std::vector<byte>>().value());
                return message;
            }
        }

        EntryEvent::EntryEvent(const std::string &name, Member &&member, type event_type,
                   TypedData &&key, TypedData &&value, TypedData &&old_value, TypedData &&merging_value)
                : name_(name), member_(std::move(member)), eventType_(event_type), key_(std::move(key)),
                value_(std::move(value)), oldValue_(std::move(old_value)), mergingValue_(std::move(merging_value)) {}

        const TypedData &EntryEvent::get_key() const {
            return key_;
        }

        const TypedData &EntryEvent::get_old_value() const {
            return oldValue_;
        }

        const TypedData &EntryEvent::get_value() const {
            return value_;
        }

        const TypedData &EntryEvent::get_merging_value() const {
            return mergingValue_;
        }

        const Member &EntryEvent::get_member() const {
            return member_;
        }

        EntryEvent::type EntryEvent::get_event_type() const {
            return eventType_;
        }

        const std::string &EntryEvent::get_name() const {
            return name_;
        }

        std::ostream &operator<<(std::ostream &os, const EntryEvent &event) {
            os << "name: " << event.name_ << " member: " << event.member_ << " eventType: " <<
               static_cast<int>(event.eventType_) << " key: " << event.key_.get_type() << " value: " << event.value_.get_type() <<
               " oldValue: " << event.oldValue_.get_type() << " mergingValue: " << event.mergingValue_.get_type();
            return os;
        }

        MapEvent::MapEvent(Member &&member, EntryEvent::type event_type, const std::string &name,
                           int number_of_entries_affected)
                : member_(member), event_type_(event_type), name_(name), number_of_entries_affected_(number_of_entries_affected) {}

        const Member &MapEvent::get_member() const {
            return member_;
        }

        EntryEvent::type MapEvent::get_event_type() const {
            return event_type_;
        }

        const std::string &MapEvent::get_name() const {
            return name_;
        }

        int MapEvent::get_number_of_entries_affected() const {
            return number_of_entries_affected_;
        }

        std::ostream &operator<<(std::ostream &os, const MapEvent &event) {
            os << "MapEvent{member: " << event.member_ << " eventType: " << static_cast<int>(event.event_type_) << " name: " << event.name_
               << " numberOfEntriesAffected: " << event.number_of_entries_affected_;
            return os;
        }

        ItemEventBase::ItemEventBase(const std::string &name, const Member &member, const ItemEventType &event_type)
                : name_(name), member_(member), event_type_(event_type) {}

        const Member &ItemEventBase::get_member() const {
            return member_;
        }

        ItemEventType ItemEventBase::get_event_type() const {
            return event_type_;
        }

        const std::string &ItemEventBase::get_name() const {
            return name_;
        }

        ItemEventBase::~ItemEventBase() = default;

        FlakeIdGenerator::FlakeIdGenerator(const std::string &object_name, spi::ClientContext *context)
                : FlakeIdGeneratorImpl(SERVICE_NAME, object_name, context) {}
    }
}
