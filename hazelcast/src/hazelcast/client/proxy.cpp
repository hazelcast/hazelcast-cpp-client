/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/impl/hazelcast_client_instance_impl.h"
#include "hazelcast/client/proxy/flake_id_generator_impl.h"
#include "hazelcast/client/spi/impl/listener/listener_service_impl.h"
#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"
#include "hazelcast/client/client_config.h"
#include "hazelcast/client/map/data_entry_view.h"
#include "hazelcast/client/proxy/RingbufferImpl.h"
#include "hazelcast/client/impl/vector_clock.h"
#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/topic/reliable_listener.h"

namespace hazelcast {
namespace client {
const std::chrono::milliseconds imap::UNSET{ -1 };

reliable_topic::reliable_topic(const std::string& instance_name,
                               spi::ClientContext* context)
  : proxy::ProxyImpl(reliable_topic::SERVICE_NAME, instance_name, context)
  , execution_service_(
      context->get_client_execution_service().shared_from_this())  
  , executor_(context->get_client_execution_service().get_user_executor())
  , logger_(context->get_logger())
{
    auto reliable_config =
      context->get_client_config().lookup_reliable_topic_config(instance_name);
    if (reliable_config) {
        batch_size_ = reliable_config->get_read_batch_size();
    } else {
        batch_size_ = config::reliable_topic_config::DEFAULT_READ_BATCH_SIZE;
    }

    ringbuffer_ =
      context->get_hazelcast_client_implementation()
        ->get_distributed_object<ringbuffer>(
          std::string(reliable_topic::TOPIC_RB_PREFIX) + instance_name);
}

bool
reliable_topic::remove_message_listener(const std::string& registration_id)
{
    int id = util::IOUtil::to_value<int>(registration_id);
    auto runner = runners_map_.get(id);
    if (!runner) {
        return false;
    }
    runner->cancel();
    return true;
}

void
reliable_topic::on_shutdown()
{
    // cancel all runners
    for (auto& entry : runners_map_.clear()) {
        entry.second->cancel();
    }
}

void
reliable_topic::on_destroy()
{
    // cancel all runners
    for (auto& entry : runners_map_.clear()) {
        entry.second->cancel();
    }
}

void
reliable_topic::post_destroy()
{
    // destroy the underlying ringbuffer
    ringbuffer_.get()->destroy().get();
}

namespace topic {
reliable_listener::reliable_listener(bool loss_tolerant,
                                     int64_t initial_sequence_id)
  : loss_tolerant_(loss_tolerant)
  , initial_sequence_id_(initial_sequence_id)
{}
} // namespace topic

namespace impl {
ClientLockReferenceIdGenerator::ClientLockReferenceIdGenerator()
  : reference_id_counter_(0)
{}

int64_t
ClientLockReferenceIdGenerator::get_next_reference_id()
{
    return ++reference_id_counter_;
}
} // namespace impl

namespace proxy {
MultiMapImpl::MultiMapImpl(const std::string& instance_name,
                           spi::ClientContext* context)
  : ProxyImpl(multi_map::SERVICE_NAME, instance_name, context)
{
    // TODO: remove this line once the client instance get_distributed_object
    // works as expected in Java for this proxy type
    lock_reference_id_generator_ =
      get_context().get_lock_reference_id_generator();
}

boost::future<bool>
MultiMapImpl::put(const serialization::pimpl::data& key,
                  const serialization::pimpl::data& value)
{
    auto request = protocol::codec::multimap_put_encode(
      get_name(), key, value, util::get_current_thread_id());
    return invoke_and_get_future<bool>(request, key);
}

boost::future<std::vector<serialization::pimpl::data>>
MultiMapImpl::get_data(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::multimap_get_encode(
      get_name(), key, util::get_current_thread_id());
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request, key);
}

boost::future<bool>
MultiMapImpl::remove(const serialization::pimpl::data& key,
                     const serialization::pimpl::data& value)
{
    auto request = protocol::codec::multimap_removeentry_encode(
      get_name(), key, value, util::get_current_thread_id());
    return invoke_and_get_future<bool>(request, key);
}

boost::future<std::vector<serialization::pimpl::data>>
MultiMapImpl::remove_data(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::multimap_remove_encode(
      get_name(), key, util::get_current_thread_id());
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request, key);
}

boost::future<std::vector<serialization::pimpl::data>>
MultiMapImpl::key_set_data()
{
    auto request = protocol::codec::multimap_keyset_encode(get_name());
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request);
}

boost::future<std::vector<serialization::pimpl::data>>
MultiMapImpl::values_data()
{
    auto request = protocol::codec::multimap_values_encode(get_name());
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request);
}

boost::future<EntryVector>
MultiMapImpl::entry_set_data()
{
    auto request = protocol::codec::multimap_entryset_encode(get_name());
    return invoke_and_get_future<EntryVector>(request);
}

boost::future<bool>
MultiMapImpl::contains_key(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::multimap_containskey_encode(
      get_name(), key, util::get_current_thread_id());
    return invoke_and_get_future<bool>(request, key);
}

boost::future<bool>
MultiMapImpl::contains_value(const serialization::pimpl::data& value)
{
    auto request =
      protocol::codec::multimap_containsvalue_encode(get_name(), value);
    return invoke_and_get_future<bool>(request);
}

boost::future<bool>
MultiMapImpl::contains_entry(const serialization::pimpl::data& key,
                             const serialization::pimpl::data& value)
{
    auto request = protocol::codec::multimap_containsentry_encode(
      get_name(), key, value, util::get_current_thread_id());
    return invoke_and_get_future<bool>(request, key);
}

boost::future<int>
MultiMapImpl::size()
{
    auto request = protocol::codec::multimap_size_encode(get_name());
    return invoke_and_get_future<int>(request);
}

boost::future<void>
MultiMapImpl::clear()
{
    auto request = protocol::codec::multimap_clear_encode(get_name());
    return to_void_future(invoke(request));
}

boost::future<int>
MultiMapImpl::value_count(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::multimap_valuecount_encode(
      get_name(), key, util::get_current_thread_id());
    return invoke_and_get_future<int>(request, key);
}

boost::future<boost::uuids::uuid>
MultiMapImpl::add_entry_listener(
  std::shared_ptr<impl::BaseEventHandler> entry_event_handler,
  bool include_value)
{
    return register_listener(
      create_multi_map_entry_listener_codec(include_value),
      std::move(entry_event_handler));
}

boost::future<boost::uuids::uuid>
MultiMapImpl::add_entry_listener(
  std::shared_ptr<impl::BaseEventHandler> entry_event_handler,
  bool include_value,
  serialization::pimpl::data&& key)
{
    return register_listener(
      create_multi_map_entry_listener_codec(include_value, std::move(key)),
      std::move(entry_event_handler));
}

boost::future<bool>
MultiMapImpl::remove_entry_listener(boost::uuids::uuid registration_id)
{
    return get_context().get_client_listener_service().deregister_listener(
      registration_id);
}

boost::future<void>
MultiMapImpl::lock(const serialization::pimpl::data& key)
{
    return lock(key, std::chrono::milliseconds(-1));
}

boost::future<void>
MultiMapImpl::lock(const serialization::pimpl::data& key,
                   std::chrono::milliseconds lease_time)
{
    auto request = protocol::codec::multimap_lock_encode(
      get_name(),
      key,
      util::get_current_thread_id(),
      std::chrono::duration_cast<std::chrono::milliseconds>(lease_time).count(),
      lock_reference_id_generator_->get_next_reference_id());
    return to_void_future(invoke_on_partition(request, get_partition_id(key)));
}

boost::future<bool>
MultiMapImpl::is_locked(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::multimap_islocked_encode(get_name(), key);
    return invoke_and_get_future<bool>(request, key);
}

boost::future<bool>
MultiMapImpl::try_lock(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::multimap_trylock_encode(
      get_name(),
      key,
      util::get_current_thread_id(),
      INT64_MAX,
      0,
      lock_reference_id_generator_->get_next_reference_id());
    return invoke_and_get_future<bool>(request, key);
}

boost::future<bool>
MultiMapImpl::try_lock(const serialization::pimpl::data& key,
                       std::chrono::milliseconds timeout)
{
    return try_lock(key, timeout, std::chrono::milliseconds(INT64_MAX));
}

boost::future<bool>
MultiMapImpl::try_lock(const serialization::pimpl::data& key,
                       std::chrono::milliseconds timeout,
                       std::chrono::milliseconds lease_time)
{
    auto request = protocol::codec::multimap_trylock_encode(
      get_name(),
      key,
      util::get_current_thread_id(),
      std::chrono::duration_cast<std::chrono::milliseconds>(lease_time).count(),
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(),
      lock_reference_id_generator_->get_next_reference_id());
    return invoke_and_get_future<bool>(request, key);
}

boost::future<void>
MultiMapImpl::unlock(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::multimap_unlock_encode(
      get_name(),
      key,
      util::get_current_thread_id(),
      lock_reference_id_generator_->get_next_reference_id());
    return to_void_future(invoke_on_partition(request, get_partition_id(key)));
}

boost::future<void>
MultiMapImpl::force_unlock(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::multimap_forceunlock_encode(
      get_name(), key, lock_reference_id_generator_->get_next_reference_id());
    return to_void_future(invoke_on_partition(request, get_partition_id(key)));
}

std::shared_ptr<spi::impl::ListenerMessageCodec>
MultiMapImpl::create_multi_map_entry_listener_codec(bool include_value)
{
    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
      new MultiMapEntryListenerMessageCodec(get_name(), include_value));
}

std::shared_ptr<spi::impl::ListenerMessageCodec>
MultiMapImpl::create_multi_map_entry_listener_codec(
  bool include_value,
  serialization::pimpl::data&& key)
{
    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
      new MultiMapEntryListenerToKeyCodec(
        get_name(), include_value, std::move(key)));
}

void
MultiMapImpl::on_initialize()
{
    ProxyImpl::on_initialize();
    lock_reference_id_generator_ =
      get_context().get_lock_reference_id_generator();
}

MultiMapImpl::MultiMapEntryListenerMessageCodec::
  MultiMapEntryListenerMessageCodec(std::string name, bool include_value)
  : name_(std::move(name))
  , include_value_(include_value)
{}

protocol::ClientMessage
MultiMapImpl::MultiMapEntryListenerMessageCodec::encode_add_request(
  bool local_only) const
{
    return protocol::codec::multimap_addentrylistener_encode(
      name_, include_value_, local_only);
}

protocol::ClientMessage
MultiMapImpl::MultiMapEntryListenerMessageCodec::encode_remove_request(
  boost::uuids::uuid real_registration_id) const
{
    return protocol::codec::multimap_removeentrylistener_encode(
      name_, real_registration_id);
}

protocol::ClientMessage
MultiMapImpl::MultiMapEntryListenerToKeyCodec::encode_add_request(
  bool local_only) const
{
    return protocol::codec::multimap_addentrylistenertokey_encode(
      name_, key_, include_value_, local_only);
}

protocol::ClientMessage
MultiMapImpl::MultiMapEntryListenerToKeyCodec::encode_remove_request(
  boost::uuids::uuid real_registration_id) const
{
    return protocol::codec::multimap_removeentrylistener_encode(
      name_, real_registration_id);
}

MultiMapImpl::MultiMapEntryListenerToKeyCodec::MultiMapEntryListenerToKeyCodec(
  std::string name,
  bool include_value,
  serialization::pimpl::data&& key)
  : name_(std::move(name))
  , include_value_(include_value)
  , key_(std::move(key))
{}

const std::shared_ptr<std::unordered_set<member>>
  PNCounterImpl::EMPTY_ADDRESS_LIST(new std::unordered_set<member>());

PNCounterImpl::PNCounterImpl(const std::string& service_name,
                             const std::string& object_name,
                             spi::ClientContext* context)
  : ProxyImpl(service_name, object_name, context)
  , max_configured_replica_count_(0)
  , observed_clock_(
      std::shared_ptr<impl::vector_clock>(new impl::vector_clock()))
  , logger_(context->get_logger())
{}

std::ostream&
operator<<(std::ostream& os, const PNCounterImpl& proxy)
{
    os << "PNCounter{name='" << proxy.get_name() << "\'}";
    return os;
}

boost::future<int64_t>
PNCounterImpl::get()
{
    boost::shared_ptr<member> target =
      get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
    if (!target) {
        BOOST_THROW_EXCEPTION(exception::no_data_member_in_cluster(
          "ClientPNCounterProxy::get",
          "Cannot invoke operations on a CRDT because the cluster does not "
          "contain any data members"));
    }
    return invoke_get_internal(EMPTY_ADDRESS_LIST, nullptr, target);
}

boost::future<int64_t>
PNCounterImpl::get_and_add(int64_t delta)
{
    boost::shared_ptr<member> target =
      get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
    if (!target) {
        BOOST_THROW_EXCEPTION(exception::no_data_member_in_cluster(
          "ClientPNCounterProxy::getAndAdd",
          "Cannot invoke operations on a CRDT because the cluster does not "
          "contain any data members"));
    }
    return invoke_add_internal(
      delta, true, EMPTY_ADDRESS_LIST, nullptr, target);
}

boost::future<int64_t>
PNCounterImpl::add_and_get(int64_t delta)
{
    boost::shared_ptr<member> target =
      get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
    if (!target) {
        BOOST_THROW_EXCEPTION(exception::no_data_member_in_cluster(
          "ClientPNCounterProxy::addAndGet",
          "Cannot invoke operations on a CRDT because the cluster does not "
          "contain any data members"));
    }
    return invoke_add_internal(
      delta, false, EMPTY_ADDRESS_LIST, nullptr, target);
}

boost::future<int64_t>
PNCounterImpl::get_and_subtract(int64_t delta)
{
    boost::shared_ptr<member> target =
      get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
    if (!target) {
        BOOST_THROW_EXCEPTION(exception::no_data_member_in_cluster(
          "ClientPNCounterProxy::getAndSubtract",
          "Cannot invoke operations on a CRDT because the cluster does not "
          "contain any data members"));
    }
    return invoke_add_internal(
      -delta, true, EMPTY_ADDRESS_LIST, nullptr, target);
}

boost::future<int64_t>
PNCounterImpl::subtract_and_get(int64_t delta)
{
    boost::shared_ptr<member> target =
      get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
    if (!target) {
        BOOST_THROW_EXCEPTION(exception::no_data_member_in_cluster(
          "ClientPNCounterProxy::subtractAndGet",
          "Cannot invoke operations on a CRDT because the cluster does not "
          "contain any data members"));
    }
    return invoke_add_internal(
      -delta, false, EMPTY_ADDRESS_LIST, nullptr, target);
}

boost::future<int64_t>
PNCounterImpl::decrement_and_get()
{
    boost::shared_ptr<member> target =
      get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
    if (!target) {
        BOOST_THROW_EXCEPTION(exception::no_data_member_in_cluster(
          "ClientPNCounterProxy::decrementAndGet",
          "Cannot invoke operations on a CRDT because the cluster does not "
          "contain any data members"));
    }
    return invoke_add_internal(-1, false, EMPTY_ADDRESS_LIST, nullptr, target);
}

boost::future<int64_t>
PNCounterImpl::increment_and_get()
{
    boost::shared_ptr<member> target =
      get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
    if (!target) {
        BOOST_THROW_EXCEPTION(exception::no_data_member_in_cluster(
          "ClientPNCounterProxy::incrementAndGet",
          "Cannot invoke operations on a CRDT because the cluster does not "
          "contain any data members"));
    }
    return invoke_add_internal(1, false, EMPTY_ADDRESS_LIST, nullptr, target);
}

boost::future<int64_t>
PNCounterImpl::get_and_decrement()
{
    boost::shared_ptr<member> target =
      get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
    if (!target) {
        BOOST_THROW_EXCEPTION(exception::no_data_member_in_cluster(
          "ClientPNCounterProxy::getAndDecrement",
          "Cannot invoke operations on a CRDT because the cluster does not "
          "contain any data members"));
    }
    return invoke_add_internal(-1, true, EMPTY_ADDRESS_LIST, nullptr, target);
}

boost::future<int64_t>
PNCounterImpl::get_and_increment()
{
    boost::shared_ptr<member> target =
      get_crdt_operation_target(*EMPTY_ADDRESS_LIST);
    if (!target) {
        BOOST_THROW_EXCEPTION(exception::no_data_member_in_cluster(
          "ClientPNCounterProxy::getAndIncrement",
          "Cannot invoke operations on a CRDT because the cluster does not "
          "contain any data members"));
    }
    return invoke_add_internal(1, true, EMPTY_ADDRESS_LIST, nullptr, target);
}

boost::future<void>
PNCounterImpl::reset()
{
    observed_clock_ =
      std::shared_ptr<impl::vector_clock>(new impl::vector_clock());
    return boost::make_ready_future();
}

boost::shared_ptr<member>
PNCounterImpl::get_crdt_operation_target(
  const std::unordered_set<member>& excluded_addresses)
{
    auto replicaAddress = current_target_replica_address_.load();
    if (replicaAddress &&
        excluded_addresses.find(*replicaAddress) == excluded_addresses.end()) {
        return replicaAddress;
    }

    {
        std::lock_guard<std::mutex> guard(target_selection_mutex_);
        replicaAddress = current_target_replica_address_.load();
        if (!replicaAddress || excluded_addresses.find(*replicaAddress) !=
                                 excluded_addresses.end()) {
            current_target_replica_address_ =
              choose_target_replica(excluded_addresses);
        }
    }
    return current_target_replica_address_;
}

boost::shared_ptr<member>
PNCounterImpl::choose_target_replica(
  const std::unordered_set<member>& excluded_addresses)
{
    std::vector<member> replicaAddresses =
      get_replica_addresses(excluded_addresses);
    if (replicaAddresses.empty()) {
        return nullptr;
    }
    // TODO: Use a random generator as used in Java (ThreadLocalRandomProvider)
    // which is per thread
    int randomReplicaIndex = std::abs(rand()) % (int)replicaAddresses.size();
    return boost::make_shared<member>(replicaAddresses[randomReplicaIndex]);
}

std::vector<member>
PNCounterImpl::get_replica_addresses(
  const std::unordered_set<member>& excluded_members)
{
    std::vector<member> dataMembers =
      get_context().get_client_cluster_service().get_members(
        *member_selectors::DATA_MEMBER_SELECTOR);
    int32_t replicaCount = get_max_configured_replica_count();
    int currentReplicaCount =
      util::min<int>(replicaCount, (int)dataMembers.size());

    std::vector<member> replicaMembers;
    for (int i = 0; i < currentReplicaCount; i++) {
        if (excluded_members.find(dataMembers[i]) == excluded_members.end()) {
            replicaMembers.push_back(dataMembers[i]);
        }
    }
    return replicaMembers;
}

int32_t
PNCounterImpl::get_max_configured_replica_count()
{
    if (max_configured_replica_count_ > 0) {
        return max_configured_replica_count_;
    } else {
        auto request =
          protocol::codec::pncounter_getconfiguredreplicacount_encode(
            get_name());
        max_configured_replica_count_ =
          invoke_and_get_future<int32_t>(request).get();
    }
    return max_configured_replica_count_;
}

boost::shared_ptr<member>
PNCounterImpl::try_choose_a_new_target(
  std::shared_ptr<std::unordered_set<member>> excluded_addresses,
  boost::shared_ptr<member> last_target,
  const exception::hazelcast_& last_exception)
{
    HZ_LOG(
      logger_,
      finest,
      boost::str(boost::format(
                   "Exception occurred while invoking operation on target %1%, "
                   "choosing different target. Cause: %2%") %
                 last_target % last_exception));
    if (excluded_addresses == EMPTY_ADDRESS_LIST) {
        // TODO: Make sure that this only affects the local variable of the
        // method
        excluded_addresses = std::make_shared<std::unordered_set<member>>();
    }
    excluded_addresses->insert(*last_target);
    return get_crdt_operation_target(*excluded_addresses);
}

boost::future<int64_t>
PNCounterImpl::invoke_get_internal(
  std::shared_ptr<std::unordered_set<member>> excluded_addresses,
  std::exception_ptr last_exception,
  const boost::shared_ptr<member>& target)
{
    if (!target) {
        if (last_exception) {
            std::rethrow_exception(last_exception);
        } else {
            BOOST_THROW_EXCEPTION(exception::no_data_member_in_cluster(
              "ClientPNCounterProxy::invokeGetInternal",
              "Cannot invoke operations on a CRDT because the cluster does not "
              "contain any data members"));
        }
    }
    try {
        auto timestamps = observed_clock_.get()->entry_set();
        auto request = protocol::codec::pncounter_get_encode(
          get_name(), timestamps, target->get_uuid());
        return invoke_on_member(request, target->get_uuid())
          .then(
            boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
                try {
                    return get_and_update_timestamps(std::move(f));
                } catch (exception::hazelcast_& e) {
                    return invoke_get_internal(excluded_addresses,
                                               std::current_exception(),
                                               try_choose_a_new_target(
                                                 excluded_addresses, target, e))
                      .get();
                }
            });
    } catch (exception::hazelcast_& e) {
        return invoke_get_internal(
          excluded_addresses,
          std::current_exception(),
          try_choose_a_new_target(excluded_addresses, target, e));
    }
}

boost::future<int64_t>
PNCounterImpl::invoke_add_internal(
  int64_t delta,
  bool getBeforeUpdate,
  std::shared_ptr<std::unordered_set<member>> excluded_addresses,
  std::exception_ptr last_exception,
  const boost::shared_ptr<member>& target)
{
    if (!target) {
        if (last_exception) {
            std::rethrow_exception(last_exception);
        } else {
            BOOST_THROW_EXCEPTION(exception::no_data_member_in_cluster(
              "ClientPNCounterProxy::invokeGetInternal",
              "Cannot invoke operations on a CRDT because the cluster does not "
              "contain any data members"));
        }
    }

    try {
        auto request = protocol::codec::pncounter_add_encode(
          get_name(),
          delta,
          getBeforeUpdate,
          observed_clock_.get()->entry_set(),
          target->get_uuid());
        return invoke_on_member(request, target->get_uuid())
          .then(
            boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
                try {
                    return get_and_update_timestamps(std::move(f));
                } catch (exception::hazelcast_& e) {
                    return invoke_add_internal(delta,
                                               getBeforeUpdate,
                                               excluded_addresses,
                                               std::current_exception(),
                                               try_choose_a_new_target(
                                                 excluded_addresses, target, e))
                      .get();
                }
            });
    } catch (exception::hazelcast_& e) {
        return invoke_add_internal(
          delta,
          getBeforeUpdate,
          excluded_addresses,
          std::current_exception(),
          try_choose_a_new_target(excluded_addresses, target, e));
    }
}

int64_t
PNCounterImpl::get_and_update_timestamps(
  boost::future<protocol::ClientMessage> f)
{
    auto msg = f.get();
    auto value = msg.get_first_fixed_sized_field<int64_t>();
    // skip replica count
    msg.get<int32_t>();
    update_observed_replica_timestamps(
      msg.get<impl::vector_clock::timestamp_vector>());
    return value;
}

void
PNCounterImpl::update_observed_replica_timestamps(
  const impl::vector_clock::timestamp_vector& received_logical_timestamps)
{
    std::shared_ptr<impl::vector_clock> received =
      to_vector_clock(received_logical_timestamps);
    for (;;) {
        std::shared_ptr<impl::vector_clock> currentClock =
          this->observed_clock_;
        if (currentClock->is_after(*received)) {
            break;
        }
        if (observed_clock_.compare_and_set(currentClock, received)) {
            break;
        }
    }
}

std::shared_ptr<impl::vector_clock>
PNCounterImpl::to_vector_clock(
  const impl::vector_clock::timestamp_vector& replica_logical_timestamps)
{
    return std::shared_ptr<impl::vector_clock>(
      new impl::vector_clock(replica_logical_timestamps));
}

boost::shared_ptr<member>
PNCounterImpl::get_current_target_replica_address()
{
    return current_target_replica_address_.load();
}

IListImpl::IListImpl(const std::string& instance_name,
                     spi::ClientContext* context)
  : ProxyImpl("hz:impl:listService", instance_name, context)
{
    serialization::pimpl::data key_data =
      get_context().get_serialization_service().to_data<std::string>(
        &instance_name);
    partition_id_ = get_partition_id(key_data);
}

boost::future<bool>
IListImpl::remove_item_listener(boost::uuids::uuid registration_id)
{
    return get_context().get_client_listener_service().deregister_listener(
      registration_id);
}

boost::future<int>
IListImpl::size()
{
    auto request = protocol::codec::list_size_encode(get_name());
    return invoke_and_get_future<int>(request, partition_id_);
}

boost::future<bool>
IListImpl::is_empty()
{
    auto request = protocol::codec::list_isempty_encode(get_name());
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
IListImpl::contains(const serialization::pimpl::data& element)
{
    auto request = protocol::codec::list_contains_encode(get_name(), element);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<std::vector<serialization::pimpl::data>>
IListImpl::to_array_data()
{
    auto request = protocol::codec::list_getall_encode(get_name());
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request, partition_id_);
}

boost::future<bool>
IListImpl::add(const serialization::pimpl::data& element)
{
    auto request = protocol::codec::list_add_encode(get_name(), element);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
IListImpl::remove(const serialization::pimpl::data& element)
{
    auto request = protocol::codec::list_remove_encode(get_name(), element);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
IListImpl::contains_all_data(
  const std::vector<serialization::pimpl::data>& elements)
{
    auto request =
      protocol::codec::list_containsall_encode(get_name(), elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
IListImpl::add_all_data(const std::vector<serialization::pimpl::data>& elements)
{
    auto request = protocol::codec::list_addall_encode(get_name(), elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
IListImpl::add_all_data(int index,
                        const std::vector<serialization::pimpl::data>& elements)
{
    auto request =
      protocol::codec::list_addallwithindex_encode(get_name(), index, elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
IListImpl::remove_all_data(
  const std::vector<serialization::pimpl::data>& elements)
{
    auto request =
      protocol::codec::list_compareandremoveall_encode(get_name(), elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
IListImpl::retain_all_data(
  const std::vector<serialization::pimpl::data>& elements)
{
    auto request =
      protocol::codec::list_compareandretainall_encode(get_name(), elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<void>
IListImpl::clear()
{
    auto request = protocol::codec::list_clear_encode(get_name());
    return to_void_future(invoke_on_partition(request, partition_id_));
}

boost::future<boost::optional<serialization::pimpl::data>>
IListImpl::get_data(int index)
{
    auto request = protocol::codec::list_get_encode(get_name(), index);
    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request, partition_id_);
}

boost::future<boost::optional<serialization::pimpl::data>>
IListImpl::set_data(int index, const serialization::pimpl::data& element)
{
    auto request = protocol::codec::list_set_encode(get_name(), index, element);
    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request, partition_id_);
}

boost::future<void>
IListImpl::add(int index, const serialization::pimpl::data& element)
{
    auto request =
      protocol::codec::list_addwithindex_encode(get_name(), index, element);
    return to_void_future(invoke_on_partition(request, partition_id_));
}

boost::future<boost::optional<serialization::pimpl::data>>
IListImpl::remove_data(int index)
{
    auto request =
      protocol::codec::list_removewithindex_encode(get_name(), index);
    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request, partition_id_);
}

boost::future<int>
IListImpl::index_of(const serialization::pimpl::data& element)
{
    auto request = protocol::codec::list_indexof_encode(get_name(), element);
    return invoke_and_get_future<int>(request, partition_id_);
}

boost::future<int>
IListImpl::last_index_of(const serialization::pimpl::data& element)
{
    auto request =
      protocol::codec::list_lastindexof_encode(get_name(), element);
    return invoke_and_get_future<int>(request, partition_id_);
}

boost::future<std::vector<serialization::pimpl::data>>
IListImpl::sub_list_data(int from_index, int to_index)
{
    auto request =
      protocol::codec::list_sub_encode(get_name(), from_index, to_index);
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request, partition_id_);
}

std::shared_ptr<spi::impl::ListenerMessageCodec>
IListImpl::create_item_listener_codec(bool include_value)
{
    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
      new ListListenerMessageCodec(get_name(), include_value));
}

IListImpl::ListListenerMessageCodec::ListListenerMessageCodec(
  std::string name,
  bool include_value)
  : name_(std::move(name))
  , include_value_(include_value)
{}

protocol::ClientMessage
IListImpl::ListListenerMessageCodec::encode_add_request(bool local_only) const
{
    return protocol::codec::list_addlistener_encode(
      name_, include_value_, local_only);
}

protocol::ClientMessage
IListImpl::ListListenerMessageCodec::encode_remove_request(
  boost::uuids::uuid real_registration_id) const
{
    return protocol::codec::list_removelistener_encode(name_,
                                                       real_registration_id);
}

flake_id_generator_impl::Block::Block(IdBatch&& id_batch,
                                      std::chrono::milliseconds validity)
  : id_batch_(id_batch)
  , invalid_since_(std::chrono::steady_clock::now() + validity)
  , num_returned_(0)
{}

int64_t
flake_id_generator_impl::Block::next()
{
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

flake_id_generator_impl::IdBatch::IdIterator
  flake_id_generator_impl::IdBatch::endOfBatch;

int64_t
flake_id_generator_impl::IdBatch::get_base() const
{
    return base_;
}

int64_t
flake_id_generator_impl::IdBatch::get_increment() const
{
    return increment_;
}

int32_t
flake_id_generator_impl::IdBatch::get_batch_size() const
{
    return batch_size_;
}

flake_id_generator_impl::IdBatch::IdBatch(int64_t base,
                                          int64_t increment,
                                          int32_t batch_size)
  : base_(base)
  , increment_(increment)
  , batch_size_(batch_size)
{}

flake_id_generator_impl::IdBatch::IdIterator&
flake_id_generator_impl::IdBatch::end()
{
    return endOfBatch;
}

flake_id_generator_impl::IdBatch::IdIterator
flake_id_generator_impl::IdBatch::iterator()
{
    return flake_id_generator_impl::IdBatch::IdIterator(
      base_, increment_, batch_size_);
}

flake_id_generator_impl::IdBatch::IdIterator::IdIterator(
  int64_t base2,
  const int64_t increment,
  int32_t remaining)
  : base2_(base2)
  , increment_(increment)
  , remaining_(remaining)
{}

bool
flake_id_generator_impl::IdBatch::IdIterator::operator==(
  const flake_id_generator_impl::IdBatch::IdIterator& rhs) const
{
    return base2_ == rhs.base2_ && increment_ == rhs.increment_ &&
           remaining_ == rhs.remaining_;
}

bool
flake_id_generator_impl::IdBatch::IdIterator::operator!=(
  const flake_id_generator_impl::IdBatch::IdIterator& rhs) const
{
    return !(rhs == *this);
}

flake_id_generator_impl::IdBatch::IdIterator::IdIterator()
  : base2_(-1)
  , increment_(-1)
  , remaining_(-1)
{}

flake_id_generator_impl::IdBatch::IdIterator&
flake_id_generator_impl::IdBatch::IdIterator::operator++()
{
    if (remaining_ == 0) {
        return flake_id_generator_impl::IdBatch::end();
    }

    --remaining_;

    base2_ += increment_;

    return *this;
}

flake_id_generator_impl::flake_id_generator_impl(
  const std::string& service_name,
  const std::string& object_name,
  spi::ClientContext* context)
  : ProxyImpl(service_name, object_name, context)
  , block_(nullptr)
{
    auto config =
      context->get_client_config().find_flake_id_generator_config(object_name);
    batch_size_ = config->get_prefetch_count();
    validity_ = config->get_prefetch_validity_duration();
}

int64_t
flake_id_generator_impl::new_id_internal()
{
    auto b = block_.load();
    if (b) {
        int64_t res = b->next();
        if (res != INT64_MIN) {
            return res;
        }
    }

    throw std::overflow_error("");
}

boost::future<int64_t>
flake_id_generator_impl::new_id()
{
    try {
        return boost::make_ready_future(new_id_internal());
    } catch (std::overflow_error&) {
        return new_id_batch(batch_size_)
          .then(boost::launch::sync,
                [=](boost::future<flake_id_generator_impl::IdBatch> f) {
                    auto newBlock =
                      boost::make_shared<Block>(f.get(), validity_);
                    auto value = newBlock->next();
                    auto b = block_.load();
                    block_.compare_exchange_strong(b, newBlock);
                    return value;
                });
    }
}

boost::future<flake_id_generator_impl::IdBatch>
flake_id_generator_impl::new_id_batch(int32_t size)
{
    auto request =
      protocol::codec::flakeidgenerator_newidbatch_encode(get_name(), size);
    return invoke(request).then(
      boost::launch::sync, [](boost::future<protocol::ClientMessage> f) {
          auto msg = f.get();
          msg.rd_ptr(protocol::ClientMessage::RESPONSE_HEADER_LEN);

          auto base = msg.get<int64_t>();
          auto increment = msg.get<int64_t>();
          auto batch_size = msg.get<int32_t>();
          return flake_id_generator_impl::IdBatch(base, increment, batch_size);
      });
}

IQueueImpl::IQueueImpl(const std::string& instance_name,
                       spi::ClientContext* context)
  : ProxyImpl("hz:impl:queueService", instance_name, context)
{
    serialization::pimpl::data data =
      get_context().get_serialization_service().to_data<std::string>(
        &instance_name);
    partition_id_ = get_partition_id(data);
}

boost::future<bool>
IQueueImpl::remove_item_listener(boost::uuids::uuid registration_id)
{
    return get_context().get_client_listener_service().deregister_listener(
      registration_id);
}

boost::future<bool>
IQueueImpl::offer(const serialization::pimpl::data& element,
                  std::chrono::milliseconds timeout)
{
    auto request = protocol::codec::queue_offer_encode(
      get_name(),
      element,
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<void>
IQueueImpl::put(const serialization::pimpl::data& element)
{
    auto request = protocol::codec::queue_put_encode(get_name(), element);
    return to_void_future(invoke_on_partition(request, partition_id_));
}

boost::future<boost::optional<serialization::pimpl::data>>
IQueueImpl::poll_data(std::chrono::milliseconds timeout)
{
    auto request = protocol::codec::queue_poll_encode(
      get_name(),
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request, partition_id_);
}

boost::future<int>
IQueueImpl::remaining_capacity()
{
    auto request = protocol::codec::queue_remainingcapacity_encode(get_name());
    return invoke_and_get_future<int>(request, partition_id_);
}

boost::future<bool>
IQueueImpl::remove(const serialization::pimpl::data& element)
{
    auto request = protocol::codec::queue_remove_encode(get_name(), element);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
IQueueImpl::contains(const serialization::pimpl::data& element)
{
    auto request = protocol::codec::queue_contains_encode(get_name(), element);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<std::vector<serialization::pimpl::data>>
IQueueImpl::drain_to_data(size_t max_elements)
{
    auto request = protocol::codec::queue_draintomaxsize_encode(
      get_name(), (int32_t)max_elements);

    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request, partition_id_);
}

boost::future<std::vector<serialization::pimpl::data>>
IQueueImpl::drain_to_data()
{
    auto request = protocol::codec::queue_drainto_encode(get_name());
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request, partition_id_);
}

boost::future<boost::optional<serialization::pimpl::data>>
IQueueImpl::take_data()
{
    auto request = protocol::codec::queue_take_encode(get_name());
    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request, partition_id_);
}

boost::future<boost::optional<serialization::pimpl::data>>
IQueueImpl::peek_data()
{
    auto request = protocol::codec::queue_peek_encode(get_name());
    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request, partition_id_);
}

boost::future<int>
IQueueImpl::size()
{
    auto request = protocol::codec::queue_size_encode(get_name());
    return invoke_and_get_future<int>(request, partition_id_);
}

boost::future<bool>
IQueueImpl::is_empty()
{
    auto request = protocol::codec::queue_isempty_encode(get_name());
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<std::vector<serialization::pimpl::data>>
IQueueImpl::to_array_data()
{
    auto request = protocol::codec::queue_iterator_encode(get_name());
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request, partition_id_);
}

boost::future<bool>
IQueueImpl::contains_all_data(
  const std::vector<serialization::pimpl::data>& elements)
{
    auto request =
      protocol::codec::queue_containsall_encode(get_name(), elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
IQueueImpl::add_all_data(
  const std::vector<serialization::pimpl::data>& elements)
{
    auto request = protocol::codec::queue_addall_encode(get_name(), elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
IQueueImpl::remove_all_data(
  const std::vector<serialization::pimpl::data>& elements)
{
    auto request =
      protocol::codec::queue_compareandremoveall_encode(get_name(), elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
IQueueImpl::retain_all_data(
  const std::vector<serialization::pimpl::data>& elements)
{
    auto request =
      protocol::codec::queue_compareandretainall_encode(get_name(), elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<void>
IQueueImpl::clear()
{
    auto request = protocol::codec::queue_clear_encode(get_name());
    return to_void_future(invoke_on_partition(request, partition_id_));
}

std::shared_ptr<spi::impl::ListenerMessageCodec>
IQueueImpl::create_item_listener_codec(bool include_value)
{
    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
      new QueueListenerMessageCodec(get_name(), include_value));
}

IQueueImpl::QueueListenerMessageCodec::QueueListenerMessageCodec(
  std::string name,
  bool include_value)
  : name_(std::move(name))
  , include_value_(include_value)
{}

protocol::ClientMessage
IQueueImpl::QueueListenerMessageCodec::encode_add_request(bool local_only) const
{
    return protocol::codec::queue_addlistener_encode(
      name_, include_value_, local_only);
}

protocol::ClientMessage
IQueueImpl::QueueListenerMessageCodec::encode_remove_request(
  boost::uuids::uuid real_registration_id) const
{
    return protocol::codec::queue_removelistener_encode(name_,
                                                        real_registration_id);
}

ProxyImpl::ProxyImpl(const std::string& service_name,
                     const std::string& object_name,
                     spi::ClientContext* context)
  : ClientProxy(object_name, service_name, *context)
  , SerializingProxy(*context, object_name)
{}

ProxyImpl::~ProxyImpl() = default;

SerializingProxy::SerializingProxy(spi::ClientContext& context,
                                   const std::string& object_name)
  : serialization_service_(context.get_serialization_service())
  , partition_service_(context.get_partition_service())
  , object_name_(object_name)
  , client_context_(context)
{}

int
SerializingProxy::get_partition_id(const serialization::pimpl::data& key)
{
    return partition_service_.get_partition_id(key);
}

boost::future<protocol::ClientMessage>
SerializingProxy::invoke_on_partition(protocol::ClientMessage& request,
                                      int partition_id)
{
    try {
        return spi::impl::ClientInvocation::create(
                 client_context_,
                 std::make_shared<protocol::ClientMessage>(std::move(request)),
                 object_name_,
                 partition_id)
          ->invoke();
    } catch (exception::iexception&) {
        util::exception_util::rethrow(std::current_exception());
        return boost::make_ready_future(protocol::ClientMessage(0));
    }
}

boost::future<protocol::ClientMessage>
SerializingProxy::invoke(protocol::ClientMessage& request)
{
    try {
        return spi::impl::ClientInvocation::create(
                 client_context_,
                 std::make_shared<protocol::ClientMessage>(std::move(request)),
                 object_name_)
          ->invoke();
    } catch (exception::iexception&) {
        util::exception_util::rethrow(std::current_exception());
        return boost::make_ready_future(protocol::ClientMessage(0));
    }
}

boost::future<protocol::ClientMessage>
SerializingProxy::invoke_on_connection(
  protocol::ClientMessage& request,
  std::shared_ptr<connection::Connection> connection)
{
    try {
        return spi::impl::ClientInvocation::create(
                 client_context_,
                 std::make_shared<protocol::ClientMessage>(std::move(request)),
                 object_name_,
                 connection)
          ->invoke();
    } catch (exception::iexception&) {
        util::exception_util::rethrow(std::current_exception());
        return boost::make_ready_future(protocol::ClientMessage(0));
    }
}

boost::future<protocol::ClientMessage>
SerializingProxy::invoke_on_key_owner(
  protocol::ClientMessage& request,
  const serialization::pimpl::data& key_data)
{
    try {
        return invoke_on_partition(request, get_partition_id(key_data));
    } catch (exception::iexception&) {
        util::exception_util::rethrow(std::current_exception());
        return boost::make_ready_future(protocol::ClientMessage(0));
    }
}

boost::future<protocol::ClientMessage>
SerializingProxy::invoke_on_member(protocol::ClientMessage& request,
                                   boost::uuids::uuid uuid)
{
    try {
        auto invocation = spi::impl::ClientInvocation::create(
          client_context_,
          std::make_shared<protocol::ClientMessage>(std::move(request)),
          object_name_,
          uuid);
        return invocation->invoke();
    } catch (exception::iexception&) {
        util::exception_util::rethrow(std::current_exception());
        return boost::make_ready_future(protocol::ClientMessage(0));
    }
}

template<>
boost::future<boost::optional<serialization::pimpl::data>>
SerializingProxy::invoke_and_get_future(protocol::ClientMessage& request)
{
    return decode_optional_var_sized<serialization::pimpl::data>(
      invoke(request));
}

template<>
boost::future<boost::optional<map::data_entry_view>>
SerializingProxy::invoke_and_get_future(protocol::ClientMessage& request,
                                        const serialization::pimpl::data& key)
{
    return decode_optional_var_sized<map::data_entry_view>(
      invoke_on_key_owner(request, key));
}

template<>
boost::future<boost::optional<serialization::pimpl::data>>
SerializingProxy::invoke_and_get_future(protocol::ClientMessage& request,
                                        int partition_id)
{
    return decode_optional_var_sized<serialization::pimpl::data>(
      invoke_on_partition(request, partition_id));
}

template<>
boost::future<boost::optional<serialization::pimpl::data>>
SerializingProxy::invoke_and_get_future(protocol::ClientMessage& request,
                                        const serialization::pimpl::data& key)
{
    return decode_optional_var_sized<serialization::pimpl::data>(
      invoke_on_key_owner(request, key));
}

PartitionSpecificClientProxy::PartitionSpecificClientProxy(
  const std::string& service_name,
  const std::string& object_name,
  spi::ClientContext* context)
  : ProxyImpl(service_name, object_name, context)
  , partition_id_(-1)
{}

void
PartitionSpecificClientProxy::on_initialize()
{
    std::string partitionKey = internal::partition::strategy::
      StringPartitioningStrategy::get_partition_key(name_);
    partition_id_ = get_context().get_partition_service().get_partition_id(
      to_data<std::string>(partitionKey));
}

IMapImpl::IMapImpl(const std::string& instance_name,
                   spi::ClientContext* context)
  : ProxyImpl("hz:impl:mapService", instance_name, context)
{}

boost::future<bool>
IMapImpl::contains_key(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::map_containskey_encode(
      get_name(), key, util::get_current_thread_id());
    return invoke_and_get_future<bool>(request, key);
}

boost::future<bool>
IMapImpl::contains_value(const serialization::pimpl::data& value)
{
    auto request = protocol::codec::map_containsvalue_encode(get_name(), value);
    return invoke_and_get_future<bool>(request);
}

boost::future<boost::optional<serialization::pimpl::data>>
IMapImpl::get_data(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::map_get_encode(
      get_name(), key, util::get_current_thread_id());
    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request, key);
}

boost::future<boost::optional<serialization::pimpl::data>>
IMapImpl::remove_data(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::map_remove_encode(
      get_name(), key, util::get_current_thread_id());
    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request, key);
}

boost::future<bool>
IMapImpl::remove(const serialization::pimpl::data& key,
                 const serialization::pimpl::data& value)
{
    auto request = protocol::codec::map_removeifsame_encode(
      get_name(), key, value, util::get_current_thread_id());
    return invoke_and_get_future<bool>(request, key);
}

boost::future<protocol::ClientMessage>
IMapImpl::remove_all(const serialization::pimpl::data& predicate_data)
{
    auto request =
      protocol::codec::map_removeall_encode(get_name(), predicate_data);
    return invoke(request);
}

boost::future<protocol::ClientMessage>
IMapImpl::delete_entry(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::map_delete_encode(
      get_name(), key, util::get_current_thread_id());
    return invoke_on_partition(request, get_partition_id(key));
}

boost::future<protocol::ClientMessage>
IMapImpl::flush()
{
    auto request = protocol::codec::map_flush_encode(get_name());
    return invoke(request);
}

boost::future<bool>
IMapImpl::try_remove(const serialization::pimpl::data& key,
                     std::chrono::milliseconds timeout)
{
    auto request = protocol::codec::map_tryremove_encode(
      get_name(),
      key,
      util::get_current_thread_id(),
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

    return invoke_and_get_future<bool>(request, key);
}

boost::future<bool>
IMapImpl::try_put(const serialization::pimpl::data& key,
                  const serialization::pimpl::data& value,
                  std::chrono::milliseconds timeout)
{
    auto request = protocol::codec::map_tryput_encode(
      get_name(),
      key,
      value,
      util::get_current_thread_id(),
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

    return invoke_and_get_future<bool>(request, key);
}

boost::future<boost::optional<serialization::pimpl::data>>
IMapImpl::put_data(const serialization::pimpl::data& key,
                   const serialization::pimpl::data& value,
                   std::chrono::milliseconds ttl)
{
    auto request = protocol::codec::map_put_encode(
      get_name(),
      key,
      value,
      util::get_current_thread_id(),
      std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request, key);
}

boost::future<protocol::ClientMessage>
IMapImpl::put_transient(const serialization::pimpl::data& key,
                        const serialization::pimpl::data& value,
                        std::chrono::milliseconds ttl)
{
    auto request = protocol::codec::map_puttransient_encode(
      get_name(),
      key,
      value,
      util::get_current_thread_id(),
      std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
    return invoke_on_partition(request, get_partition_id(key));
}

boost::future<boost::optional<serialization::pimpl::data>>
IMapImpl::put_if_absent_data(const serialization::pimpl::data& key,
                             const serialization::pimpl::data& value,
                             std::chrono::milliseconds ttl)
{
    auto request = protocol::codec::map_putifabsent_encode(
      get_name(),
      key,
      value,
      util::get_current_thread_id(),
      std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request, key);
}

boost::future<bool>
IMapImpl::replace(const serialization::pimpl::data& key,
                  const serialization::pimpl::data& old_value,
                  const serialization::pimpl::data& new_value)
{
    auto request = protocol::codec::map_replaceifsame_encode(
      get_name(), key, old_value, new_value, util::get_current_thread_id());

    return invoke_and_get_future<bool>(request, key);
}

boost::future<boost::optional<serialization::pimpl::data>>
IMapImpl::replace_data(const serialization::pimpl::data& key,
                       const serialization::pimpl::data& value)
{
    auto request = protocol::codec::map_replace_encode(
      get_name(), key, value, util::get_current_thread_id());

    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request, key);
}

boost::future<protocol::ClientMessage>
IMapImpl::set(const serialization::pimpl::data& key,
              const serialization::pimpl::data& value,
              std::chrono::milliseconds ttl)
{
    auto request = protocol::codec::map_set_encode(
      get_name(),
      key,
      value,
      util::get_current_thread_id(),
      std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
    return invoke_on_partition(request, get_partition_id(key));
}

boost::future<protocol::ClientMessage>
IMapImpl::lock(const serialization::pimpl::data& key)
{
    return lock(key, std::chrono::milliseconds(-1));
}

boost::future<protocol::ClientMessage>
IMapImpl::lock(const serialization::pimpl::data& key,
               std::chrono::milliseconds lease_time)
{
    auto request = protocol::codec::map_lock_encode(
      get_name(),
      key,
      util::get_current_thread_id(),
      std::chrono::duration_cast<std::chrono::milliseconds>(lease_time).count(),
      lock_reference_id_generator_->get_next_reference_id());
    return invoke_on_partition(request, get_partition_id(key));
}

boost::future<bool>
IMapImpl::is_locked(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::map_islocked_encode(get_name(), key);

    return invoke_and_get_future<bool>(request, key);
}

boost::future<bool>
IMapImpl::try_lock(const serialization::pimpl::data& key,
                   std::chrono::milliseconds timeout)
{
    return try_lock(key, timeout, std::chrono::milliseconds(-1));
}

boost::future<bool>
IMapImpl::try_lock(const serialization::pimpl::data& key,
                   std::chrono::milliseconds timeout,
                   std::chrono::milliseconds lease_time)
{
    auto request = protocol::codec::map_trylock_encode(
      get_name(),
      key,
      util::get_current_thread_id(),
      std::chrono::duration_cast<std::chrono::milliseconds>(lease_time).count(),
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(),
      lock_reference_id_generator_->get_next_reference_id());
    return invoke_and_get_future<bool>(request, key);
}

boost::future<protocol::ClientMessage>
IMapImpl::unlock(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::map_unlock_encode(
      get_name(),
      key,
      util::get_current_thread_id(),
      lock_reference_id_generator_->get_next_reference_id());
    return invoke_on_partition(request, get_partition_id(key));
}

boost::future<protocol::ClientMessage>
IMapImpl::force_unlock(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::map_forceunlock_encode(
      get_name(), key, lock_reference_id_generator_->get_next_reference_id());
    return invoke_on_partition(request, get_partition_id(key));
}

boost::future<std::string>
IMapImpl::add_interceptor(const serialization::pimpl::data& interceptor)
{
    auto request =
      protocol::codec::map_addinterceptor_encode(get_name(), interceptor);
    return invoke_and_get_future<std::string>(request);
}

// TODO: We can use generic template Listener instead of impl::BaseEventHandler
// to prevent the virtual function calls
boost::future<boost::uuids::uuid>
IMapImpl::add_entry_listener(
  std::shared_ptr<impl::BaseEventHandler> entry_event_handler,
  bool include_value,
  int32_t listener_flags)
{
    return register_listener(
      create_map_entry_listener_codec(include_value, listener_flags),
      std::move(entry_event_handler));
}

boost::future<boost::uuids::uuid>
IMapImpl::add_entry_listener(
  std::shared_ptr<impl::BaseEventHandler> entry_event_handler,
  serialization::pimpl::data&& predicate,
  bool include_value,
  int32_t listener_flags)
{
    return register_listener(
      create_map_entry_listener_codec(
        include_value, std::move(predicate), listener_flags),
      std::move(entry_event_handler));
}

boost::future<bool>
IMapImpl::remove_entry_listener(boost::uuids::uuid registration_id)
{
    return get_context().get_client_listener_service().deregister_listener(
      registration_id);
}

boost::future<boost::uuids::uuid>
IMapImpl::add_entry_listener(
  std::shared_ptr<impl::BaseEventHandler> entry_event_handler,
  bool include_value,
  serialization::pimpl::data&& key,
  int32_t listener_flags)
{
    return register_listener(create_map_entry_listener_codec(
                               include_value, listener_flags, std::move(key)),
                             std::move(entry_event_handler));
}

boost::future<boost::optional<map::data_entry_view>>
IMapImpl::get_entry_view_data(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::map_getentryview_encode(
      get_name(), key, util::get_current_thread_id());
    return invoke_and_get_future<boost::optional<map::data_entry_view>>(request,
                                                                        key);
}

boost::future<bool>
IMapImpl::evict(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::map_evict_encode(
      get_name(), key, util::get_current_thread_id());
    return invoke_and_get_future<bool>(request, key);
}

boost::future<protocol::ClientMessage>
IMapImpl::evict_all()
{
    auto request = protocol::codec::map_evictall_encode(get_name());
    return invoke(request);
}

boost::future<EntryVector>
IMapImpl::get_all_data(int partition_id,
                       const std::vector<serialization::pimpl::data>& keys)
{
    auto request = protocol::codec::map_getall_encode(get_name(), keys);
    return invoke_and_get_future<EntryVector>(request, partition_id);
}

boost::future<std::vector<serialization::pimpl::data>>
IMapImpl::key_set_data()
{
    auto request = protocol::codec::map_keyset_encode(get_name());
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request);
}

boost::future<std::vector<serialization::pimpl::data>>
IMapImpl::key_set_data(const serialization::pimpl::data& predicate)
{
    auto request =
      protocol::codec::map_keysetwithpredicate_encode(get_name(), predicate);
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request);
}

boost::future<
  std::pair<std::vector<serialization::pimpl::data>, query::anchor_data_list>>
IMapImpl::key_set_for_paging_predicate_data(
  protocol::codec::holder::paging_predicate_holder const& predicate)
{
    auto request = protocol::codec::map_keysetwithpagingpredicate_encode(
      get_name(), predicate);
    return invoke(request).then(
      boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
          return get_paging_predicate_response<
            std::vector<serialization::pimpl::data>>(std::move(f));
      });
}

boost::future<EntryVector>
IMapImpl::entry_set_data()
{
    auto request = protocol::codec::map_entryset_encode(get_name());
    return invoke_and_get_future<EntryVector>(request);
}

boost::future<EntryVector>
IMapImpl::entry_set_data(const serialization::pimpl::data& predicate)
{
    auto request =
      protocol::codec::map_entrieswithpredicate_encode(get_name(), predicate);
    return invoke_and_get_future<EntryVector>(request);
}

boost::future<std::pair<EntryVector, query::anchor_data_list>>
IMapImpl::entry_set_for_paging_predicate_data(
  protocol::codec::holder::paging_predicate_holder const& predicate)
{
    auto request = protocol::codec::map_entrieswithpagingpredicate_encode(
      get_name(), predicate);
    return invoke(request).then(
      boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
          return get_paging_predicate_response<EntryVector>(std::move(f));
      });
}

boost::future<std::vector<serialization::pimpl::data>>
IMapImpl::values_data()
{
    auto request = protocol::codec::map_values_encode(get_name());
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request);
}

boost::future<std::vector<serialization::pimpl::data>>
IMapImpl::values_data(const serialization::pimpl::data& predicate)
{
    auto request =
      protocol::codec::map_valueswithpredicate_encode(get_name(), predicate);
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request);
}

boost::future<
  std::pair<std::vector<serialization::pimpl::data>, query::anchor_data_list>>
IMapImpl::values_for_paging_predicate_data(
  protocol::codec::holder::paging_predicate_holder const& predicate)
{
    auto request = protocol::codec::map_valueswithpagingpredicate_encode(
      get_name(), predicate);
    return invoke(request).then(
      boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
          return get_paging_predicate_response<
            std::vector<serialization::pimpl::data>>(std::move(f));
      });
}

boost::future<protocol::ClientMessage>
IMapImpl::add_index_data(const config::index_config& config)
{
    auto request = protocol::codec::map_addindex_encode(get_name(), config);
    return invoke(request);
}

boost::future<int>
IMapImpl::size()
{
    auto request = protocol::codec::map_size_encode(get_name());
    return invoke_and_get_future<int>(request);
}

boost::future<bool>
IMapImpl::is_empty()
{
    auto request = protocol::codec::map_isempty_encode(get_name());
    return invoke_and_get_future<bool>(request);
}

boost::future<protocol::ClientMessage>
IMapImpl::put_all_data(int partition_id, const EntryVector& entries)
{
    auto request =
      protocol::codec::map_putall_encode(get_name(), entries, true);
    return invoke_on_partition(request, partition_id);
}

boost::future<protocol::ClientMessage>
IMapImpl::clear_data()
{
    auto request = protocol::codec::map_clear_encode(get_name());
    return invoke(request);
}

boost::future<boost::optional<serialization::pimpl::data>>
IMapImpl::execute_on_key_data(const serialization::pimpl::data& key,
                              const serialization::pimpl::data& processor)
{
    auto request = protocol::codec::map_executeonkey_encode(
      get_name(), processor, key, util::get_current_thread_id());
    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request, get_partition_id(key));
}

boost::future<boost::optional<serialization::pimpl::data>>
IMapImpl::submit_to_key_data(const serialization::pimpl::data& key,
                             const serialization::pimpl::data& processor)
{
    auto request = protocol::codec::map_submittokey_encode(
      get_name(), processor, key, util::get_current_thread_id());
    return invoke_on_partition(request, get_partition_id(key))
      .then(boost::launch::sync, [](boost::future<protocol::ClientMessage> f) {
          auto msg = f.get();
          msg.skip_frame();
          return msg.get_nullable<serialization::pimpl::data>();
      });
}

boost::future<EntryVector>
IMapImpl::execute_on_keys_data(
  const std::vector<serialization::pimpl::data>& keys,
  const serialization::pimpl::data& processor)
{
    auto request =
      protocol::codec::map_executeonkeys_encode(get_name(), processor, keys);
    return invoke_and_get_future<EntryVector>(request);
}

boost::future<protocol::ClientMessage>
IMapImpl::remove_interceptor(const std::string& id)
{
    auto request =
      protocol::codec::map_removeinterceptor_encode(get_name(), id);
    return invoke(request);
}

boost::future<EntryVector>
IMapImpl::execute_on_entries_data(
  const serialization::pimpl::data& entry_processor)
{
    auto request =
      protocol::codec::map_executeonallkeys_encode(get_name(), entry_processor);
    return invoke_and_get_future<EntryVector>(request);
}

boost::future<EntryVector>
IMapImpl::execute_on_entries_data(
  const serialization::pimpl::data& entry_processor,
  const serialization::pimpl::data& predicate)
{
    auto request = protocol::codec::map_executewithpredicate_encode(
      get_name(), entry_processor, predicate);
    return invoke_and_get_future<EntryVector>(request);
}

std::shared_ptr<spi::impl::ListenerMessageCodec>
IMapImpl::create_map_entry_listener_codec(
  bool include_value,
  serialization::pimpl::data&& predicate,
  int32_t listener_flags)
{
    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
      new MapEntryListenerWithPredicateMessageCodec(
        get_name(), include_value, listener_flags, std::move(predicate)));
}

std::shared_ptr<spi::impl::ListenerMessageCodec>
IMapImpl::create_map_entry_listener_codec(bool include_value,
                                          int32_t listener_flags)
{
    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
      new MapEntryListenerMessageCodec(
        get_name(), include_value, listener_flags));
}

std::shared_ptr<spi::impl::ListenerMessageCodec>
IMapImpl::create_map_entry_listener_codec(bool include_value,
                                          int32_t listener_flags,
                                          serialization::pimpl::data&& key)
{
    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
      new MapEntryListenerToKeyCodec(
        get_name(), include_value, listener_flags, std::move(key)));
}

void
IMapImpl::on_initialize()
{
    ProxyImpl::on_initialize();
    lock_reference_id_generator_ =
      get_context().get_lock_reference_id_generator();
}

IMapImpl::MapEntryListenerMessageCodec::MapEntryListenerMessageCodec(
  std::string name,
  bool include_value,
  int32_t listener_flags)
  : name_(std::move(name))
  , include_value_(include_value)
  , listener_flags_(listener_flags)
{}

protocol::ClientMessage
IMapImpl::MapEntryListenerMessageCodec::encode_add_request(
  bool local_only) const
{
    return protocol::codec::map_addentrylistener_encode(
      name_, include_value_, static_cast<int32_t>(listener_flags_), local_only);
}

protocol::ClientMessage
IMapImpl::MapEntryListenerMessageCodec::encode_remove_request(
  boost::uuids::uuid real_registration_id) const
{
    return protocol::codec::map_removeentrylistener_encode(
      name_, real_registration_id);
}

protocol::ClientMessage
IMapImpl::MapEntryListenerToKeyCodec::encode_add_request(bool local_only) const
{
    return protocol::codec::map_addentrylistenertokey_encode(
      name_,
      key_,
      include_value_,
      static_cast<int32_t>(listener_flags_),
      local_only);
}

protocol::ClientMessage
IMapImpl::MapEntryListenerToKeyCodec::encode_remove_request(
  boost::uuids::uuid real_registration_id) const
{
    return protocol::codec::map_removeentrylistener_encode(
      name_, real_registration_id);
}

IMapImpl::MapEntryListenerToKeyCodec::MapEntryListenerToKeyCodec(
  std::string name,
  bool include_value,
  int32_t listener_flags,
  serialization::pimpl::data key)
  : name_(std::move(name))
  , include_value_(include_value)
  , listener_flags_(listener_flags)
  , key_(std::move(key))
{}

IMapImpl::MapEntryListenerWithPredicateMessageCodec::
  MapEntryListenerWithPredicateMessageCodec(
    std::string name,
    bool include_value,
    int32_t listener_flags,
    serialization::pimpl::data&& predicate)
  : name_(std::move(name))
  , include_value_(include_value)
  , listener_flags_(listener_flags)
  , predicate_(std::move(predicate))
{}

protocol::ClientMessage
IMapImpl::MapEntryListenerWithPredicateMessageCodec::encode_add_request(
  bool local_only) const
{
    return protocol::codec::map_addentrylistenerwithpredicate_encode(
      name_,
      predicate_,
      include_value_,
      static_cast<int32_t>(listener_flags_),
      local_only);
}

protocol::ClientMessage
IMapImpl::MapEntryListenerWithPredicateMessageCodec::encode_remove_request(
  boost::uuids::uuid real_registration_id) const
{
    return protocol::codec::map_removeentrylistener_encode(
      name_, real_registration_id);
}

TransactionalQueueImpl::TransactionalQueueImpl(
  const std::string& name,
  txn::TransactionProxy& transaction_proxy)
  : TransactionalObject(iqueue::SERVICE_NAME, name, transaction_proxy)
{}

boost::future<bool>
TransactionalQueueImpl::offer(const serialization::pimpl::data& e,
                              std::chrono::milliseconds timeout)
{
    auto request = protocol::codec::transactionalqueue_offer_encode(
      get_name(),
      get_transaction_id(),
      util::get_current_thread_id(),
      e,
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

    return invoke_and_get_future<bool>(request);
}

boost::future<boost::optional<serialization::pimpl::data>>
TransactionalQueueImpl::poll_data(std::chrono::milliseconds timeout)
{
    auto request = protocol::codec::transactionalqueue_poll_encode(
      get_name(),
      get_transaction_id(),
      util::get_current_thread_id(),
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request);
}

boost::future<int>
TransactionalQueueImpl::size()
{
    auto request = protocol::codec::transactionalqueue_size_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id());

    return invoke_and_get_future<int>(request);
}

ISetImpl::ISetImpl(const std::string& instance_name,
                   spi::ClientContext* client_context)
  : ProxyImpl(iset::SERVICE_NAME, instance_name, client_context)
{
    serialization::pimpl::data key_data =
      get_context().get_serialization_service().to_data<std::string>(
        &instance_name);
    partition_id_ = get_partition_id(key_data);
}

boost::future<bool>
ISetImpl::remove_item_listener(boost::uuids::uuid registration_id)
{
    return get_context().get_client_listener_service().deregister_listener(
      registration_id);
}

boost::future<int>
ISetImpl::size()
{
    auto request = protocol::codec::set_size_encode(get_name());
    return invoke_and_get_future<int>(request, partition_id_);
}

boost::future<bool>
ISetImpl::is_empty()
{
    auto request = protocol::codec::set_isempty_encode(get_name());
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
ISetImpl::contains(const serialization::pimpl::data& element)
{
    auto request = protocol::codec::set_contains_encode(get_name(), element);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<std::vector<serialization::pimpl::data>>
ISetImpl::to_array_data()
{
    auto request = protocol::codec::set_getall_encode(get_name());
    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request, partition_id_);
}

boost::future<bool>
ISetImpl::add(const serialization::pimpl::data& element)
{
    auto request = protocol::codec::set_add_encode(get_name(), element);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
ISetImpl::remove(const serialization::pimpl::data& element)
{
    auto request = protocol::codec::set_remove_encode(get_name(), element);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
ISetImpl::contains_all(const std::vector<serialization::pimpl::data>& elements)
{
    auto request =
      protocol::codec::set_containsall_encode(get_name(), elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
ISetImpl::add_all(const std::vector<serialization::pimpl::data>& elements)
{
    auto request = protocol::codec::set_addall_encode(get_name(), elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
ISetImpl::remove_all(const std::vector<serialization::pimpl::data>& elements)
{
    auto request =
      protocol::codec::set_compareandremoveall_encode(get_name(), elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<bool>
ISetImpl::retain_all(const std::vector<serialization::pimpl::data>& elements)
{
    auto request =
      protocol::codec::set_compareandretainall_encode(get_name(), elements);
    return invoke_and_get_future<bool>(request, partition_id_);
}

boost::future<void>
ISetImpl::clear()
{
    auto request = protocol::codec::set_clear_encode(get_name());
    return to_void_future(invoke_on_partition(request, partition_id_));
}

std::shared_ptr<spi::impl::ListenerMessageCodec>
ISetImpl::create_item_listener_codec(bool include_value)
{
    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
      new SetListenerMessageCodec(get_name(), include_value));
}

ISetImpl::SetListenerMessageCodec::SetListenerMessageCodec(std::string name,
                                                           bool include_value)
  : name_(std::move(name))
  , include_value_(include_value)
{}

protocol::ClientMessage
ISetImpl::SetListenerMessageCodec::encode_add_request(bool local_only) const
{
    return protocol::codec::set_addlistener_encode(
      name_, include_value_, local_only);
}

protocol::ClientMessage
ISetImpl::SetListenerMessageCodec::encode_remove_request(
  boost::uuids::uuid real_registration_id) const
{
    return protocol::codec::set_removelistener_encode(name_,
                                                      real_registration_id);
}

ITopicImpl::ITopicImpl(const std::string& instance_name,
                       spi::ClientContext* context)
  : proxy::ProxyImpl("hz:impl:topicService", instance_name, context)
  , partition_id_(get_partition_id(to_data(instance_name)))
{}

boost::future<void>
ITopicImpl::publish(const serialization::pimpl::data& data)
{
    auto request = protocol::codec::topic_publish_encode(get_name(), data);
    return to_void_future(invoke_on_partition(request, partition_id_));
}

boost::future<boost::uuids::uuid>
ITopicImpl::add_message_listener(
  std::shared_ptr<impl::BaseEventHandler> topic_event_handler)
{
    return register_listener(create_item_listener_codec(),
                             std::move(topic_event_handler));
}

boost::future<bool>
ITopicImpl::remove_message_listener(boost::uuids::uuid registration_id)
{
    return get_context().get_client_listener_service().deregister_listener(
      registration_id);
}

std::shared_ptr<spi::impl::ListenerMessageCodec>
ITopicImpl::create_item_listener_codec()
{
    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
      new TopicListenerMessageCodec(get_name()));
}

ITopicImpl::TopicListenerMessageCodec::TopicListenerMessageCodec(
  std::string name)
  : name_(std::move(name))
{}

protocol::ClientMessage
ITopicImpl::TopicListenerMessageCodec::encode_add_request(bool local_only) const
{
    return protocol::codec::topic_addmessagelistener_encode(name_, local_only);
}

protocol::ClientMessage
ITopicImpl::TopicListenerMessageCodec::encode_remove_request(
  boost::uuids::uuid real_registration_id) const
{
    return protocol::codec::topic_removemessagelistener_encode(
      name_, real_registration_id);
}

ReplicatedMapImpl::ReplicatedMapImpl(const std::string& service_name,
                                     const std::string& object_name,
                                     spi::ClientContext* context)
  : ProxyImpl(service_name, object_name, context)
  , target_partition_id_(-1)
{}

const int32_t RingbufferImpl::MAX_BATCH_SIZE{ 1000 };
} // namespace proxy

namespace map {
const serialization::pimpl::data&
data_entry_view::get_key() const
{
    return key_;
}

const serialization::pimpl::data&
data_entry_view::get_value() const
{
    return value_;
}

int64_t
data_entry_view::get_cost() const
{
    return cost_;
}

int64_t
data_entry_view::get_creation_time() const
{
    return creation_time_;
}

int64_t
data_entry_view::get_expiration_time() const
{
    return expiration_time_;
}

int64_t
data_entry_view::get_hits() const
{
    return hits_;
}

int64_t
data_entry_view::get_last_access_time() const
{
    return last_access_time_;
}

int64_t
data_entry_view::get_last_stored_time() const
{
    return last_stored_time_;
}

int64_t
data_entry_view::get_last_update_time() const
{
    return last_update_time_;
}

int64_t
data_entry_view::get_version() const
{
    return version_;
}

int64_t
data_entry_view::get_ttl() const
{
    return ttl_;
}

int64_t
data_entry_view::get_max_idle() const
{
    return max_idle_;
}

data_entry_view::data_entry_view(serialization::pimpl::data&& key,
                                 serialization::pimpl::data&& value,
                                 int64_t cost,
                                 int64_t creation_time,
                                 int64_t expiration_time,
                                 int64_t hits,
                                 int64_t last_access_time,
                                 int64_t last_stored_time,
                                 int64_t last_update_time,
                                 int64_t version,
                                 int64_t ttl,
                                 int64_t max_idle)
  : key_(std::move(key))
  , value_(std::move(value))
  , cost_(cost)
  , creation_time_(creation_time)
  , expiration_time_(expiration_time)
  , hits_(hits)
  , last_access_time_(last_access_time)
  , last_stored_time_(last_stored_time)
  , last_update_time_(last_update_time)
  , version_(version)
  , ttl_(ttl)
  , max_idle_(max_idle)
{}
} // namespace map

namespace topic {
namespace impl {
namespace reliable {
ReliableTopicMessage::ReliableTopicMessage()
  : publish_time_(std::chrono::system_clock::now())
{}

ReliableTopicMessage::ReliableTopicMessage(
  hazelcast::client::serialization::pimpl::data&& payload_data,
  std::unique_ptr<address> address)
  : publish_time_(std::chrono::system_clock::now())
  , payload_(std::move(payload_data))
{
    if (address) {
        publisher_address_ = boost::make_optional(*address);
    }
}

std::chrono::system_clock::time_point
ReliableTopicMessage::get_publish_time() const
{
    return publish_time_;
}

const boost::optional<address>&
ReliableTopicMessage::get_publisher_address() const
{
    return publisher_address_;
}

serialization::pimpl::data&
ReliableTopicMessage::get_payload()
{
    return payload_;
}
} // namespace reliable
} // namespace impl
} // namespace topic

namespace serialization {
int32_t
hz_serializer<topic::impl::reliable::ReliableTopicMessage>::get_factory_id()
{
    return F_ID;
}

int
hz_serializer<topic::impl::reliable::ReliableTopicMessage>::get_class_id()
{
    return RELIABLE_TOPIC_MESSAGE;
}

void
hz_serializer<topic::impl::reliable::ReliableTopicMessage>::write_data(
  const topic::impl::reliable::ReliableTopicMessage& object,
  object_data_output& out)
{
    out.write<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                         object.publish_time_.time_since_epoch())
                         .count());
    out.write_object(object.publisher_address_);
    out.write(object.payload_.to_byte_array());
}

topic::impl::reliable::ReliableTopicMessage
hz_serializer<topic::impl::reliable::ReliableTopicMessage>::read_data(
  object_data_input& in)
{
    topic::impl::reliable::ReliableTopicMessage message;
    auto now = std::chrono::system_clock::now();
    message.publish_time_ = now +
                            std::chrono::milliseconds(in.read<int64_t>()) -
                            now.time_since_epoch();
    message.publisher_address_ = in.read_object<address>();
    message.payload_ =
      serialization::pimpl::data(in.read<std::vector<byte>>().value());
    return message;
}
} // namespace serialization

entry_event::entry_event(const std::string& name,
                         member&& member,
                         type event_type,
                         typed_data&& key,
                         typed_data&& value,
                         typed_data&& old_value,
                         typed_data&& merging_value)
  : name_(name)
  , member_(std::move(member))
  , event_type_(event_type)
  , key_(std::move(key))
  , value_(std::move(value))
  , old_value_(std::move(old_value))
  , merging_value_(std::move(merging_value))
{}

const typed_data&
entry_event::get_key() const
{
    return key_;
}

const typed_data&
entry_event::get_old_value() const
{
    return old_value_;
}

const typed_data&
entry_event::get_value() const
{
    return value_;
}

const typed_data&
entry_event::get_merging_value() const
{
    return merging_value_;
}

const member&
entry_event::get_member() const
{
    return member_;
}

entry_event::type
entry_event::get_event_type() const
{
    return event_type_;
}

const std::string&
entry_event::get_name() const
{
    return name_;
}

std::ostream&
operator<<(std::ostream& os, const entry_event& event)
{
    os << "name: " << event.name_ << " member: " << event.member_
       << " eventType: " << static_cast<int>(event.event_type_)
       << " key: " << event.key_.get_type()
       << " value: " << event.value_.get_type()
       << " oldValue: " << event.old_value_.get_type()
       << " mergingValue: " << event.merging_value_.get_type();
    return os;
}

map_event::map_event(member&& member,
                     entry_event::type event_type,
                     const std::string& name,
                     int number_of_entries_affected)
  : member_(member)
  , event_type_(event_type)
  , name_(name)
  , number_of_entries_affected_(number_of_entries_affected)
{}

const member&
map_event::get_member() const
{
    return member_;
}

entry_event::type
map_event::get_event_type() const
{
    return event_type_;
}

const std::string&
map_event::get_name() const
{
    return name_;
}

int
map_event::get_number_of_entries_affected() const
{
    return number_of_entries_affected_;
}

std::ostream&
operator<<(std::ostream& os, const map_event& event)
{
    os << "MapEvent{member: " << event.member_
       << " eventType: " << static_cast<int>(event.event_type_)
       << " name: " << event.name_
       << " numberOfEntriesAffected: " << event.number_of_entries_affected_;
    return os;
}

item_event_base::item_event_base(const std::string& name,
                                 const member& member,
                                 const item_event_type& event_type)
  : name_(name)
  , member_(member)
  , event_type_(event_type)
{}

const member&
item_event_base::get_member() const
{
    return member_;
}

item_event_type
item_event_base::get_event_type() const
{
    return event_type_;
}

const std::string&
item_event_base::get_name() const
{
    return name_;
}

item_event_base::~item_event_base() = default;

flake_id_generator::flake_id_generator(const std::string& object_name,
                                       spi::ClientContext* context)
  : flake_id_generator_impl(SERVICE_NAME, object_name, context)
{}
} // namespace client
} // namespace hazelcast
