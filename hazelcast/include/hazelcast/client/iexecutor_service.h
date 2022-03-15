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
#pragma once

#include <vector>
#include <atomic>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/client/member_selectors.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/member.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/util/exception_util.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"

// CODECs
#include "hazelcast/client/protocol/codec/codecs.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        /**
         * Distributed implementation of java.util.concurrent.ExecutorService.
         * IExecutorService provides additional methods like executing tasks
         * on a specific member, on a member who is owner of a specific key,
         * executing a tasks on multiple members.
         *
         */
        class HAZELCAST_API iexecutor_service : public proxy::ProxyImpl {
            friend class spi::ProxyManager;
        public:
            static constexpr const char *SERVICE_NAME = "hz:impl:executorService";

            template<typename T>
            class executor_promise {
            public:
                executor_promise(spi::ClientContext &context) : context_(context) {}

                executor_promise(boost::future<boost::optional<T>> &future, boost::uuids::uuid uuid, int partition_id,
                                 boost::uuids::uuid member, spi::ClientContext &context,
                                 const std::shared_ptr<spi::impl::ClientInvocation> &invocation)
                        : shared_future_(future.share()), uuid_(uuid), partition_id_(partition_id), member_uuid_(member),
                          context_(context), invocation_(invocation) {}

                bool cancel(bool may_interrupt_if_running) {
                    if (shared_future_.is_ready()) {
                        return false;
                    }

                    try {
                        return invoke_cancel_request(may_interrupt_if_running);
                    } catch (exception::iexception &) {
                        util::exception_util::rethrow(std::current_exception());
                    }
                    return false;
                }

                boost::shared_future<boost::optional<T>> get_future() {
                    return shared_future_;
                }

            private:
                boost::shared_future<boost::optional<T>> shared_future_;
                boost::uuids::uuid uuid_;
                int partition_id_;
                boost::uuids::uuid member_uuid_;
                spi::ClientContext &context_;
                std::shared_ptr<spi::impl::ClientInvocation> invocation_;

                bool invoke_cancel_request(bool may_interrupt_if_running) {
                    invocation_->wait_invoked();

                    if (partition_id_ > -1) {
                        auto request = protocol::codec::executorservice_cancelonpartition_encode(uuid_, may_interrupt_if_running);
                        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                                context_, request, boost::uuids::to_string(uuid_), partition_id_);
                        return clientInvocation->invoke().get().get_first_fixed_sized_field<bool>();
                    } else {
                        auto request = protocol::codec::executorservice_cancelonmember_encode(
                                uuid_, member_uuid_, may_interrupt_if_running);
                        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                                context_, request, boost::uuids::to_string(uuid_), member_uuid_);
                        return clientInvocation->invoke().get().get_first_fixed_sized_field<bool>();
                    }
                }
            };

            /**
             * Executes the given command at some time in the future.  The command
             * may execute in a new thread, in a pooled thread, or in the calling
             * thread, at the discretion of the {@code Executor} implementation.
             *
             * @param command the runnable task
             * @throws rejected_execution if this task cannot be
             * accepted for execution
             */
            template<typename HazelcastSerializable>
            void execute(const HazelcastSerializable &command) {
                submit<HazelcastSerializable, executor_marker>(command);
            }

            /**
             * Executes a task on a randomly selected member.
             *
             * @param command        the task that is executed on a randomly selected member
             * @param memberSelector memberSelector
             * @throws rejected_execution if no member is selected
             */
            template<typename HazelcastSerializable>
            void execute(const HazelcastSerializable &command,
                         const member_selector &member_selector) {
                std::vector<member> members = select_members(member_selector);
                int selectedMember = rand() % (int) members.size();
                execute_on_member<HazelcastSerializable>(command, members[selectedMember]);
            }

            /**
             * Executes a task on the owner of the specified key.
             *
             * @param command a task executed on the owner of the specified key
             * @param key     the specified key
             */
            template<typename HazelcastSerializable, typename K>
            void execute_on_key_owner(const HazelcastSerializable &command, const K &key) {
                submit_to_key_owner<HazelcastSerializable, K>(command, key);
            }

            /**
             * Executes a task on the specified member.
             *
             * @param command the task executed on the specified member
             * @param member  the specified member
             */
            template<typename HazelcastSerializable>
            void execute_on_member(const HazelcastSerializable &command, const member &member) {
                submit_to_member<HazelcastSerializable, executor_marker>(command, member);
            }

            /**
             * Executes a task on each of the specified members.
             *
             * @param command the task executed on the specified members
             * @param members the specified members
             */
            template<typename HazelcastSerializable>
            void execute_on_members(const HazelcastSerializable &command, const std::vector<member> &members) {
                for (std::vector<member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    submit_to_member<HazelcastSerializable, executor_marker>(command, *it);
                }
            }

            /**
             * Executes a task on each of the selected members.
             *
             * @param command        a task executed on each of the selected members
             * @param memberSelector memberSelector
             * @throws rejected_execution if no member is selected
             */
            template<typename HazelcastSerializable>
            void execute_on_members(const HazelcastSerializable &command,
                                  const member_selector &member_selector) {
                std::vector<member> members = select_members(member_selector);
                execute_on_members<HazelcastSerializable>(command, members);
            }

            /**
             * Executes a task on all of the known cluster members.
             *
             * @param command a task executed  on all of the known cluster members
             */
            template<typename HazelcastSerializable>
            void execute_on_all_members(const HazelcastSerializable &command) {
                std::vector<member> memberList = get_context().get_client_cluster_service().get_member_list();
                for (std::vector<member>::const_iterator it = memberList.begin(); it != memberList.end(); ++it) {
                    submit_to_member<HazelcastSerializable, executor_marker>(command, *it);
                }
            }

            /**
             * Submits a task to the owner of the specified key and returns a executor_promise
             * representing that task.
             *
             * @param task task submitted to the owner of the specified key
             * @param key  the specified key
             * @param <T>  the result type of callable
             * @return a executor_promise representing pending completion of the task
             */
            template<typename HazelcastSerializable, typename T, typename K>
            executor_promise<T>
            submit_to_key_owner(const HazelcastSerializable &task, const K &key) {
                return submit_to_key_owner_internal<HazelcastSerializable, T, K>(task, key, false);
            }

            /**
             * Submits a task to the specified member and returns a executor_promise
             * representing that task.
             *
             * @param task   the task submitted to the specified member
             * @param member the specified member
             * @param <T>    the result type of callable
             * @return a executor_promise representing pending completion of the task
             */
            template<typename HazelcastSerializable, typename T>
            executor_promise<T>
            submit_to_member(const HazelcastSerializable &task, const member &member) {
                return submit_to_target_internal<HazelcastSerializable, T>(task, member, false);
            }

            /**
             * Submits a task to given members and returns
             * map of Member-executor_promise pairs representing pending completion of the task on each member
             *
             * @param task    the task submitted to given members
             * @param members the given members
             * @param <T>     the result type of callable
             * @return map of Member-executor_promise pairs representing pending completion of the task on each member
             */
            template<typename HazelcastSerializable, typename T>
            std::unordered_map<member, executor_promise<T>>
            submit_to_members(const HazelcastSerializable &task, const std::vector<member> &members) {
                std::unordered_map<member, executor_promise<T>> futureMap;
                for (auto &member : members) {
                    auto f = submit_to_target_internal<HazelcastSerializable, T>(task, member, true);
                    // no need to check if emplace is success since member is unique
                    futureMap.emplace(member, std::move(f));
                }
                return futureMap;
            }

            /**
             * Submits a task to selected members and returns a
             * map of Member-executor_promise pairs representing pending completion of the task on each member.
             *
             * @param task           the task submitted to selected members
             * @param memberSelector memberSelector
             * @param <T>            the result type of callable
             * @return map of Member-executor_promise pairs representing pending completion of the task on each member
             * @throws rejected_execution if no member is selected
             */
            template<typename HazelcastSerializable, typename T>
            std::unordered_map<member, executor_promise<T>>
            submit_to_members(const HazelcastSerializable &task,
                            const member_selector &member_selector) {
                std::vector<member> members = select_members(member_selector);
                return submit_to_members<HazelcastSerializable, T>(task, members);
            }

            /**
             * Submits task to all cluster members and returns a
             * map of Member-executor_promise pairs representing pending completion of the task on each member.
             *
             * @param task the task submitted to all cluster members
             * @param <T>  the result type of callable
             * @return map of Member-executor_promise pairs representing pending completion of the task on each member
             */
            template<typename HazelcastSerializable, typename T>
            std::unordered_map<member, executor_promise<T>>
            submit_to_all_members(const HazelcastSerializable &task) {
                std::unordered_map<member, executor_promise<T>> futureMap;
                for (const auto &m : get_context().get_client_cluster_service().get_member_list()) {
                    auto f = submit_to_target_internal<HazelcastSerializable, T>(task, m, true);
                    // no need to check if emplace is success since member is unique
                    futureMap.emplace(m, std::move(f));
                }
                return futureMap;
            }

            /**
             * Submits a task for execution and returns a executor_promise
             * representing that task. The executor_promise's {@code executor_promise::get} method will
             * return the given result upon successful completion.
             *
             * @param task the task to submit
             * @param result the result to return
             * @param <T> the type of the result
             * @return a executor_promise representing pending completion of the task
             * @throws rejected_execution if the task cannot be
             *         scheduled for execution
             * @throws null_pointer if the task is null
             */
            template<typename HazelcastSerializable, typename T>
            executor_promise<T>
            submit(const HazelcastSerializable &task) {
                serialization::pimpl::data task_data = to_data<HazelcastSerializable>(task);

                if (task_data.has_partition_hash()) {
                    int partitionId = get_partition_id(task_data);

                    return submit_to_partition_internal<T>(task_data, false, partitionId);
                } else {
                    return submit_to_random_internal<T>(task_data, false);
                }
            }

            /**
             * Submits a task to a randomly selected member and returns a executor_promise
             * representing that task.
             *
             * @param task           task submitted to a randomly selected member
             * @param memberSelector memberSelector
             * @param <T>            the result type of callable
             * @return a executor_promise representing pending completion of the task
             * @throws rejected_execution if no member is selected
             */
            template<typename HazelcastSerializable, typename T>
            executor_promise<T>
            submit(const HazelcastSerializable &task, const member_selector &member_selector) {
                std::vector<member> members = select_members(member_selector);
                int selectedMember = rand() % (int) members.size();
                return submit_to_member<HazelcastSerializable, T>(task, members[selectedMember]);
            }

            /**
             * Initiates an orderly shutdown in which previously submitted
             * tasks are executed, but no new tasks will be accepted.
             * Invocation has no additional effect if already shut down.
             *
             * <p>This method does not wait for previously submitted tasks to
             * complete execution.
             *
             */
            void shutdown();

            /**
             * Returns {@code true} if this executor has been shut down.
             *
             * @return {@code true} if this executor has been shut down
             */
            boost::future<bool> is_shutdown();

            /**
             * Returns {@code true} if all tasks have completed following shut down.
             * Note that {@code isTerminated} is never {@code true} unless
             * either {@code shutdown}.
             *
             * @return {@code true} if all tasks have completed following shut down
             */
            boost::future<bool> is_terminated();

        private:
            iexecutor_service(const std::string &name, spi::ClientContext *context);

            struct executor_marker {};

            std::vector<member> select_members(const member_selector &member_selector);

            template<typename T>
            executor_promise<T>
            submit_to_partition_internal(const serialization::pimpl::data &task_data, bool prevent_sync, int partition_id) {
                auto uuid = context_.random_uuid();

                auto f = invoke_on_partition_internal(task_data, partition_id, uuid);

                return check_sync<T>(f, uuid, partition_id, prevent_sync);
            }

            std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
            invoke_on_partition_internal(const serialization::pimpl::data &task_data, int partition_id,
                                      boost::uuids::uuid uuid) {
                return invoke_on_partition_owner(
                        protocol::codec::executorservice_submittopartition_encode(name_, uuid, task_data), partition_id);
            }

            template<typename HazelcastSerializable, typename T, typename K>
            executor_promise<T>
            submit_to_key_owner_internal(const HazelcastSerializable &task, const K &key, bool prevent_sync) {

                serialization::pimpl::data dataKey = to_data<K>(key);

                int partitionId = get_partition_id(dataKey);

                return submit_to_partition_internal<T>(to_data<HazelcastSerializable>(task), prevent_sync, partitionId);
            }

            template<typename T>
            executor_promise<T>
            submit_to_random_internal(const serialization::pimpl::data &task_data, bool prevent_sync) {

                int partitionId = random_partition_id();

                return submit_to_partition_internal<T>(task_data, prevent_sync, partitionId);
            }

            template<typename HazelcastSerializable, typename T>
            executor_promise<T> submit_to_target_internal(const HazelcastSerializable &task, const member &member,
                                                       bool prevent_sync) {
                boost::uuids::uuid uuid = context_.random_uuid();

                auto f = invoke_on_target_internal<HazelcastSerializable>(task, member, uuid);

                return check_sync<T>(f, uuid, -1, member, prevent_sync);
            }

            template<typename HazelcastSerializable>
            std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
            invoke_on_target_internal(const HazelcastSerializable &task, const member &member,
                                   boost::uuids::uuid uuid) {
                return invoke_on_target(
                        protocol::codec::executorservice_submittomember_encode(name_, uuid, to_data(task), member.get_uuid()),
                        member.get_uuid());
            }

            std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
            invoke_on_partition_owner(protocol::ClientMessage &&request, int partition_id);

            std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
            invoke_on_target(protocol::ClientMessage &&request, boost::uuids::uuid target);

            template<typename T>
            boost::optional<T>
            retrieve_result_from_message(serialization::pimpl::SerializationService *serialization_service,
                    boost::future<protocol::ClientMessage> f) {
                auto msg = f.get();
                msg.skip_frame();
                return serialization_service->to_object<T>(msg.get_nullable<serialization::pimpl::data>().get_ptr());
            }

            template<typename T>
            executor_promise<T>
            check_sync(
                    std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>> &future_pair,
                    boost::uuids::uuid uuid, int partition_id, bool prevent_sync) {
                return check_sync<T>(future_pair, uuid, partition_id, member(), prevent_sync);
            }

            template<typename T>
            boost::future<boost::optional<T>>
            retrieve_result_sync(boost::future<protocol::ClientMessage> future) {
                try {
                    auto response = retrieve_result_from_message<T>(&(get_serialization_service()), std::move(future));
                    return boost::make_ready_future(response);
                } catch (exception::iexception &) {
                    return boost::make_exceptional_future<boost::optional<T>>(boost::current_exception());
                }
            }

            template<typename T>
            typename std::enable_if<!std::is_same<executor_marker, T>::value, executor_promise<T>>::type
            check_sync(
                    std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>> &future_pair,
                    boost::uuids::uuid uuid, int partition_id, const member &member, bool prevent_sync) {
                bool sync = is_sync_computation(prevent_sync);
                boost::future<boost::optional<T>> objectFuture;
                if (sync) {
                    objectFuture = retrieve_result_sync<T>(std::move(future_pair.first));
                } else {
                    serialization::pimpl::SerializationService *serializationService = &get_serialization_service();
                    objectFuture = future_pair.first.then(boost::launch::sync,
                                                         [=](boost::future<protocol::ClientMessage> f) {
                                                             return retrieve_result_from_message<T>(
                                                                     serializationService, std::move(f));
                                                         });
                }

                return executor_promise<T>(objectFuture, uuid, partition_id, member.get_uuid(), get_context(), future_pair.second);
            }

            template<typename T>
            typename std::enable_if<std::is_same<executor_marker, T>::value, executor_promise<T>>::type
            check_sync(
                    std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>> &future_pair,
                    boost::uuids::uuid uuid, int partition_id, const member &member, bool prevent_sync) {
                bool sync = is_sync_computation(prevent_sync);
                if (sync) {
                    future_pair.first.get();
                }

                return executor_promise<T>(get_context());
            }

            bool is_sync_computation(bool prevent_sync);

            address get_member_address(const member &member);

            int random_partition_id();

            static const int32_t MIN_TIME_RESOLUTION_OF_CONSECUTIVE_SUBMITS = 10;
            static const int32_t MAX_CONSECUTIVE_SUBMITS = 100;

            std::atomic<int32_t> consecutive_submits_;
            std::atomic<int64_t> last_submit_time_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


