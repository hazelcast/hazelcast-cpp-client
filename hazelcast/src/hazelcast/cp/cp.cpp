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
#include <algorithm>
#include <boost/algorithm/string.hpp>

#include "hazelcast/cp/cp.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"

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
                auto existing = lock_proxies_.putIfAbsent(proxy_name, proxy);
                if (!existing) {
                    return proxy;
                } else if (existing->get_group_id() == group_id) {
                    return existing;
                }

                group_id = get_group_id(proxy_name, object_name);
            }
        }

        std::shared_ptr<semaphore>
        raft_proxy_factory::create_semaphore(raft_group_id &&group_id, const std::string &proxy_name,
                                             const std::string &object_name) {
            return nullptr;
        }

        std::string raft_proxy_factory::without_default_group_name(const std::string &n) {
            std::string name = n;
            boost::trim(name);
            auto index = name.find('@');
            if (index == std::string::npos) {
                return name;
            }

            Preconditions::checkTrue(name.find('@', index + 1) == std::string::npos,
                                     "Custom group name must be specified at most once");

            auto group_name = name.substr(index + 1);
            boost::trim(group_name);
            if (group_name == DEFAULT_GROUP_NAME) {
                name.substr(0, index);
            }
            return name;
        }

        std::string raft_proxy_factory::object_name_for_proxy(const std::string &name) {
            auto index = name.find('@');
            if (index == std::string::npos) {
                return name;
            }

            Preconditions::checkTrue(index < (name.size() - 1),
                                     "Object name cannot be empty string");
            Preconditions::checkTrue(name.find('@', index + 1) == std::string::npos,
                                     "Custom CP group name must be specified at most once");

            auto object_name = name.substr(0, index);
            boost::trim(object_name);
            Preconditions::checkTrue(object_name.size() > 0,
                                     "Object name cannot be empty string");
            return object_name;
        }

        raft_group_id raft_proxy_factory::get_group_id(const std::string &proxy_name, const std::string &object_name) {
            auto request = cpgroup_createcpgroup_encode(proxy_name);
            return *spi::impl::ClientInvocation::create(context_, request,
                                                        object_name)->invoke().get().get_first_var_sized_field<raft_group_id>();
        }

        cp_subsystem::cp_subsystem(client::spi::ClientContext &context) : context_(context), proxy_factory_(context) {
        }

        std::shared_ptr<atomic_long> cp_subsystem::get_atomic_long(const std::string &name) {
            return proxy_factory_.create_proxy<atomic_long>(name);
        }

        std::shared_ptr<atomic_reference> cp_subsystem::get_atomic_reference(const std::string &name) {
            return proxy_factory_.create_proxy<atomic_reference>(name);
        }

        std::shared_ptr<latch> cp_subsystem::get_latch(const std::string &name) {
            return proxy_factory_.create_proxy<latch>(name);
        }

        std::shared_ptr<fenced_lock> cp_subsystem::get_lock(const std::string &name) {
            return proxy_factory_.create_proxy<fenced_lock>(name);
        }

        cp_proxy::cp_proxy(const std::string &serviceName, const std::string &proxyName,
                           client::spi::ClientContext *context,
                           const raft_group_id &groupId, const std::string &objectName) : ProxyImpl(serviceName,
                                                                                                    proxyName,
                                                                                                    context),
                                                                                          group_id_(groupId),
                                                                                          object_name_(objectName) {}

        void cp_proxy::onDestroy() {
            auto request = cpgroup_destroycpobject_encode(group_id_, getServiceName(), object_name_);
            invoke(request).get();
        }

        const raft_group_id &cp_proxy::get_group_id() const {
            return group_id_;
        }

        atomic_long::atomic_long(const std::string &name, spi::ClientContext &context,
                                 const raft_group_id &groupId, const std::string &objectName)
                : cp_proxy(SERVICE_NAME, name, &context, groupId, objectName) {}

        boost::future<int64_t> atomic_long::add_and_get(int64_t delta) {
            auto request = atomiclong_addandget_encode(group_id_, object_name_, delta);
            return invokeAndGetFuture<int64_t>(request);
        }

        boost::future<bool> atomic_long::compare_and_set(int64_t expect, int64_t update) {
            auto request = atomiclong_compareandset_encode(group_id_, object_name_, expect, update);
            return invokeAndGetFuture<bool>(request);
        }

        boost::future<int64_t> atomic_long::decrement_and_get() {
            return add_and_get(-1);
        }

        boost::future<int64_t> atomic_long::get() {
            auto request = atomiclong_get_encode(group_id_, object_name_);
            return invokeAndGetFuture<int64_t>(request);
        }

        boost::future<int64_t> atomic_long::get_and_add(int64_t delta) {
            auto request = atomiclong_getandadd_encode(group_id_, object_name_, delta);
            return invokeAndGetFuture<int64_t>(request);
        }

        boost::future<int64_t> atomic_long::get_and_set(int64_t newValue) {
            auto request = atomiclong_getandset_encode(group_id_, object_name_, newValue);
            return invokeAndGetFuture<int64_t>(request);
        }

        boost::future<int64_t> atomic_long::increment_and_get() {
            return add_and_get(1);
        }

        boost::future<int64_t> atomic_long::get_and_increment() {
            return get_and_add(1);
        }

        boost::future<void> atomic_long::set(int64_t newValue) {
            return toVoidFuture(get_and_set(newValue));
        }

        boost::future<int64_t> atomic_long::alter_data(Data &function_data,
                                                       alter_result_type result_type) {
            auto request = atomiclong_alter_encode(group_id_, object_name_, function_data,
                                                   static_cast<int32_t>(result_type));
            return invokeAndGetFuture<int64_t>(request);
        }

        boost::future<boost::optional<Data>> atomic_long::apply_data(Data &function_data) {
            auto request = atomiclong_apply_encode(group_id_, object_name_, function_data);
            return invokeAndGetFuture<boost::optional<Data>>(request);
        }

        atomic_reference::atomic_reference(const std::string &name, spi::ClientContext &context,
                                           const raft_group_id &groupId, const std::string &objectName)
                : cp_proxy(SERVICE_NAME, name, &context, groupId, objectName) {}

        boost::future<boost::optional<Data>> atomic_reference::get_data() {
            auto request = atomicref_get_encode(group_id_, object_name_);
            return invokeAndGetFuture<boost::optional<Data>>(request);
        }

        boost::future<boost::optional<Data>> atomic_reference::set_data(const Data &new_value_data) {
            auto request = atomicref_set_encode(group_id_, object_name_, &new_value_data, false);
            return invokeAndGetFuture<boost::optional<Data>>(request);
        }

        boost::future<boost::optional<Data>> atomic_reference::get_and_set_data(const Data &new_value_data) {
            auto request = atomicref_set_encode(group_id_, object_name_, &new_value_data, true);
            return invokeAndGetFuture<boost::optional<Data>>(request);
        }

        boost::future<bool> atomic_reference::compare_and_set_data(const Data &expect_data, const Data &update_data) {
            auto request = atomicref_compareandset_encode(group_id_, object_name_, &expect_data, &update_data);
            return invokeAndGetFuture<bool>(request);
        }

        boost::future<bool> atomic_reference::contains_data(const Data &value_data) {
            auto request = atomicref_contains_encode(group_id_, object_name_, &value_data);
            return invokeAndGetFuture<bool>(request);
        }

        boost::future<void> atomic_reference::alter_data(const Data &function_data) {
            return toVoidFuture(invoke_apply(function_data, return_value_type::NO_VALUE, true));
        }

        boost::future<boost::optional<Data>> atomic_reference::alter_and_get_data(const Data &function_data) {
            return invoke_apply(function_data, return_value_type::NEW, true);
        }

        boost::future<boost::optional<Data>> atomic_reference::get_and_alter_data(const Data &function_data) {
            return invoke_apply(function_data, return_value_type::OLD, true);
        }

        boost::future<boost::optional<Data>> atomic_reference::apply_data(const Data &function_data) {
            return invoke_apply(function_data, return_value_type::NEW, false);
        }

        boost::future<bool> atomic_reference::is_null() {
            return contains(static_cast<byte *>(nullptr));
        }

        boost::future<void> atomic_reference::clear() {
            return toVoidFuture(set(static_cast<byte *>(nullptr)));
        }

        boost::future<boost::optional<Data>>
        atomic_reference::invoke_apply(const Data function_data, return_value_type return_type, bool alter) {
            auto request = atomicref_apply_encode(group_id_, object_name_, function_data,
                                                  static_cast<int32_t>(return_type), alter);
            return invokeAndGetFuture<boost::optional<Data>>(request);
        }

        latch::latch(const std::string &name, spi::ClientContext &context, const raft_group_id &groupId,
                     const std::string &objectName) : cp_proxy(SERVICE_NAME, name, &context, groupId, objectName) {}

        boost::future<bool> latch::try_set_count(int32_t count) {
            util::Preconditions::checkPositive(count, "count must be positive!");

            auto request = countdownlatch_trysetcount_encode(group_id_, object_name_, count);
            return invokeAndGetFuture<bool>(request);
        }

        boost::future<int32_t> latch::get_count() {
            auto request = countdownlatch_getcount_encode(group_id_, object_name_);
            return invokeAndGetFuture<int32_t>(request);
        }

        boost::future<void> latch::count_down() {
            auto invocation_uid = getContext().getHazelcastClientImplementation()->random_uuid();
            return get_round().then(boost::launch::deferred,[=] (boost::future<int32_t> f) {
                auto round = f.get();
                for (;;) {
                    try {
                        count_down(round, invocation_uid);
                        return;
                    } catch (exception::OperationTimeoutException &) {
                        // I can retry safely because my retry would be idempotent...
                    }
                }

            });
        }

        boost::future<bool> latch::try_wait() {
            return get_count().then(boost::launch::deferred, [] (boost::future<int32_t> f) {
                return f.get() == 0;
            });
        }

        boost::future<int32_t> latch::get_round() {
            auto request = countdownlatch_getround_encode(group_id_, object_name_);
            return invokeAndGetFuture<int32_t>(request);
        }

        void latch::count_down(int round, boost::uuids::uuid invocation_uid) {
            auto request = countdownlatch_countdown_encode(group_id_, object_name_, invocation_uid, round);
            invoke(request).get();
        }

        boost::future<void> latch::wait() {
            return toVoidFuture(wait_for(std::chrono::hours::max()));
        }

        boost::future<std::cv_status> latch::wait_for(int64_t milliseconds) {
            auto timeout_millis = std::max<int64_t>(0, milliseconds);
            auto invoation_uid = getContext().getHazelcastClientImplementation()->random_uuid();
            auto request = countdownlatch_await_encode(group_id_, object_name_, invoation_uid, timeout_millis);
            return invokeAndGetFuture<bool>(request).then(boost::launch::deferred, [] (boost::future<bool> f) {
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
        fenced_lock::fenced_lock(const std::string &name, spi::ClientContext &context, const raft_group_id &groupId,
                                 const std::string &objectName) : session_aware_proxy(SERVICE_NAME, name, &context,
                                                                                      groupId, objectName,
                                                                                      context.get_proxy_session_manager()) {}

        boost::future<void> fenced_lock::lock() {
            return toVoidFuture(lock_and_get_fence());
        }

        boost::future<int64_t> fenced_lock::lock_and_get_fence() {
            auto thread_id = util::getCurrentThreadId();
            auto invocation_uid = getContext().random_uuid();

            auto do_lock_once = [=] () {
                auto session_id = session_manager_.acquire_session(group_id_);
                verify_locked_session_id_if_present(thread_id, session_id, true);
                return do_lock(session_id, thread_id, invocation_uid).then(boost::launch::deferred, [=] (boost::future<int64_t> f) {
                    try {
                        auto fence = f.get();
                        if (fence != INVALID_FENCE) {
                            locked_session_ids_.put(thread_id, std::make_shared<int64_t>(session_id));
                            return fence;
                        }
                        BOOST_THROW_EXCEPTION(exception::LockAcquireLimitReachedException(
                                                      "fenced_lock::lock_and_get_fence", (boost::format("Lock [%1%] reentrant lock limit is already reached!") %object_name_).str()));
                    } catch(exception::SessionExpiredException &) {
                        invalidate_session(session_id);
                        verify_no_locked_session_id_present(thread_id);
                        return INVALID_FENCE;
                    } catch(exception::WaitKeyCancelledException &) {
                        release_session(session_id);
                        BOOST_THROW_EXCEPTION(exception::LockAcquireLimitReachedException(
                                                      "fenced_lock::lock_and_get_fence", (boost::format("Lock [%1%] not acquired because the lock call on the CP group is cancelled, possibly because of another indeterminate call from the same thread.") %object_name_).str()));
                    }
                });
            };

            return do_lock_once().then(boost::launch::deferred, [=] (boost::future<int64_t> f) {
                auto result = f.get();
                if (result != INVALID_FENCE) {
                    return result;
                }
                // iterate in the user thread
                for (result = do_lock_once().get();result == INVALID_FENCE;) {}
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
            BOOST_THROW_EXCEPTION(client::exception::LockOwnershipLostException("fenced_lock", (boost::format(
                    "Current thread is not owner of the Lock[%1%] because its Session[%2%] is closed by server!") %
                                                                         getName() % session_id).str()));
        }

        void fenced_lock::throw_illegal_monitor_state() const {
            BOOST_THROW_EXCEPTION(client::exception::IllegalMonitorStateException("fenced_lock", (boost::format(
                    "Current thread is not owner of the Lock[%1%]") %getName()).str()));
        }

        boost::future<int64_t>
        fenced_lock::do_lock(int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid) {
            auto request = client::protocol::codec::fencedlock_lock_encode(group_id_, object_name_, session_id, thread_id, invocation_uid);
            return invokeAndGetFuture<int64_t>(request);
        }

        boost::future<int64_t>
        fenced_lock::do_try_lock(int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid,
                                 std::chrono::steady_clock::duration timeout) {
            auto request = client::protocol::codec::fencedlock_trylock_encode(group_id_, object_name_, session_id,
                                                                              thread_id, invocation_uid,
                                                                              std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                                      timeout).count());
            return invokeAndGetFuture<int64_t>(request);
        }

        boost::future<bool>
        fenced_lock::do_unlock(int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid) {
            auto request = client::protocol::codec::fencedlock_unlock_encode(group_id_, object_name_, session_id, thread_id, invocation_uid);
            return invokeAndGetFuture<bool>(request);
        }

        boost::future<fenced_lock::lock_ownership_state> fenced_lock::do_get_lock_ownership_state(){
            auto request = client::protocol::codec::fencedlock_getlockownership_encode(group_id_, object_name_);
            return invoke(request).then(boost::launch::deferred, [] (boost::future<client::protocol::ClientMessage> f) {
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
            return try_lock_and_get_fence().then(boost::launch::deferred, [] (boost::future<int64_t> f) {
                return f.get() != INVALID_FENCE;
            });
        }

        boost::future<bool> fenced_lock::try_lock(std::chrono::steady_clock::duration timeout) {
            return try_lock_and_get_fence(timeout).then(boost::launch::deferred, [] (boost::future<int64_t> f) {
                return f.get() != INVALID_FENCE;
            });
        }

        boost::future<int64_t> fenced_lock::try_lock_and_get_fence() {
            return try_lock_and_get_fence(std::chrono::milliseconds(0));
        }

        boost::future<int64_t>
        fenced_lock::try_lock_and_get_fence(std::chrono::steady_clock::duration timeout) {
            auto thread_id = util::getCurrentThreadId();
            auto invocation_uid = getContext().random_uuid();

            auto do_try_lock_once = [=] () {
                using namespace std::chrono;
                auto start = steady_clock::now();
                auto session_id = session_manager_.acquire_session(group_id_);
                verify_locked_session_id_if_present(thread_id, session_id, true);
                return do_try_lock(session_id, thread_id, invocation_uid, timeout).then(boost::launch::deferred, [=] (boost::future<int64_t> f) {
                    try {
                        auto fence = f.get();
                        if (fence != INVALID_FENCE) {
                            locked_session_ids_.put(thread_id, std::make_shared<int64_t>(session_id));
                            return std::make_pair(fence, false);
                        } else {
                            release_session(session_id);
                        }
                        return std::make_pair(fence, false);
                    } catch(exception::SessionExpiredException &) {
                        invalidate_session(session_id);
                        verify_no_locked_session_id_present(thread_id);
                        auto timeout_left = timeout -  (steady_clock::now() - start);
                        if (timeout_left.count() <= 0) {
                            return std::make_pair(INVALID_FENCE, false);
                        }
                        return std::make_pair(INVALID_FENCE, false);
                    } catch(exception::WaitKeyCancelledException &) {
                        release_session(session_id);
                        return std::make_pair(INVALID_FENCE, false);
                    }
                });
            };

            return do_try_lock_once().then(boost::launch::deferred, [=] (boost::future<std::pair<int64_t, bool>> f) {
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
            auto thread_id = util::getCurrentThreadId();
            int64_t session_id = session_manager_.get_session(group_id_);

            // the order of the following checks is important.
            verify_locked_session_id_if_present(thread_id, session_id, false);
            if (session_id == internal::session::proxy_session_manager::NO_SESSION_ID) {
                locked_session_ids_.remove(thread_id);
                throw_illegal_monitor_state();
            }

            return do_unlock(session_id, thread_id, getContext().random_uuid()).then(boost::launch::deferred, [=] (boost::future<bool> f) {
                try {
                    auto still_locked_by_current_thread = f.get();
                    if (still_locked_by_current_thread) {
                            locked_session_ids_.put(thread_id, std::make_shared<int64_t>(session_id));
                    } else {
                        locked_session_ids_.remove(thread_id);
                    }

                    release_session(session_id);
                } catch (exception::SessionExpiredException &) {
                    invalidate_session(session_id);
                    locked_session_ids_.remove(thread_id);

                    throw_lock_ownership_lost(session_id);
                } catch (exception::IllegalMonitorStateException &) {
                    locked_session_ids_.remove(thread_id);
                    throw;
                }
            });
        }

        boost::future<int64_t> fenced_lock::get_fence() {
            auto thread_id = util::getCurrentThreadId();
            int64_t session_id = session_manager_.get_session(group_id_);

            // the order of the following checks is important.
            verify_locked_session_id_if_present(thread_id, session_id, false);
            if (session_id == internal::session::proxy_session_manager::NO_SESSION_ID) {
                locked_session_ids_.remove(thread_id);
                throw_illegal_monitor_state();
            }

            return do_get_lock_ownership_state().then(boost::launch::deferred, [=] (boost::future<lock_ownership_state> f) {
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
            auto thread_id = util::getCurrentThreadId();
            int64_t session_id = session_manager_.get_session(group_id_);

            verify_locked_session_id_if_present(thread_id, session_id, false);

            return do_get_lock_ownership_state().then(boost::launch::deferred, [=] (boost::future<lock_ownership_state> f) {
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
            auto thread_id = util::getCurrentThreadId();
            int64_t session_id = session_manager_.get_session(group_id_);

            verify_locked_session_id_if_present(thread_id, session_id, false);

            return do_get_lock_ownership_state().then(boost::launch::deferred, [=] (boost::future<lock_ownership_state> f) {
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
            auto thread_id = util::getCurrentThreadId();
            int64_t session_id = session_manager_.get_session(group_id_);

            verify_locked_session_id_if_present(thread_id, session_id, false);

            return do_get_lock_ownership_state().then(boost::launch::deferred, [=] (boost::future<lock_ownership_state> f) {
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
            return lhs.getServiceName() == rhs.getServiceName() && lhs.getName() == rhs.getName();
        }

        void fenced_lock::postDestroy() {
            ClientProxy::postDestroy();
            locked_session_ids_.clear();
        }

        session_aware_proxy::session_aware_proxy(const std::string &serviceName, const std::string &proxyName,
                                                 spi::ClientContext *context, const raft_group_id &groupId,
                                                 const std::string &objectName,
                                                 internal::session::proxy_session_manager &sessionManager) : cp_proxy(
                serviceName, proxyName, context, groupId, objectName), session_manager_(sessionManager) {}

        void session_aware_proxy::release_session(int64_t session_id) {
            session_manager_.release_session(group_id_, session_id);
        }

        bool fenced_lock::lock_ownership_state::is_locked_by(int64_t session, int64_t thread) {
            return is_locked() && session_id == session && thread_id == thread;
        }

        bool fenced_lock::lock_ownership_state::is_locked() {
            return fence != INVALID_FENCE;
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



