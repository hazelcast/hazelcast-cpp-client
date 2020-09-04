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
            return nullptr;
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

        cp_proxy::cp_proxy(const std::string &serviceName, const std::string &proxyName, client::spi::ClientContext *context,
                           const raft_group_id &groupId, const std::string &objectName) : ProxyImpl(serviceName,
                                                                                                    proxyName,
                                                                                                    context),
                                                                                          group_id_(groupId),
                                                                                          object_name_(objectName) {}

        void cp_proxy::onDestroy() {
            auto request = cpgroup_destroycpobject_encode(group_id_, getServiceName(), object_name_);
            invoke(request).get();
        }

        const raft_group_id &cp_proxy::getGroupId() const {
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
            auto request = countdownlatch_trysetcount_encode(group_id_, object_name_, count);
            return invokeAndGetFuture<bool>(request);
        }

        boost::future<int32_t> latch::get_count() {
            auto request = countdownlatch_getcount_encode(group_id_, object_name_);
            return invokeAndGetFuture<int32_t>(request);
        }

        boost::future<void> latch::count_down() {
            auto invocation_uid = getContext().getHazelcastClientImplementation()->random_uuid();
            return get_round().then([=] (boost::future<int32_t> f) {
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
            return get_count().then([] (boost::future<int32_t> f) {
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
            return invokeAndGetFuture<bool>(request).then([] (boost::future<bool> f) {
                return f.get() ? std::cv_status::no_timeout : std::cv_status::timeout;
            });
        }
    }
}

