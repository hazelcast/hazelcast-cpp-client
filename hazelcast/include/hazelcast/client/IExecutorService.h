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
#pragma once

#include <vector>
#include <atomic>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/cluster/memberselector/MemberSelectors.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/MultiExecutionCallback.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/util/ExceptionUtil.h"
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
         * executing a tasks on multiple members and listening execution result using a callback.
         *
         *
         * @see ExecutionCallback
         * @see MultiExecutionCallback
         */
        class HAZELCAST_API IExecutorService : public proxy::ProxyImpl {
            friend class spi::ProxyManager;
        public:
            static constexpr const char *SERVICE_NAME = "hz:impl:executorService";

            template<typename T>
            class executor_promise {
            public:
                executor_promise(spi::ClientContext &context) : context_(context) {}

                executor_promise(boost::future<boost::optional<T>> &future, boost::uuids::uuid uuid, int partitionId,
                                 boost::uuids::uuid member, spi::ClientContext &context,
                                 const std::shared_ptr<spi::impl::ClientInvocation> &invocation)
                        : sharedFuture_(future.share()), uuid_(uuid), partitionId_(partitionId), memberUuid_(member),
                          context_(context), invocation_(invocation) {}

                bool cancel(bool mayInterruptIfRunning) {
                    if (sharedFuture_.is_ready()) {
                        return false;
                    }

                    try {
                        return invokeCancelRequest(mayInterruptIfRunning);
                    } catch (exception::IException &) {
                        util::ExceptionUtil::rethrow(std::current_exception());
                    }
                    return false;
                }

                boost::shared_future<boost::optional<T>> get_future() {
                    return sharedFuture_;
                }

            private:
                boost::shared_future<boost::optional<T>> sharedFuture_;
                boost::uuids::uuid uuid_;
                int partitionId_;
                boost::uuids::uuid memberUuid_;
                spi::ClientContext &context_;
                std::shared_ptr<spi::impl::ClientInvocation> invocation_;

                bool invokeCancelRequest(bool mayInterruptIfRunning) {
                    invocation_->getSendConnectionOrWait();

                    if (partitionId_ > -1) {
                        auto request = protocol::codec::executorservice_cancelonpartition_encode(uuid_, mayInterruptIfRunning);
                        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                                context_, request, boost::uuids::to_string(uuid_), partitionId_);
                        return clientInvocation->invoke().get().get_first_fixed_sized_field<bool>();
                    } else {
                        auto request = protocol::codec::executorservice_cancelonmember_encode(
                                uuid_, memberUuid_, mayInterruptIfRunning);
                        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                                context_, request, boost::uuids::to_string(uuid_), memberUuid_);
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
             * @throws RejectedExecutionException if this task cannot be
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
             * @throws RejectedExecutionException if no member is selected
             */
            template<typename HazelcastSerializable>
            void execute(const HazelcastSerializable &command,
                         const cluster::memberselector::MemberSelector &memberSelector) {
                std::vector<Member> members = selectMembers(memberSelector);
                int selectedMember = rand() % (int) members.size();
                executeOnMember<HazelcastSerializable>(command, members[selectedMember]);
            }

            /**
             * Executes a task on the owner of the specified key.
             *
             * @param command a task executed on the owner of the specified key
             * @param key     the specified key
             */
            template<typename HazelcastSerializable, typename K>
            void executeOnKeyOwner(const HazelcastSerializable &command, const K &key) {
                submitToKeyOwner<HazelcastSerializable, K>(command, key);
            }

            /**
             * Executes a task on the specified member.
             *
             * @param command the task executed on the specified member
             * @param member  the specified member
             */
            template<typename HazelcastSerializable>
            void executeOnMember(const HazelcastSerializable &command, const Member &member) {
                submitToMember<HazelcastSerializable, executor_marker>(command, member);
            }

            /**
             * Executes a task on each of the specified members.
             *
             * @param command the task executed on the specified members
             * @param members the specified members
             */
            template<typename HazelcastSerializable>
            void executeOnMembers(const HazelcastSerializable &command, const std::vector<Member> &members) {
                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    submitToMember<HazelcastSerializable, executor_marker>(command, *it);
                }
            }

            /**
             * Executes a task on each of the selected members.
             *
             * @param command        a task executed on each of the selected members
             * @param memberSelector memberSelector
             * @throws RejectedExecutionException if no member is selected
             */
            template<typename HazelcastSerializable>
            void executeOnMembers(const HazelcastSerializable &command,
                                  const cluster::memberselector::MemberSelector &memberSelector) {
                std::vector<Member> members = selectMembers(memberSelector);
                executeOnMembers<HazelcastSerializable>(command, members);
            }

            /**
             * Executes a task on all of the known cluster members.
             *
             * @param command a task executed  on all of the known cluster members
             */
            template<typename HazelcastSerializable>
            void executeOnAllMembers(const HazelcastSerializable &command) {
                std::vector<Member> memberList = getContext().getClientClusterService().getMemberList();
                for (std::vector<Member>::const_iterator it = memberList.begin(); it != memberList.end(); ++it) {
                    submitToMember<HazelcastSerializable, executor_marker>(command, *it);
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
            submitToKeyOwner(const HazelcastSerializable &task, const K &key) {
                return submitToKeyOwnerInternal<HazelcastSerializable, T, K>(task, key, false);
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
            submitToMember(const HazelcastSerializable &task, const Member &member) {
                return submitToTargetInternal<HazelcastSerializable, T>(task, member, false);
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
            std::unordered_map<Member, executor_promise<T>>
            submitToMembers(const HazelcastSerializable &task, const std::vector<Member> &members) {
                std::unordered_map<Member, executor_promise<T>> futureMap;
                for (auto &member : members) {
                    auto f = submitToTargetInternal<HazelcastSerializable, T>(task, member, true);
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
             * @throws RejectedExecutionException if no member is selected
             */
            template<typename HazelcastSerializable, typename T>
            std::unordered_map<Member, executor_promise<T>>
            submitToMembers(const HazelcastSerializable &task,
                            const cluster::memberselector::MemberSelector &memberSelector) {
                std::vector<Member> members = selectMembers(memberSelector);
                return submitToMembers<HazelcastSerializable, T>(task, members);
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
            std::unordered_map<Member, executor_promise<T>>
            submitToAllMembers(const HazelcastSerializable &task) {
                std::unordered_map<Member, executor_promise<T>> futureMap;
                for (const auto &m : getContext().getClientClusterService().getMemberList()) {
                    auto f = submitToTargetInternal<HazelcastSerializable, T>(task, m, true);
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
             * @throws RejectedExecutionException if the task cannot be
             *         scheduled for execution
             * @throws NullPointerException if the task is null
             */
            template<typename HazelcastSerializable, typename T>
            executor_promise<T>
            submit(const HazelcastSerializable &task) {
                Data taskData = toData<HazelcastSerializable>(task);

                if (taskData.hasPartitionHash()) {
                    int partitionId = getPartitionId(taskData);

                    return submitToPartitionInternal<T>(taskData, false, partitionId);
                } else {
                    return submitToRandomInternal<T>(taskData, false);
                }
            }

            /**
             * Submits a task to a random member. Caller will be notified of the result of the task by
             * {@link ExecutionCallback<T>::onResponse()} or {@link ExecutionCallback<T>::onFailure(exception::IException)}.
             *
             * @param task     a task submitted to a random member
             * @param callback callback
             * @param <T>      the response type of callback
             */
            template<typename HazelcastSerializable, typename T>
            void submit(const HazelcastSerializable &task, const std::shared_ptr<ExecutionCallback<T> > &callback) {
                Data taskData = toData<HazelcastSerializable>(task);

                if (taskData.hasPartitionHash()) {
                    int partitionId = getPartitionId(taskData);

                    submitToPartitionInternal<T>(taskData, partitionId, callback);
                } else {
                    submitToRandomInternal<T>(taskData, callback);
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
             * @throws RejectedExecutionException if no member is selected
             */
            template<typename HazelcastSerializable, typename T>
            executor_promise<T>
            submit(const HazelcastSerializable &task, const cluster::memberselector::MemberSelector &memberSelector) {
                std::vector<Member> members = selectMembers(memberSelector);
                int selectedMember = rand() % (int) members.size();
                return submitToMember<HazelcastSerializable, T>(task, members[selectedMember]);
            }

            /**
             * Submits a task to randomly selected members. Caller will be notified for the result of the task by
             * {@link ExecutionCallback<T>::onResponse()} or {@link ExecutionCallback<T>::onFailure(exception::IException)}.
             *
             * @param task           the task submitted to randomly selected members
             * @param memberSelector memberSelector
             * @param callback       callback
             * @param <T>            the response type of callback
             * @throws RejectedExecutionException if no member is selected
             */
            template<typename HazelcastSerializable, typename T>
            void
            submit(const HazelcastSerializable &task, const cluster::memberselector::MemberSelector &memberSelector,
                   const std::shared_ptr<ExecutionCallback<T> > &callback) {
                std::vector<Member> members = selectMembers(memberSelector);
                int selectedMember = rand() % (int) members.size();
                return submitToMember(task, members[selectedMember], callback);
            }

            /**
             * Submits a task to the owner of the specified key. Caller will be notified for the result of the task by
             * {@link ExecutionCallback<T>::onResponse()} or {@link ExecutionCallback<T>::onFailure(exception::IException)}.
             *
             * @param task     task submitted to the owner of the specified key
             * @param key      the specified key
             * @param callback callback
             * @param <T>      the response type of callback
             */
            template<typename HazelcastSerializable, typename T, typename K>
            void submitToKeyOwner(const HazelcastSerializable &task, const K &key,
                                  const std::shared_ptr<ExecutionCallback<T> > &callback) {
                submitToKeyOwnerInternal<HazelcastSerializable, T, K>(task, key, callback);
            }

            /**
             * Submits a task to the specified member. Caller will be notified for the result of the task by
             * {@link ExecutionCallback<T>::onResponse()} or {@link ExecutionCallback<T>::onFailure(exception::IException)}.
             *
             * @param task     the task submitted to the specified member
             * @param member   the specified member
             * @param callback callback
             * @param <T>      the response type of callback
             */
            template<typename HazelcastSerializable, typename T>
            void submitToMember(const HazelcastSerializable &task, const Member &member,
                                const std::shared_ptr<ExecutionCallback<T> > &callback) {
                return submitToTargetInternal<HazelcastSerializable, T>(task, member, callback);
            }

            /**
             * Submits a task to the specified members. Caller will be notified for the result of the each task by
             * {@link MultiExecutionCallback#onResponse(Member, Object)}, and when all tasks are completed,
             * {@link MultiExecutionCallback#onComplete(std::vector)} will be called.
             *
             * @param task     the task submitted to the specified members
             * @param members  the specified members
             * @param callback callback
             */
            template<typename HazelcastSerializable, typename T>
            void submitToMembers(const HazelcastSerializable &task, const std::vector<Member> &members,
                                 const std::shared_ptr<MultiExecutionCallback<T> > &callback) {
                std::shared_ptr<MultiExecutionCallbackWrapper < T> >
                multiExecutionCallbackWrapper(new MultiExecutionCallbackWrapper<T>((int) members.size(), callback));

                for (auto &member : members) {
                    std::shared_ptr<ExecutionCallbackWrapper < T> >
                    executionCallback(new ExecutionCallbackWrapper<T>(multiExecutionCallbackWrapper, member));
                    submitToMember<HazelcastSerializable, T>(task, member, executionCallback);
                }
            }

            /**
             * Submits task to the selected members. Caller will be notified for the result of the each task by
             * {@link MultiExecutionCallback#onResponse(Member, Object)}, and when all tasks are completed,
             * {@link MultiExecutionCallback#onComplete(std::unordered_map)} will be called.
             *
             * @param task           the task submitted to the selected members
             * @param memberSelector memberSelector
             * @param callback       callback
             * @throws RejectedExecutionException if no member is selected
             */
            template<typename HazelcastSerializable, typename T>
            void submitToMembers(const HazelcastSerializable &task,
                                 const cluster::memberselector::MemberSelector &memberSelector,
                                 const std::shared_ptr<MultiExecutionCallback<T> > &callback) {
                std::vector<Member> members = selectMembers(memberSelector);
                submitToMembers<HazelcastSerializable, T>(task, members, callback);
            }

            /**
             * Submits task to all the cluster members. Caller will be notified for the result of each task by
             * {@link MultiExecutionCallback#onResponse(const Member &, const std::shared_ptr<V> &)}, and when all tasks are completed,
             * {@link MultiExecutionCallback#onComplete(std::unordered_map)} will be called.
             *
             * @param task     the task submitted to all the cluster members
             * @param callback callback
             */
            template<typename HazelcastSerializable, typename T>
            void submitToAllMembers(const HazelcastSerializable &task,
                                    const std::shared_ptr<MultiExecutionCallback<T> > &callback) {
                std::vector<Member> memberList = getContext().getClientClusterService().getMemberList();
                submitToMembers<HazelcastSerializable, T>(task, memberList, callback);
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
            boost::future<bool> isShutdown();

            /**
             * Returns {@code true} if all tasks have completed following shut down.
             * Note that {@code isTerminated} is never {@code true} unless
             * either {@code shutdown}.
             *
             * @return {@code true} if all tasks have completed following shut down
             */
            boost::future<bool> isTerminated();

        private:
            IExecutorService(const std::string &name, spi::ClientContext *context);

            struct executor_marker {};

            template<typename T>
            class MultiExecutionCallbackWrapper : MultiExecutionCallback<T> {
            public:
                MultiExecutionCallbackWrapper(
                        int memberSize, const std::shared_ptr<MultiExecutionCallback<T> > &multiExecutionCallback)
                        : multiExecutionCallback_(multiExecutionCallback), members_(memberSize) {
                }

            public:
                void onResponse(const Member &member, const boost::optional<T> &value) override {
                    multiExecutionCallback_->onResponse(member, value);

                    std::lock_guard<std::mutex> guard(lock_);
                    values_[member] = value;
                    int waitingResponse = --members_;
                    if (waitingResponse == 0) {
                        onComplete(values_, exceptions_);
                    }
                }

                void
                onFailure(const Member &member, std::exception_ptr exception) override {
                    multiExecutionCallback_->onFailure(member, exception);

                    std::lock_guard<std::mutex> guard(lock_);
                    exceptions_[member] = exception;
                    int waitingResponse = --members_;
                    if (waitingResponse == 0) {
                        onComplete(values_, exceptions_);
                    }
                }

                void onComplete(const std::unordered_map<Member, boost::optional<T> > &vals,
                                        const std::unordered_map<Member, std::exception_ptr> &excs) override {
                    multiExecutionCallback_->onComplete(vals, excs);
                }

            private:

                const std::shared_ptr<MultiExecutionCallback<T> > multiExecutionCallback_;
                // TODO: We may not need thread safe structures here if being used from the same thread
                std::unordered_map<Member, boost::optional<T>> values_;
                std::unordered_map<Member, std::exception_ptr> exceptions_;
                int members_;
                std::mutex lock_;
            };

            template<typename T>
            class ExecutionCallbackWrapper : public ExecutionCallback<T> {
            public:
                ExecutionCallbackWrapper(
                        const std::shared_ptr<MultiExecutionCallbackWrapper<T> > &multiExecutionCallbackWrapper,
                        Member member) : multiExecutionCallbackWrapper_(multiExecutionCallbackWrapper),
                                                member_(std::move(member)) {}

                void onResponse(const boost::optional<T> &response) override {
                    multiExecutionCallbackWrapper_->onResponse(member_, response);
                }

                void onFailure(std::exception_ptr e) override {
                    multiExecutionCallbackWrapper_->onFailure(member_, e);
                }

            private:
                const std::shared_ptr<MultiExecutionCallbackWrapper<T> > multiExecutionCallbackWrapper_;
                const Member member_;
            };

            std::vector<Member> selectMembers(const cluster::memberselector::MemberSelector &memberSelector);

            template<typename T>
            executor_promise<T>
            submitToPartitionInternal(const serialization::pimpl::Data &taskData, bool preventSync, int partitionId) {
                auto uuid = context_.random_uuid();

                auto f = invokeOnPartitionInternal(taskData, partitionId, uuid);

                return checkSync<T>(f, uuid, partitionId, preventSync);
            }

            template<typename T>
            void submitToPartitionInternal(const serialization::pimpl::Data &taskData, int partitionId,
                                           const std::shared_ptr<ExecutionCallback<T> > &callback) {
                boost::uuids::uuid uuid = context_.random_uuid();

                auto messageFuture = invokeOnPartitionInternal(taskData, partitionId, uuid);

                serialization::pimpl::SerializationService *serializationService = &getSerializationService();
                spi::impl::ClientExecutionServiceImpl *executionService = &getContext().getClientExecutionService();
                messageFuture.first.then(boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
                    try {
                        auto result = retrieveResultFromMessage<T>(serializationService, std::move(f));
                        executionService->execute([=]() { callback->onResponse(result); });
                    } catch (exception::IException &) {
                        auto exception = std::current_exception();
                        executionService->execute([=]() { callback->onFailure(exception); });
                    }
                });
            }

            std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
            invokeOnPartitionInternal(const serialization::pimpl::Data &taskData, int partitionId,
                                      boost::uuids::uuid uuid) {
                return invokeOnPartitionOwner(
                        protocol::codec::executorservice_submittopartition_encode(name_, uuid, taskData), partitionId);
            }

            template<typename HazelcastSerializable, typename T, typename K>
            executor_promise<T>
            submitToKeyOwnerInternal(const HazelcastSerializable &task, const K &key, bool preventSync) {

                Data dataKey = toData<K>(key);

                int partitionId = getPartitionId(dataKey);

                return submitToPartitionInternal<T>(toData<HazelcastSerializable>(task), preventSync, partitionId);
            }

            template<typename HazelcastSerializable, typename T, typename K>
            void submitToKeyOwnerInternal(const HazelcastSerializable &task, const K &key,
                                          const std::shared_ptr<ExecutionCallback<T> > &callback) {

                Data dataKey = toData<K>(key);

                int partitionId = getPartitionId(dataKey);

                submitToPartitionInternal<T>(toData<HazelcastSerializable>(task), partitionId, callback);
            }

            template<typename T>
            executor_promise<T>
            submitToRandomInternal(const serialization::pimpl::Data &taskData, bool preventSync) {

                int partitionId = randomPartitionId();

                return submitToPartitionInternal<T>(taskData, preventSync, partitionId);
            }

            template<typename T>
            void submitToRandomInternal(const serialization::pimpl::Data &taskData,
                                        const std::shared_ptr<ExecutionCallback<T> > &callback) {

                int partitionId = randomPartitionId();

                submitToPartitionInternal<T>(taskData, partitionId, callback);
            }

            template<typename HazelcastSerializable, typename T>
            executor_promise<T> submitToTargetInternal(const HazelcastSerializable &task, const Member &member,
                                                       bool preventSync) {
                boost::uuids::uuid uuid = context_.random_uuid();

                auto f = invokeOnTargetInternal<HazelcastSerializable>(task, member, uuid);

                return checkSync<T>(f, uuid, -1, member, preventSync);
            }

            template<typename HazelcastSerializable, typename T>
            void submitToTargetInternal(const HazelcastSerializable &task, const Member &member,
                                        const std::shared_ptr<ExecutionCallback<T> > &callback) {
                boost::uuids::uuid uuid = context_.random_uuid();

                auto messageFuture = invokeOnTargetInternal<HazelcastSerializable>(task, member, uuid);

                serialization::pimpl::SerializationService *serializationService = &(getSerializationService());
                spi::impl::ClientExecutionServiceImpl *executionService = &getContext().getClientExecutionService();
                messageFuture.first.then(boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
                    try {
                        auto result = retrieveResultFromMessage<T>(serializationService, std::move(f));
                        executionService->execute([=]() { callback->onResponse(result); });
                    } catch (exception::IException &) {
                        auto exception = std::current_exception();
                        executionService->execute([=]() { callback->onFailure(exception); });
                    }
                });
            }

            template<typename HazelcastSerializable>
            std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
            invokeOnTargetInternal(const HazelcastSerializable &task, const Member &member,
                                   boost::uuids::uuid uuid) {
                return invokeOnTarget(
                        protocol::codec::executorservice_submittomember_encode(name_, uuid, toData(task), member.getUuid()),
                        member.getUuid());
            }

            std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
            invokeOnPartitionOwner(protocol::ClientMessage &&request, int partitionId);

            std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
            invokeOnTarget(protocol::ClientMessage &&request, boost::uuids::uuid target);

            template<typename T>
            boost::optional<T>
            retrieveResultFromMessage(serialization::pimpl::SerializationService *serializationService,
                    boost::future<protocol::ClientMessage> f) {
                auto msg = f.get();
                msg.skip_frame();
                return serializationService->toObject<T>(msg.getNullable<serialization::pimpl::Data>().get_ptr());
            }

            template<typename T>
            executor_promise<T>
            checkSync(
                    std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>> &futurePair,
                    boost::uuids::uuid uuid, int partitionId, bool preventSync) {
                return checkSync<T>(futurePair, uuid, partitionId, Member(), preventSync);
            }

            template<typename T>
            boost::future<boost::optional<T>>
            retrieveResultSync(boost::future<protocol::ClientMessage> future) {
                try {
                    auto response = retrieveResultFromMessage<T>(&(getSerializationService()), std::move(future));
                    return boost::make_ready_future(response);
                } catch (exception::IException &) {
                    return boost::make_exceptional_future<boost::optional<T>>(boost::current_exception());
                }
            }

            template<typename T>
            typename std::enable_if<!std::is_same<executor_marker, T>::value, executor_promise<T>>::type
            checkSync(
                    std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>> &futurePair,
                    boost::uuids::uuid uuid, int partitionId, const Member &member, bool preventSync) {
                bool sync = isSyncComputation(preventSync);
                boost::future<boost::optional<T>> objectFuture;
                if (sync) {
                    objectFuture = retrieveResultSync<T>(std::move(futurePair.first));
                } else {
                    serialization::pimpl::SerializationService *serializationService = &getSerializationService();
                    objectFuture = futurePair.first.then(boost::launch::sync,
                                                         [=](boost::future<protocol::ClientMessage> f) {
                                                             return retrieveResultFromMessage<T>(
                                                                     serializationService, std::move(f));
                                                         });
                }

                return executor_promise<T>(objectFuture, uuid, partitionId, member.getUuid(), getContext(), futurePair.second);
            }

            template<typename T>
            typename std::enable_if<std::is_same<executor_marker, T>::value, executor_promise<T>>::type
            checkSync(
                    std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>> &futurePair,
                    boost::uuids::uuid uuid, int partitionId, const Member &member, bool preventSync) {
                bool sync = isSyncComputation(preventSync);
                if (sync) {
                    futurePair.first.get();
                }

                return executor_promise<T>(getContext());
            }

            bool isSyncComputation(bool preventSync);

            Address getMemberAddress(const Member &member);

            int randomPartitionId();

            static const int32_t MIN_TIME_RESOLUTION_OF_CONSECUTIVE_SUBMITS = 10;
            static const int32_t MAX_CONSECUTIVE_SUBMITS = 100;

            std::atomic<int32_t> consecutiveSubmits_;
            std::atomic<int64_t> lastSubmitTime_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


