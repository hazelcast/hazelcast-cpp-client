/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include <algorithm>
#include <boost/algorithm/string.hpp>

#include "hazelcast/cp/cp.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/impl/hazelcast_client_instance_impl.h"

namespace hazelcast {
    namespace cp {
        using namespace hazelcast::client::protocol;
        using namespace hazelcast::client::protocol::codec;
        using namespace hazelcast::util;

        raft_proxy_factory::raft_proxy_factory(client::spi::ClientContext &context) : context_(context) {}

        std::shared_ptr<fenced_lock>
        raft_proxy_factory::create_fenced_lock(raft_group_id &&group_id, const std::string &proxy_name,
                                               const std::string &object_name) {
            while (true) {
                auto proxy = lock_proxies_.get(proxy_name);
                if (proxy) {
                    if (proxy->get_group_id() != group_id) {
                        lock_proxies_.remove(proxy_name, proxy);
                    } else {
                        return proxy;
                    }
                }

                proxy = std::make_shared<fenced_lock>(proxy_name, context_, group_id, object_name);
                auto existing = lock_proxies_.put_if_absent(proxy_name, proxy);
                if (!existing) {
                    return proxy;
                } else if (existing->get_group_id() == group_id) {
                    return existing;
                }

                group_id = get_group_id(proxy_name, object_name).get();
            }
        }

        std::shared_ptr<counting_semaphore>
        raft_proxy_factory::create_semaphore(raft_group_id &&group_id, const std::string &proxy_name,
                                             const std::string &object_name) {
            auto request = client::protocol::codec::semaphore_getsemaphoretype_encode(proxy_name);
            auto is_sessionless = client::spi::impl::ClientInvocation::create(context_, request,
                                                                              object_name)->invoke().get().get_first_fixed_sized_field<bool>();
            if (is_sessionless) {
                return std::shared_ptr<counting_semaphore>(
                        new sessionless_semaphore(proxy_name, &context_, group_id, object_name,
                                                  context_.get_proxy_session_manager()));
            } else {
                return std::shared_ptr<counting_semaphore>(
                        new session_semaphore(proxy_name, &context_, group_id, object_name,
                                              context_.get_proxy_session_manager()));
            }
        }

        std::string raft_proxy_factory::without_default_group_name(const std::string &n) {
            std::string name = n;
            boost::trim(name);
            auto index = name.find('@');
            if (index == std::string::npos) {
                return name;
            }

            Preconditions::check_true(name.find('@', index + 1) == std::string::npos,
                                     "Custom group name must be specified at most once");

            auto group_name = name.substr(index + 1);
            boost::trim(group_name);
            if (group_name == DEFAULT_GROUP_NAME) {
                return name.substr(0, index);
            }
            return name;
        }

        std::string raft_proxy_factory::object_name_for_proxy(const std::string &name) {
            auto index = name.find('@');
            if (index == std::string::npos) {
                return name;
            }

            Preconditions::check_true(index < (name.size() - 1),
                                     "Object name cannot be empty string");
            Preconditions::check_true(name.find('@', index + 1) == std::string::npos,
                                     "Custom CP group name must be specified at most once");

            auto object_name = name.substr(0, index);
            boost::trim(object_name);
            Preconditions::check_true(object_name.size() > 0,
                                     "Object name cannot be empty string");
            return object_name;
        }

        boost::future<raft_group_id> raft_proxy_factory::get_group_id(const std::string &proxy_name, const std::string &object_name) {
            auto request = cpgroup_createcpgroup_encode(proxy_name);
            return client::spi::impl::ClientInvocation::create(context_, request, object_name)->invoke().then(
                    boost::launch::sync, [](boost::future<client::protocol::ClientMessage> f) {
                        return *f.get().get_first_var_sized_field<raft_group_id>();
                    });
        }

        cp_subsystem::cp_subsystem(client::spi::ClientContext &context) : context_(context), proxy_factory_(context) {
        }

        boost::future<std::shared_ptr<atomic_long>> cp_subsystem::get_atomic_long(const std::string &name) {
            return proxy_factory_.create_proxy<atomic_long>(name);
        }

        boost::future<std::shared_ptr<atomic_reference>> cp_subsystem::get_atomic_reference(const std::string &name) {
            return proxy_factory_.create_proxy<atomic_reference>(name);
        }

        boost::future<std::shared_ptr<latch>> cp_subsystem::get_latch(const std::string &name) {
            return proxy_factory_.create_proxy<latch>(name);
        }

        boost::future<std::shared_ptr<fenced_lock>> cp_subsystem::get_lock(const std::string &name) {
            return proxy_factory_.create_proxy<fenced_lock>(name);
        }

        boost::future<std::shared_ptr<counting_semaphore>> cp_subsystem::get_semaphore(const std::string &name) {
            return proxy_factory_.create_proxy<counting_semaphore>(name);
        }

        cp_proxy::cp_proxy(const std::string &service_name, const std::string &proxy_name,
                           client::spi::ClientContext *context,
                           const raft_group_id &group_id, const std::string &object_name) : ProxyImpl(service_name,
                                                                                                    proxy_name,
                                                                                                    context),
                                                                                          group_id_(group_id),
                                                                                          object_name_(object_name) {}

        void cp_proxy::on_destroy() {
            auto request = cpgroup_destroycpobject_encode(group_id_, get_service_name(), object_name_);
            invoke(request).get();
        }

        const raft_group_id &cp_proxy::get_group_id() const {
            return group_id_;
        }

        atomic_long::atomic_long(const std::string &name, client::spi::ClientContext &context,
                                 const raft_group_id &group_id, const std::string &object_name)
                : cp_proxy(SERVICE_NAME, name, &context, group_id, object_name) {}

        boost::future<int64_t> atomic_long::add_and_get(int64_t delta) {
            auto request = atomiclong_addandget_encode(group_id_, object_name_, delta);
            return invoke_and_get_future<int64_t>(request);
        }

        boost::future<bool> atomic_long::compare_and_set(int64_t expect, int64_t update) {
            auto request = atomiclong_compareandset_encode(group_id_, object_name_, expect, update);
            return invoke_and_get_future<bool>(request);
        }

        boost::future<int64_t> atomic_long::get_and_decrement() {
            return get_and_add(-1);
        }

        boost::future<int64_t> atomic_long::decrement_and_get() {
            return add_and_get(-1);
        }

        boost::future<int64_t> atomic_long::get() {
            auto request = atomiclong_get_encode(group_id_, object_name_);
            return invoke_and_get_future<int64_t>(request);
        }

        boost::future<int64_t> atomic_long::get_and_add(int64_t delta) {
            auto request = atomiclong_getandadd_encode(group_id_, object_name_, delta);
            return invoke_and_get_future<int64_t>(request);
        }

        boost::future<int64_t> atomic_long::get_and_set(int64_t new_value) {
            auto request = atomiclong_getandset_encode(group_id_, object_name_, new_value);
            return invoke_and_get_future<int64_t>(request);
        }

        boost::future<int64_t> atomic_long::increment_and_get() {
            return add_and_get(1);
        }

        boost::future<int64_t> atomic_long::get_and_increment() {
            return get_and_add(1);
        }

        boost::future<void> atomic_long::set(int64_t new_value) {
            return to_void_future(get_and_set(new_value));
        }

        boost::future<int64_t> atomic_long::alter_data(client::serialization::pimpl::data &function_data,
                                                       alter_result_type result_type) {
            auto request = atomiclong_alter_encode(group_id_, object_name_, function_data,
                                                   static_cast<int32_t>(result_type));
            return invoke_and_get_future<int64_t>(request);
        }

        boost::future<boost::optional<client::serialization::pimpl::data>>
        atomic_long::apply_data(client::serialization::pimpl::data &function_data) {
            auto request = atomiclong_apply_encode(group_id_, object_name_, function_data);
            return invoke_and_get_future<boost::optional<client::serialization::pimpl::data>>(request);
        }

        atomic_reference::atomic_reference(const std::string &name, client::spi::ClientContext &context,
                                           const raft_group_id &group_id, const std::string &object_name)
                : cp_proxy(SERVICE_NAME, name, &context, group_id, object_name) {}

        boost::future<boost::optional<client::serialization::pimpl::data>> atomic_reference::get_data() {
            auto request = atomicref_get_encode(group_id_, object_name_);
            return invoke_and_get_future<boost::optional<client::serialization::pimpl::data>>(request);
        }

        boost::future<boost::optional<client::serialization::pimpl::data>>
        atomic_reference::set_data(const client::serialization::pimpl::data &new_value_data) {
            auto request = atomicref_set_encode(group_id_, object_name_, &new_value_data, false);
            return invoke_and_get_future<boost::optional<client::serialization::pimpl::data>>(request);
        }

        boost::future<boost::optional<client::serialization::pimpl::data>>
        atomic_reference::get_and_set_data(const client::serialization::pimpl::data &new_value_data) {
            auto request = atomicref_set_encode(group_id_, object_name_, &new_value_data, true);
            return invoke_and_get_future<boost::optional<client::serialization::pimpl::data>>(request);
        }

        boost::future<bool>
        atomic_reference::compare_and_set_data(const client::serialization::pimpl::data &expect_data,
                                               const client::serialization::pimpl::data &update_data) {
            auto request = atomicref_compareandset_encode(group_id_, object_name_, &expect_data, &update_data);
            return invoke_and_get_future<bool>(request);
        }

        boost::future<bool> atomic_reference::contains_data(const client::serialization::pimpl::data &value_data) {
            auto request = atomicref_contains_encode(group_id_, object_name_, &value_data);
            return invoke_and_get_future<bool>(request);
        }

        boost::future<void> atomic_reference::alter_data(const client::serialization::pimpl::data &function_data) {
            return to_void_future(invoke_apply(function_data, return_value_type::NO_VALUE, true));
        }

        boost::future<boost::optional<client::serialization::pimpl::data>>
        atomic_reference::alter_and_get_data(const client::serialization::pimpl::data &function_data) {
            return invoke_apply(function_data, return_value_type::NEW, true);
        }

        boost::future<boost::optional<client::serialization::pimpl::data>>
        atomic_reference::get_and_alter_data(const client::serialization::pimpl::data &function_data) {
            return invoke_apply(function_data, return_value_type::OLD, true);
        }

        boost::future<boost::optional<client::serialization::pimpl::data>>
        atomic_reference::apply_data(const client::serialization::pimpl::data &function_data) {
            return invoke_apply(function_data, return_value_type::NEW, false);
        }

        boost::future<bool> atomic_reference::is_null() {
            return contains(static_cast<byte *>(nullptr));
        }

        boost::future<void> atomic_reference::clear() {
            return to_void_future(set(static_cast<byte *>(nullptr)));
        }

        boost::future<boost::optional<client::serialization::pimpl::data>>
        atomic_reference::invoke_apply(const client::serialization::pimpl::data function_data,
                                       return_value_type return_type, bool alter) {
            auto request = atomicref_apply_encode(group_id_, object_name_, function_data,
                                                  static_cast<int32_t>(return_type), alter);
            return invoke_and_get_future<boost::optional<client::serialization::pimpl::data>>(request);
        }

        latch::latch(const std::string &name, client::spi::ClientContext &context, const raft_group_id &group_id,
                     const std::string &object_name) : cp_proxy(SERVICE_NAME, name, &context, group_id, object_name) {}

        boost::future<bool> latch::try_set_count(int32_t count) {
            util::Preconditions::check_positive(count, "count must be positive!");

            auto request = countdownlatch_trysetcount_encode(group_id_, object_name_, count);
            return invoke_and_get_future<bool>(request);
        }

        boost::future<int32_t> latch::get_count() {
            auto request = countdownlatch_getcount_encode(group_id_, object_name_);
            return invoke_and_get_future<int32_t>(request);
        }

        boost::future<void> latch::count_down() {
            auto invocation_uid = get_context().get_hazelcast_client_implementation()->random_uuid();
            return get_round().then(boost::launch::sync, [=](boost::future<int32_t> f) {
                auto round = f.get();
                for (;;) {
                    try {
                        count_down(round, invocation_uid);
                        return;
                    } catch (client::exception::operation_timeout &) {
                        // I can retry safely because my retry would be idempotent...
                    }
                }

            });
        }

        boost::future<bool> latch::try_wait() {
            return get_count().then(boost::launch::sync, [](boost::future<int32_t> f) {
                return f.get() == 0;
            });
        }

        boost::future<int32_t> latch::get_round() {
            auto request = countdownlatch_getround_encode(group_id_, object_name_);
            return invoke_and_get_future<int32_t>(request);
        }

        void latch::count_down(int round, boost::uuids::uuid invocation_uid) {
            auto request = countdownlatch_countdown_encode(group_id_, object_name_, invocation_uid, round);
            invoke(request).get();
        }

        boost::future<void> latch::wait() {
            return to_void_future(wait_for(std::chrono::milliseconds(INT64_MAX)));
        }

        boost::future<std::cv_status> latch::wait_for(std::chrono::milliseconds timeout) {
            auto timeout_millis = std::max<int64_t>(0, timeout.count());
            auto invoation_uid = get_context().get_hazelcast_client_implementation()->random_uuid();
            auto request = countdownlatch_await_encode(group_id_, object_name_, invoation_uid, timeout_millis);
            return invoke_and_get_future<bool>(request).then(boost::launch::sync, [](boost::future<bool> f) {
                return f.get() ? std::cv_status::no_timeout : std::cv_status::timeout;
            });
        }

        bool raft_group_id::operator==(const raft_group_id &rhs) const {
            return name == rhs.name &&
                   seed == rhs.seed &&
                   group_id == rhs.group_id;
        }

        bool raft_group_id::operator!=(const raft_group_id &rhs) const {
            return !(rhs == *this);
        }

        constexpr int64_t fenced_lock::INVALID_FENCE;

        fenced_lock::fenced_lock(const std::string &name, client::spi::ClientContext &context,
                                 const raft_group_id &group_id,
                                 const std::string &object_name) : session_aware_proxy(SERVICE_NAME, name, &context,
                                                                                       group_id, object_name,
                                                                                       context.get_proxy_session_manager()) {}

        boost::future<void> fenced_lock::lock() {
            return to_void_future(lock_and_get_fence());
        }

        boost::future<int64_t> fenced_lock::lock_and_get_fence() {
            auto thread_id = util::get_current_thread_id();
            auto invocation_uid = get_context().random_uuid();

            auto do_lock_once = [=] () {
                auto session_id = session_manager_.acquire_session(group_id_);
                verify_locked_session_id_if_present(thread_id, session_id, true);
                return do_lock(session_id, thread_id, invocation_uid).then(boost::launch::sync,
                                                                           [=](boost::future<int64_t> f) {
                                                                               try {
                                                                                   auto fence = f.get();
                                                                                   if (fence != INVALID_FENCE) {
                                                                                       locked_session_ids_.put(
                                                                                               thread_id,
                                                                                               std::make_shared<int64_t>(
                                                                                                       session_id));
                                                                                       return fence;
                                                                                   }
                                                                                   BOOST_THROW_EXCEPTION(
                                                                                           client::exception::lock_acquire_limit_reached(
                                                                                           "fenced_lock::lock_and_get_fence", (
                                                                                                   boost::format(
                                                                                                           "Lock [%1%] reentrant lock limit is already reached!") %
                                                                                                   object_name_).str()));
                    } catch (client::exception::session_expired &) {
                        invalidate_session(session_id);
                        verify_no_locked_session_id_present(thread_id);
                        return INVALID_FENCE;
                    } catch (client::exception::wait_key_cancelled &) {
                        release_session(session_id);
                        BOOST_THROW_EXCEPTION(client::exception::lock_acquire_limit_reached(
                                                      "fenced_lock::lock_and_get_fence", (boost::format(
                                                              "Lock [%1%] not acquired because the lock call on the CP group is cancelled, possibly because of another indeterminate call from the same thread.") %
                                                                                          object_name_).str()));
                    } catch (...) {
                        release_session(session_id);
                        throw;
                    }
                });
            };

            return do_lock_once().then(boost::launch::sync, [=](boost::future<int64_t> f) {
                auto result = f.get();
                if (result != INVALID_FENCE) {
                    return result;
                }
                // iterate in the user thread
                for (result = do_lock_once().get(); result == INVALID_FENCE;) {}
                return result;
            });
        }

        void fenced_lock::verify_locked_session_id_if_present(int64_t thread_id, int64_t session_id,
                                                              bool should_release) {
            auto locked_session_id = locked_session_ids_.get(thread_id);
            if (locked_session_id && *locked_session_id != session_id) {
                locked_session_ids_.remove(thread_id);
                if (should_release) {
                    release_session(session_id);
                }

                throw_lock_ownership_lost(*locked_session_id);
            }
        }

        void fenced_lock::verify_no_locked_session_id_present(int64_t thread_id) {
            auto locked_session_id = locked_session_ids_.remove(thread_id);
            if (locked_session_id) {
                locked_session_ids_.remove(thread_id);
                throw_lock_ownership_lost(*locked_session_id);
            }
        }

        void fenced_lock::throw_lock_ownership_lost(int64_t session_id) const {
            BOOST_THROW_EXCEPTION(client::exception::lock_ownership_lost("fenced_lock", (boost::format(
                    "Current thread is not owner of the Lock[%1%] because its Session[%2%] is closed by server!") %
                                                                         get_name() % session_id).str()));
        }

        void fenced_lock::throw_illegal_monitor_state() const {
            BOOST_THROW_EXCEPTION(client::exception::illegal_monitor_state("fenced_lock", (boost::format(
                    "Current thread is not owner of the Lock[%1%]") %get_name()).str()));
        }

        boost::future<int64_t>
        fenced_lock::do_lock(int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid) {
            auto request = client::protocol::codec::fencedlock_lock_encode(group_id_, object_name_, session_id, thread_id, invocation_uid);
            return invoke_and_get_future<int64_t>(request);
        }

        boost::future<int64_t>
        fenced_lock::do_try_lock(int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid,
                                 std::chrono::milliseconds timeout) {
            auto request = client::protocol::codec::fencedlock_trylock_encode(group_id_, object_name_, session_id,
                                                                              thread_id, invocation_uid,
                                                                              std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                                      timeout).count());
            return invoke_and_get_future<int64_t>(request);
        }

        boost::future<bool>
        fenced_lock::do_unlock(int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid) {
            auto request = client::protocol::codec::fencedlock_unlock_encode(group_id_, object_name_, session_id, thread_id, invocation_uid);
            return invoke_and_get_future<bool>(request);
        }

        boost::future<fenced_lock::lock_ownership_state> fenced_lock::do_get_lock_ownership_state(){
            auto request = client::protocol::codec::fencedlock_getlockownership_encode(group_id_, object_name_);
            return invoke(request).then(boost::launch::sync, [](boost::future<client::protocol::ClientMessage> f) {
                auto msg = f.get();
                fenced_lock::lock_ownership_state state;
                state.fence = msg.get_first_fixed_sized_field<int64_t>();
                state.lock_count = msg.get<int32_t>();
                state.session_id = msg.get<int64_t>();
                state.thread_id = msg.get<int64_t>();
                return state;
            });
        }

        void fenced_lock::invalidate_session(int64_t session_id) {
            session_manager_.invalidate_session(group_id_, session_id);
        }

        boost::future<bool> fenced_lock::try_lock() {
            return try_lock_and_get_fence().then(boost::launch::sync, [](boost::future<int64_t> f) {
                return f.get() != INVALID_FENCE;
            });
        }

        boost::future<bool> fenced_lock::try_lock(std::chrono::milliseconds timeout) {
            return try_lock_and_get_fence(timeout).then(boost::launch::sync, [](boost::future<int64_t> f) {
                return f.get() != INVALID_FENCE;
            });
        }

        boost::future<int64_t> fenced_lock::try_lock_and_get_fence() {
            return try_lock_and_get_fence(std::chrono::milliseconds(0));
        }

        boost::future<int64_t>
        fenced_lock::try_lock_and_get_fence(std::chrono::milliseconds timeout) {
            auto thread_id = util::get_current_thread_id();
            auto invocation_uid = get_context().random_uuid();

            auto do_try_lock_once = [=] () {
                using namespace std::chrono;
                auto start = steady_clock::now();
                auto session_id = session_manager_.acquire_session(group_id_);
                verify_locked_session_id_if_present(thread_id, session_id, true);
                return do_try_lock(session_id, thread_id, invocation_uid, timeout).then(boost::launch::sync,
                                                                                        [=](boost::future<int64_t> f) {
                                                                                            try {
                                                                                                auto fence = f.get();
                                                                                                if (fence !=
                                                                                                    INVALID_FENCE) {
                                                                                                    locked_session_ids_.put(
                                                                                                            thread_id,
                                                                                                            std::make_shared<int64_t>(
                                                                                                                    session_id));
                                                                                                    return std::make_pair(
                                                                                                            fence,
                                                                                                            false);
                                                                                                } else {
                                                                                                    release_session(
                                                                                                            session_id);
                                                                                                }
                                                                                                return std::make_pair(
                                                                                                        fence, false);
                                                                                            } catch (
                                                                                                    client::exception::session_expired &) {
                        invalidate_session(session_id);
                        verify_no_locked_session_id_present(thread_id);
                        auto timeout_left = timeout - (steady_clock::now() - start);
                        if (timeout_left.count() <= 0) {
                            return std::make_pair(INVALID_FENCE, false);
                        }
                        return std::make_pair(INVALID_FENCE, false);
                    } catch (client::exception::wait_key_cancelled &) {
                        release_session(session_id);
                        return std::make_pair(INVALID_FENCE, false);
                    } catch (...) {
                        release_session(session_id);
                        throw;
                    }
                });
            };

            return do_try_lock_once().then(boost::launch::sync, [=](boost::future<std::pair<int64_t, bool>> f) {
                auto result = f.get();
                if (!result.second) {
                    return result.first;
                }
                // iterate in the user thread
                for (result = do_try_lock_once().get(); result.second;) {}
                return result.first;
            });
        }

        boost::future<void> fenced_lock::unlock() {
            auto thread_id = util::get_current_thread_id();
            int64_t session_id = session_manager_.get_session(group_id_);

            // the order of the following checks is important.
            verify_locked_session_id_if_present(thread_id, session_id, false);
            if (session_id == internal::session::proxy_session_manager::NO_SESSION_ID) {
                locked_session_ids_.remove(thread_id);
                throw_illegal_monitor_state();
            }

            return do_unlock(session_id, thread_id, get_context().random_uuid()).then(boost::launch::sync,
                                                                                      [=](boost::future<bool> f) {
                                                                                          try {
                                                                                              auto still_locked_by_current_thread = f.get();
                                                                                              if (still_locked_by_current_thread) {
                                                                                                  locked_session_ids_.put(
                                                                                                          thread_id,
                                                                                                          std::make_shared<int64_t>(
                                                                                                                  session_id));
                                                                                              } else {
                                                                                                  locked_session_ids_.remove(
                                                                                                          thread_id);
                                                                                              }

                                                                                              release_session(
                                                                                                      session_id);
                                                                                          } catch (
                                                                                                  client::exception::session_expired &) {
                    invalidate_session(session_id);
                    locked_session_ids_.remove(thread_id);

                    throw_lock_ownership_lost(session_id);
                } catch (client::exception::illegal_monitor_state &) {
                    locked_session_ids_.remove(thread_id);
                    throw;
                }
            });
        }

        boost::future<int64_t> fenced_lock::get_fence() {
            auto thread_id = util::get_current_thread_id();
            int64_t session_id = session_manager_.get_session(group_id_);

            // the order of the following checks is important.
            verify_locked_session_id_if_present(thread_id, session_id, false);
            if (session_id == internal::session::proxy_session_manager::NO_SESSION_ID) {
                locked_session_ids_.remove(thread_id);
                throw_illegal_monitor_state();
            }

            return do_get_lock_ownership_state().then(boost::launch::sync, [=](boost::future<lock_ownership_state> f) {
                auto ownership = f.get();
                if (ownership.is_locked_by(session_id, thread_id)) {
                    locked_session_ids_.put(thread_id, std::make_shared<int64_t>(session_id));
                    return ownership.fence;
                }

                verify_no_locked_session_id_present(thread_id);
                throw_illegal_monitor_state();
                return INVALID_FENCE;
            });
        }

        boost::future<bool> fenced_lock::is_locked() {
            auto thread_id = util::get_current_thread_id();
            int64_t session_id = session_manager_.get_session(group_id_);

            verify_locked_session_id_if_present(thread_id, session_id, false);

            return do_get_lock_ownership_state().then(boost::launch::sync, [=](boost::future<lock_ownership_state> f) {
                auto ownership = f.get();
                if (ownership.is_locked_by(session_id, thread_id)) {
                    locked_session_ids_.put(thread_id, std::make_shared<int64_t>(session_id));
                    return true;
                }

                verify_no_locked_session_id_present(thread_id);
                return ownership.is_locked();
            });
        }

        boost::future<bool> fenced_lock::is_locked_by_current_thread() {
            auto thread_id = util::get_current_thread_id();
            int64_t session_id = session_manager_.get_session(group_id_);

            verify_locked_session_id_if_present(thread_id, session_id, false);

            return do_get_lock_ownership_state().then(boost::launch::sync, [=](boost::future<lock_ownership_state> f) {
                auto ownership = f.get();
                auto locked_by_current_thread = ownership.is_locked_by(session_id, thread_id);
                if (locked_by_current_thread) {
                    locked_session_ids_.put(thread_id, std::make_shared<int64_t>(session_id));
                } else {
                    verify_no_locked_session_id_present(thread_id);
                }

                return locked_by_current_thread;
            });
        }

        boost::future<int32_t> fenced_lock::get_lock_count() {
            auto thread_id = util::get_current_thread_id();
            int64_t session_id = session_manager_.get_session(group_id_);

            verify_locked_session_id_if_present(thread_id, session_id, false);

            return do_get_lock_ownership_state().then(boost::launch::sync, [=](boost::future<lock_ownership_state> f) {
                auto ownership = f.get();
                if (ownership.is_locked_by(session_id, thread_id)) {
                    locked_session_ids_.put(thread_id, std::make_shared<int64_t>(session_id));
                } else {
                    verify_no_locked_session_id_present(thread_id);
                }

                return ownership.lock_count;
            });
        }

        const raft_group_id &fenced_lock::get_group_id() {
            return group_id_;
        }

        bool operator==(const fenced_lock &lhs, const fenced_lock &rhs) {
            return lhs.get_service_name() == rhs.get_service_name() && lhs.get_name() == rhs.get_name();
        }

        void fenced_lock::post_destroy() {
            ClientProxy::post_destroy();
            locked_session_ids_.clear();
        }

        session_aware_proxy::session_aware_proxy(const std::string &service_name, const std::string &proxy_name,
                                                 client::spi::ClientContext *context, const raft_group_id &group_id,
                                                 const std::string &object_name,
                                                 internal::session::proxy_session_manager &session_manager) : cp_proxy(
                service_name, proxy_name, context, group_id, object_name), session_manager_(session_manager) {}

        void session_aware_proxy::release_session(int64_t session_id) {
            session_manager_.release_session(group_id_, session_id);
        }

        bool fenced_lock::lock_ownership_state::is_locked_by(int64_t session, int64_t thread) {
            return is_locked() && session_id == session && thread_id == thread;
        }

        bool fenced_lock::lock_ownership_state::is_locked() {
            return fence != INVALID_FENCE;
        }

        counting_semaphore::counting_semaphore(const std::string &proxy_name, client::spi::ClientContext *context,
                                               const raft_group_id &group_id, const std::string &object_name,
                                               internal::session::proxy_session_manager &session_manager)
                : session_aware_proxy(SERVICE_NAME,
                                      proxy_name, context,
                                      group_id,
                                      object_name, session_manager) {}

        boost::future<bool> counting_semaphore::init(int32_t permits) {
            util::Preconditions::check_not_negative(permits, "Permits must be non-negative!");

            auto request = client::protocol::codec::semaphore_init_encode(group_id_, object_name_, permits);
            return client::spi::impl::ClientInvocation::create(context_, request, object_name_)->invoke().then(
                    boost::launch::sync, [](boost::future<client::protocol::ClientMessage> f) {
                        return f.get().get_first_fixed_sized_field<bool>();
                    });
        }

        boost::future<bool> counting_semaphore::try_acquire(int32_t permits) {
            return try_acquire_for(std::chrono::milliseconds::zero(), permits);
        }

        boost::future<bool> counting_semaphore::try_acquire_for(std::chrono::milliseconds rel_time, int32_t permits) {
            if (rel_time < std::chrono::milliseconds::zero()) {
                rel_time = std::chrono::milliseconds ::zero();
            }
            return try_acquire_for_millis(permits, rel_time);
        }

        boost::future<void> counting_semaphore::do_release(int32_t permits, int64_t thread_id, int64_t session_id) {
            auto invocation_uid = get_context().get_hazelcast_client_implementation()->random_uuid();
            auto request = codec::semaphore_release_encode(group_id_, object_name_, session_id, thread_id, invocation_uid, permits);
            return to_void_future(
                    client::spi::impl::ClientInvocation::create(context_, request, object_name_)->invoke());
        }

        boost::future<int32_t> counting_semaphore::available_permits() {
            auto request = codec::semaphore_availablepermits_encode(group_id_, object_name_);
            return decode<int32_t>(
                    client::spi::impl::ClientInvocation::create(context_, request, object_name_)->invoke());
        }

        boost::future<void> counting_semaphore::reduce_permits(int32_t reduction) {
            util::Preconditions::check_not_negative(reduction, "Reduction must be non-negative!");
            if (reduction == 0) {
                return boost::make_ready_future();
            }
            return do_change_permits(-reduction);
        }

        boost::future<void> counting_semaphore::increase_permits(int32_t increase) {
            util::Preconditions::check_not_negative(increase, "Increase must be non-negative!");
            if (increase == 0) {
                return boost::make_ready_future();
            }
            return do_change_permits(increase);
        }

        sessionless_semaphore::sessionless_semaphore(const std::string &proxy_name, client::spi::ClientContext *context,
                                                     const raft_group_id &group_id, const std::string &object_name,
                                                     internal::session::proxy_session_manager &session_manager)
                : counting_semaphore(proxy_name, context, group_id, object_name, session_manager) {}

        boost::future<void> sessionless_semaphore::acquire(int32_t permits) {
            util::Preconditions::check_positive(permits, "permits must be positive number.");

            return to_void_future(do_try_acquire(permits, std::chrono::milliseconds(-1)));
        }

        boost::future<bool>
        sessionless_semaphore::do_try_acquire(int32_t permits, std::chrono::milliseconds timeout_ms) {
            auto cluster_wide_threadId = get_thread_id();
            auto invocation_uid = get_context().get_hazelcast_client_implementation()->random_uuid();
            auto request = client::protocol::codec::semaphore_acquire_encode(group_id_, object_name_, internal::session::proxy_session_manager::NO_SESSION_ID, cluster_wide_threadId, invocation_uid, permits, timeout_ms.count());
            return client::spi::impl::ClientInvocation::create(context_, request, object_name_)->invoke().then(
                    boost::launch::sync, [=](boost::future<client::protocol::ClientMessage> f) {
                        try {
                            return f.get().get_first_fixed_sized_field<bool>();
                        } catch (client::exception::wait_key_cancelled &) {
                            throw client::exception::illegal_state("sessionless_semaphore::acquire",
                                                                   (boost::format(
                                                                           "Semaphore[%1%] ] not acquired because the acquire call on the CP group is cancelled, possibly because of another indeterminate call from the same thread.") %
                                                                    object_name_).str());
                        }
                    });
        }

        boost::future<bool> sessionless_semaphore::try_acquire_for_millis(int32_t permits, std::chrono::milliseconds timeout) {
            util::Preconditions::check_positive(permits, "Permits must be positive!");

            return do_try_acquire(permits, timeout > std::chrono::milliseconds::zero() ? timeout : std::chrono::milliseconds::zero());
        }

        boost::future<void> sessionless_semaphore::release(int32_t permits) {
            util::Preconditions::check_positive(permits, "Permits must be positive!");
            auto thread_id = get_thread_id();
            return do_release(permits, thread_id, internal::session::proxy_session_manager::NO_SESSION_ID);
        }

        int64_t sessionless_semaphore::get_thread_id() {
            return session_manager_.get_or_create_unique_thread_id(group_id_);
        }

        boost::future<int32_t> sessionless_semaphore::drain_permits() {
            auto cluster_wide_threadId = get_thread_id();
            auto invocation_uid = get_context().get_hazelcast_client_implementation()->random_uuid();
            auto request = client::protocol::codec::semaphore_drain_encode(group_id_, object_name_, internal::session::proxy_session_manager::NO_SESSION_ID, cluster_wide_threadId, invocation_uid);
            return decode<int32_t>(
                    client::spi::impl::ClientInvocation::create(context_, request, object_name_)->invoke());
        }

        boost::future<void> sessionless_semaphore::do_change_permits(int32_t delta) {
            auto cluster_wide_threadId = get_thread_id();
            auto invocation_uid = get_context().get_hazelcast_client_implementation()->random_uuid();
            auto request = client::protocol::codec::semaphore_change_encode(group_id_, object_name_, internal::session::proxy_session_manager::NO_SESSION_ID, cluster_wide_threadId, invocation_uid, delta);
            return to_void_future(
                    client::spi::impl::ClientInvocation::create(context_, request, object_name_)->invoke());
        }

        session_semaphore::session_semaphore(const std::string &proxy_name, client::spi::ClientContext *context,
                                             const raft_group_id &group_id, const std::string &object_name,
                                             internal::session::proxy_session_manager &session_manager)
                : counting_semaphore(proxy_name, context, group_id, object_name, session_manager) {}

        boost::future<void> session_semaphore::acquire(int32_t permits) {
            return to_void_future(try_acquire_for_millis(permits, std::chrono::milliseconds(-1)));
        }

        boost::future<bool>
        session_semaphore::try_acquire_for_millis(int32_t permits, std::chrono::milliseconds timeout) {
            util::Preconditions::check_not_negative(permits, "permits must not be negative number.");

            auto thread_id = get_thread_id();
            auto invocation_uid = get_context().get_hazelcast_client_implementation()->random_uuid();

            auto do_try_acquire_once = ([=] () {
                auto start = std::chrono::steady_clock::now();
                auto use_timeout = timeout >= std::chrono::milliseconds::zero();
                auto session_id = session_manager_.acquire_session(group_id_, permits);
                auto request = client::protocol::codec::semaphore_acquire_encode(group_id_, object_name_,
                                                                                 session_id,
                                                                                 thread_id, invocation_uid, permits,
                                                                                 timeout.count());
                return client::spi::impl::ClientInvocation::create(context_, request, object_name_)->invoke().then(
                        boost::launch::sync, [=](boost::future<client::protocol::ClientMessage> f) {
                            try {
                                auto acquired = f.get().get_first_fixed_sized_field<bool>();
                                if (!acquired) {
                                    session_manager_.release_session(group_id_, session_id);
                                }
                                // first bool means acquired or not, second bool means if should try again
                                return std::make_pair(acquired, false);
                            } catch (client::exception::session_expired &) {
                                session_manager_.invalidate_session(group_id_, session_id);
                                if (use_timeout && (timeout - (std::chrono::steady_clock::now() - start) <=
                                                    std::chrono::milliseconds::zero())) {
                                    return std::make_pair(false, false);
                                }
                                return std::make_pair(false, true);
                            } catch (client::exception::wait_key_cancelled &) {
                                session_manager_.release_session(group_id_, session_id, permits);
                                if (!use_timeout) {
                                    BOOST_THROW_EXCEPTION(
                                            client::exception::illegal_state(
                                                    "session_semaphore::try_acquire_for_millis", (boost::format(
                                                            "Semaphore[%1%] not acquired because the acquire call on the CP group is cancelled, possibly because of another indeterminate call from the same thread.") %
                                                                                                  object_name_).str()));
                                }
                                return std::make_pair(false, false);
                            }
                        });
            });

            return do_try_acquire_once().then(boost::launch::sync, [=](boost::future<std::pair<bool, bool>> f) {
                auto result = f.get();
                if (!result.second) {
                    return result.first;
                }
                for (; result.second; result = do_try_acquire_once().get());
                return result.first;
            });
        }

        boost::future<void> session_semaphore::release(int32_t permits) {
            util::Preconditions::check_positive(permits, "Permits must be positive!");
            auto session_id = session_manager_.get_session(group_id_);
            if (session_id == internal::session::proxy_session_manager::NO_SESSION_ID) {
                throw_illegal_state_exception(nullptr);
            }

            auto thread_id = get_thread_id();
            return do_release(permits, thread_id, session_id).then([=] (boost::future<void> f) {
                try {
                    f.get();
                    session_manager_.release_session(group_id_, session_id, permits);
                } catch (client::exception::session_expired &) {
                    session_manager_.invalidate_session(group_id_, session_id);
                    session_manager_.release_session(group_id_, session_id, permits);
                    throw_illegal_state_exception(std::current_exception());
                }
            });
        }

        void session_semaphore::throw_illegal_state_exception(std::exception_ptr e) {
            auto ise = boost::enable_current_exception(
                    client::exception::illegal_state("session_semaphore::illegal_state",
                                                     "No valid session!"));
            if (!e) {
                throw ise;
            }
            try {
                std::rethrow_exception(e);
            } catch (...) {
                std::throw_with_nested(ise);
            }
        }

        int64_t session_semaphore::get_thread_id() {
            return util::get_current_thread_id();
        }

        boost::future<int32_t> session_semaphore::drain_permits() {
            auto thread_id = get_thread_id();
            auto invocation_uid = get_context().get_hazelcast_client_implementation()->random_uuid();

            auto do_drain_once = ([=] () {
                auto session_id = session_manager_.acquire_session(group_id_, DRAIN_SESSION_ACQ_COUNT);
                auto request = client::protocol::codec::semaphore_drain_encode(group_id_, object_name_,
                                                                               session_id,
                                                                               thread_id, invocation_uid);
                return client::spi::impl::ClientInvocation::create(context_, request, object_name_)->invoke().then(
                        boost::launch::sync, [=](boost::future<client::protocol::ClientMessage> f) {
                            try {
                                auto count = f.get().get_first_fixed_sized_field<int32_t>();
                                session_manager_.release_session(group_id_, session_id,
                                                                 DRAIN_SESSION_ACQ_COUNT - count);
                                return count;
                            } catch (client::exception::session_expired &) {
                                session_manager_.invalidate_session(group_id_, session_id);
                                return -1;
                            }
                        });
            });

            return do_drain_once().then(boost::launch::sync, [=](boost::future<int32_t> f) {
                int32_t count = f.get();
                if (count != -1) {
                    return count;
                }
                while ((count = do_drain_once().get()) == -1) {}
                return count;
            });
        }

        boost::future<void> session_semaphore::do_change_permits(int32_t delta) {
            auto session_id = session_manager_.acquire_session(group_id_, DRAIN_SESSION_ACQ_COUNT);
            auto thread_id = get_thread_id();
            auto invocation_uid = get_context().get_hazelcast_client_implementation()->random_uuid();

            auto request = client::protocol::codec::semaphore_change_encode(group_id_, object_name_,
                                                                            session_id,
                                                                            thread_id, invocation_uid, delta);
            return client::spi::impl::ClientInvocation::create(context_, request, object_name_)->invoke().then(
                    boost::launch::sync, [=](boost::future<client::protocol::ClientMessage> f) {
                        try {
                            f.get();
                            session_manager_.release_session(group_id_, session_id);
                        } catch (client::exception::session_expired &) {
                            session_manager_.invalidate_session(group_id_, session_id);
                            throw_illegal_state_exception(std::current_exception());
                        }
                    });
        }
    }
}

namespace std {
    std::size_t
    hash<hazelcast::cp::raft_group_id>::operator()(const hazelcast::cp::raft_group_id &group_id) const noexcept {
        std::size_t seed = 0;
        boost::hash_combine(seed, group_id.name);
        boost::hash_combine(seed, group_id.seed);
        boost::hash_combine(seed, group_id.group_id);
        return seed;
    }
}

namespace boost {
    std::size_t
    hash<hazelcast::cp::raft_group_id>::operator()(const hazelcast::cp::raft_group_id &group_id) const noexcept {
        return std::hash<hazelcast::cp::raft_group_id>()(group_id);
    }
}



