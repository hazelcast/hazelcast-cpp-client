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

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/cluster/memberselector/MemberSelectors.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/MultiExecutionCallback.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/util/UuidUtil.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/util/ExceptionUtil.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"

// CODECs
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"

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
            static const std::string SERVICE_NAME;

            template<typename T>
            class executor_promise {
            public:
                executor_promise(spi::ClientContext &context) : context(context) {}

                executor_promise(boost::future<boost::optional<T>> &future, const std::string &uuid, int partitionId,
                                 const Address &address, spi::ClientContext &context,
                                 const std::shared_ptr<spi::impl::ClientInvocation> &invocation)
                        : sharedFuture(future.share()), uuid(uuid), partitionId(partitionId), address(address),
                          context(context), invocation(invocation) {}

                bool cancel(bool mayInterruptIfRunning) {
                    if (sharedFuture.is_ready()) {
                        return false;
                    }

                    try {
                        return invokeCancelRequest(mayInterruptIfRunning);
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(std::current_exception());
                    }
                    return false;
                }

                boost::shared_future<boost::optional<T>> get_future() {
                    return sharedFuture;
                }

            private:
                boost::shared_future<boost::optional<T>> sharedFuture;
                std::string uuid;
                int partitionId;
                Address address;
                spi::ClientContext &context;
                std::shared_ptr<spi::impl::ClientInvocation> invocation;

                bool invokeCancelRequest(bool mayInterruptIfRunning) {
                    invocation->getSendConnectionOrWait();

                    if (partitionId > -1) {
                        auto request = protocol::codec::ExecutorServiceCancelOnPartitionCodec::encodeRequest(uuid,
                                                                                                             partitionId,
                                                                                                             mayInterruptIfRunning);
                        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                                context, request, uuid, partitionId);
                        return protocol::codec::ExecutorServiceCancelOnPartitionCodec::ResponseParameters::decode(
                                clientInvocation->invoke().get()).response;
                    } else {
                        auto request = protocol::codec::ExecutorServiceCancelOnAddressCodec::encodeRequest(
                                uuid, address, mayInterruptIfRunning);
                        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                                context, request, uuid, address);
                        return protocol::codec::ExecutorServiceCancelOnAddressCodec::ResponseParameters::decode(
                                clientInvocation->invoke().get()).response;
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
                const Address memberAddress = getMemberAddress(member);
                return submitToTargetInternal<HazelcastSerializable, T>(task, memberAddress, false);
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
                    Address memberAddress = getMemberAddress(member);
                    auto f = submitToTargetInternal<HazelcastSerializable, T>(task, memberAddress, true);
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
                std::vector<Member> members = getContext().getClientClusterService().getMemberList();
                std::unordered_map<Member, executor_promise<T>> futureMap;
                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    Address memberAddress = getMemberAddress(*it);
                    auto f = submitToTargetInternal<HazelcastSerializable, T>(task, memberAddress, true);
                    // no need to check if emplace is success since member is unique
                    futureMap.emplace(*it, std::move(f));
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
                const Address memberAddress = getMemberAddress(member);
                return submitToTargetInternal<HazelcastSerializable, T>(task, memberAddress, callback);
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
                        : multiExecutionCallback(multiExecutionCallback), members(memberSize) {
                }

            public:
                virtual void onResponse(const Member &member, const boost::optional<T> &value) {
                    multiExecutionCallback->onResponse(member, value);

                    std::lock_guard<std::mutex> guard(lock);
                    values[member] = value;
                    int waitingResponse = --members;
                    if (waitingResponse == 0) {
                        onComplete(values, exceptions);
                    }
                }

                virtual void
                onFailure(const Member &member, std::exception_ptr exception) {
                    multiExecutionCallback->onFailure(member, exception);

                    std::lock_guard<std::mutex> guard(lock);
                    exceptions[member] = exception;
                    int waitingResponse = --members;
                    if (waitingResponse == 0) {
                        onComplete(values, exceptions);
                    }
                }

                virtual void onComplete(const std::unordered_map<Member, boost::optional<T> > &vals,
                                        const std::unordered_map<Member, std::exception_ptr> &excs) {
                    multiExecutionCallback->onComplete(vals, excs);
                }

            private:

                const std::shared_ptr<MultiExecutionCallback<T> > multiExecutionCallback;
                // TODO: We may not need thread safe structures here if being used from the same thread
                std::unordered_map<Member, boost::optional<T>> values;
                std::unordered_map<Member, std::exception_ptr> exceptions;
                int members;
                std::mutex lock;
            };

            template<typename T>
            class ExecutionCallbackWrapper : public ExecutionCallback<T> {
            public:
                ExecutionCallbackWrapper(
                        const std::shared_ptr<MultiExecutionCallbackWrapper<T> > &multiExecutionCallbackWrapper,
                        Member member) : multiExecutionCallbackWrapper(multiExecutionCallbackWrapper),
                                                member(std::move(member)) {}

                virtual void onResponse(const boost::optional<T> &response) {
                    multiExecutionCallbackWrapper->onResponse(member, response);
                }

                virtual void onFailure(std::exception_ptr e) {
                    multiExecutionCallbackWrapper->onFailure(member, e);
                }

            private:
                const std::shared_ptr<MultiExecutionCallbackWrapper<T> > multiExecutionCallbackWrapper;
                const Member member;
            };

            std::vector<Member> selectMembers(const cluster::memberselector::MemberSelector &memberSelector);

            template<typename T>
            executor_promise<T>
            submitToPartitionInternal(const serialization::pimpl::Data &taskData, bool preventSync, int partitionId) {
                std::string uuid = util::UuidUtil::newUnsecureUuidString();

                auto f = invokeOnPartitionInternal(taskData, partitionId, uuid);

                return checkSync<T, protocol::codec::ExecutorServiceSubmitToPartitionCodec>(f, uuid, partitionId,
                                                                                            preventSync);
            }

            template<typename T>
            void submitToPartitionInternal(const serialization::pimpl::Data &taskData, int partitionId,
                                           const std::shared_ptr<ExecutionCallback<T> > &callback) {
                std::string uuid = util::UuidUtil::newUnsecureUuidString();

                auto messageFuture = invokeOnPartitionInternal(taskData, partitionId, uuid);

                serialization::pimpl::SerializationService *serializationService = &getSerializationService();
                spi::impl::ClientExecutionServiceImpl *executionService = &getContext().getClientExecutionService();
                messageFuture.first.then(boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
                    try {
                        auto result = retrieveResultFromMessage<T, protocol::codec::ExecutorServiceSubmitToPartitionCodec>(
                                serializationService, std::move(f));
                        executionService->execute([=]() { callback->onResponse(result); });
                    } catch (exception::IException &e) {
                        auto exception = std::current_exception();
                        executionService->execute([=]() { callback->onFailure(exception); });
                    }
                });
            }

            std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
            invokeOnPartitionInternal(const serialization::pimpl::Data &taskData, int partitionId,
                                      const std::string &uuid) {
                auto request = protocol::codec::ExecutorServiceSubmitToPartitionCodec::encodeRequest(name, uuid, taskData,
                                                                                              partitionId);

                return invokeOnPartitionOwner(request, partitionId);
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
            executor_promise<T> submitToTargetInternal(const HazelcastSerializable &task, const Address &address,
                                                       bool preventSync) {
                std::string uuid = util::UuidUtil::newUnsecureUuidString();

                auto f = invokeOnAddressInternal<HazelcastSerializable>(task, address, uuid);

                return checkSync<T, protocol::codec::ExecutorServiceSubmitToAddressCodec>(f, uuid, -1, address,
                                                                                          preventSync);
            }

            template<typename HazelcastSerializable, typename T>
            void submitToTargetInternal(const HazelcastSerializable &task, const Address &address,
                                        const std::shared_ptr<ExecutionCallback<T> > &callback) {
                std::string uuid = util::UuidUtil::newUnsecureUuidString();

                auto messageFuture = invokeOnAddressInternal<HazelcastSerializable>(task, address, uuid);

                serialization::pimpl::SerializationService *serializationService = &(getSerializationService());
                spi::impl::ClientExecutionServiceImpl *executionService = &getContext().getClientExecutionService();
                messageFuture.first.then(boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
                    try {
                        auto result = retrieveResultFromMessage<T, protocol::codec::ExecutorServiceSubmitToAddressCodec>(
                                serializationService, std::move(f));
                        executionService->execute([=]() { callback->onResponse(result); });
                    } catch (exception::IException &) {
                        auto exception = std::current_exception();
                        executionService->execute([=]() { callback->onFailure(exception); });
                    }
                });
            }

            template<typename HazelcastSerializable>
            std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
            invokeOnAddressInternal(const HazelcastSerializable &task, const Address &address,
                                    const std::string &uuid) {
                auto request = protocol::codec::ExecutorServiceSubmitToAddressCodec::encodeRequest(name, uuid,
                                                                                                   toData(task),
                                                                                                   address);

                return invokeOnTarget(request, address);
            }

            std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
            invokeOnPartitionOwner(std::unique_ptr<protocol::ClientMessage> &request, int partitionId);

            std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>>
            invokeOnTarget(std::unique_ptr<protocol::ClientMessage> &request, const Address &target);

            template<typename T, typename DECODER>
            boost::optional<T>
            retrieveResultFromMessage(serialization::pimpl::SerializationService *serializationService,
                    boost::future<protocol::ClientMessage> f) {
                return serializationService->toObject<T>(DECODER::ResponseParameters::decode(f.get()).response.get());
            }

            template<typename T, typename DECODER>
            executor_promise<T>
            checkSync(
                    std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>> &futurePair,
                    const std::string &uuid, int partitionId, bool preventSync) {
                return checkSync<T, DECODER>(futurePair, uuid, partitionId, Address(), preventSync);
            }

            template<typename T, typename DECODER>
            boost::future<boost::optional<T>>
            retrieveResultSync(boost::future<protocol::ClientMessage> future) {
                try {
                    auto response = retrieveResultFromMessage<T, DECODER>(&(getSerializationService()), std::move(future));
                    return boost::make_ready_future(response);
                } catch (exception::IException &) {
                    return boost::make_exceptional_future<boost::optional<T>>(boost::current_exception());
                }
            }

            template<typename T, typename DECODER>
            typename std::enable_if<!std::is_same<executor_marker, T>::value, executor_promise<T>>::type
            checkSync(
                    std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>> &futurePair,
                    const std::string &uuid, int partitionId, const Address &address, bool preventSync) {
                bool sync = isSyncComputation(preventSync);
                boost::future<boost::optional<T>> objectFuture;
                if (sync) {
                    objectFuture = retrieveResultSync<T, DECODER>(std::move(futurePair.first));
                } else {
                    serialization::pimpl::SerializationService *serializationService = &getSerializationService();
                    objectFuture = futurePair.first.then(boost::launch::sync,
                                                         [=](boost::future<protocol::ClientMessage> f) {
                                                             return retrieveResultFromMessage<T, DECODER>(
                                                                     serializationService, std::move(f));
                                                         });
                }

                return executor_promise<T>(objectFuture, uuid, partitionId, address, getContext(), futurePair.second);
            }

            template<typename T, typename DECODER>
            typename std::enable_if<std::is_same<executor_marker, T>::value, executor_promise<T>>::type
            checkSync(
                    std::pair<boost::future<protocol::ClientMessage>, std::shared_ptr<spi::impl::ClientInvocation>> &futurePair,
                    const std::string &uuid, int partitionId, const Address &address, bool preventSync) {
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

            std::atomic<int32_t> consecutiveSubmits;
            std::atomic<int64_t> lastSubmitTime;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


