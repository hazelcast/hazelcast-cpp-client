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
#ifndef HAZELCAST_CLIENT_IEXECUTORSERVICE_H_
#define HAZELCAST_CLIENT_IEXECUTORSERVICE_H_

#include <vector>
#include <atomic>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/cluster/memberselector/MemberSelectors.h"
#include "hazelcast/client/ICompletableFuture.h"
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
#include "hazelcast/client/internal/executor/CompletedFuture.h"
#include "hazelcast/client/proxy/IExecutorDelegatingFuture.h"

// CODECs
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace executor {
            namespace impl {
                class ExecutorServiceProxyFactory;
            }
        }

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
            friend class executor::impl::ExecutorServiceProxyFactory;

        public:
            static const std::string SERVICE_NAME;

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
                submit<HazelcastSerializable, bool>(command);
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
                submitToMember<HazelcastSerializable, bool>(command, member);
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
                    submitToMember<HazelcastSerializable, bool>(command, *it);
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
                    submitToMember<HazelcastSerializable, bool>(command, *it);
                }
            }

            /**
             * Submits a task to the owner of the specified key and returns a ICompletableFuture
             * representing that task.
             *
             * @param task task submitted to the owner of the specified key
             * @param key  the specified key
             * @param <T>  the result type of callable
             * @return a ICompletableFuture representing pending completion of the task
             */
            template<typename HazelcastSerializable, typename T, typename K>
            std::shared_ptr<ICompletableFuture<T> >
            submitToKeyOwner(const HazelcastSerializable &task, const K &key) {
                return submitToKeyOwnerInternal<HazelcastSerializable, T, K>(task, key, std::shared_ptr<T>(), false);
            }

            /**
             * Submits a task to the specified member and returns a ICompletableFuture
             * representing that task.
             *
             * @param task   the task submitted to the specified member
             * @param member the specified member
             * @param <T>    the result type of callable
             * @return a ICompletableFuture representing pending completion of the task
             */
            template<typename HazelcastSerializable, typename T>
            std::shared_ptr<ICompletableFuture<T> >
            submitToMember(const HazelcastSerializable &task, const Member &member) {
                const Address memberAddress = getMemberAddress(member);
                return submitToTargetInternal<HazelcastSerializable, T>(task, memberAddress, std::shared_ptr<T>(),
                                                                        false);
            }

            /**
             * Submits a task to given members and returns
             * map of Member-ICompletableFuture pairs representing pending completion of the task on each member
             *
             * @param task    the task submitted to given members
             * @param members the given members
             * @param <T>     the result type of callable
             * @return map of Member-ICompletableFuture pairs representing pending completion of the task on each member
             */
            template<typename HazelcastSerializable, typename T>
            std::map<Member, std::shared_ptr<ICompletableFuture<T> > >
            submitToMembers(const HazelcastSerializable &task, const std::vector<Member> &members) {
                std::map<Member, std::shared_ptr<ICompletableFuture<T> > > futureMap;
                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    Address memberAddress = getMemberAddress(*it);
                    std::shared_ptr<ICompletableFuture<T> > f = submitToTargetInternal<HazelcastSerializable, T>(task,
                                                                                                                   memberAddress,
                                                                                                                   std::shared_ptr<T>(),
                                                                                                                   true);
                    futureMap[*it] = f;
                }
                return futureMap;
            }

            /**
             * Submits a task to selected members and returns a
             * map of Member-ICompletableFuture pairs representing pending completion of the task on each member.
             *
             * @param task           the task submitted to selected members
             * @param memberSelector memberSelector
             * @param <T>            the result type of callable
             * @return map of Member-ICompletableFuture pairs representing pending completion of the task on each member
             * @throws RejectedExecutionException if no member is selected
             */
            template<typename HazelcastSerializable, typename T>
            std::map<Member, std::shared_ptr<ICompletableFuture<T> > >
            submitToMembers(const HazelcastSerializable &task,
                            const cluster::memberselector::MemberSelector &memberSelector) {
                std::vector<Member> members = selectMembers(memberSelector);
                return submitToMembers<HazelcastSerializable, T>(task, members);
            }

            /**
             * Submits task to all cluster members and returns a
             * map of Member-ICompletableFuture pairs representing pending completion of the task on each member.
             *
             * @param task the task submitted to all cluster members
             * @param <T>  the result type of callable
             * @return map of Member-ICompletableFuture pairs representing pending completion of the task on each member
             */
            template<typename HazelcastSerializable, typename T>
            std::map<Member, std::shared_ptr<ICompletableFuture<T> > >
            submitToAllMembers(const HazelcastSerializable &task) {
                std::vector<Member> members = getContext().getClientClusterService().getMemberList();
                std::map<Member, std::shared_ptr<ICompletableFuture<T> > > futureMap;
                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    Address memberAddress = getMemberAddress(*it);
                    std::shared_ptr<ICompletableFuture<T> > f = submitToTargetInternal<HazelcastSerializable, T>(task,
                                                                                                                   memberAddress,
                                                                                                                   std::shared_ptr<T>(),
                                                                                                                   true);
                    futureMap[*it] = f;
                }
                return futureMap;
            }

            /**
             * Submits a task for execution and returns a ICompletableFuture
             * representing that task. The ICompletableFuture's {@code ICompletableFuture::get} method will
             * return the given result upon successful completion.
             *
             * @param task the task to submit
             * @param result the result to return
             * @param <T> the type of the result
             * @return a ICompletableFuture representing pending completion of the task
             * @throws RejectedExecutionException if the task cannot be
             *         scheduled for execution
             * @throws NullPointerException if the task is null
             */
            template<typename HazelcastSerializable, typename T>
            std::shared_ptr<ICompletableFuture<T> >
            submit(const HazelcastSerializable &task, const std::shared_ptr<T> &result) {
                Data taskData = toData<HazelcastSerializable>(task);

                if (taskData.hasPartitionHash()) {
                    int partitionId = getPartitionId(taskData);

                    return submitToPartitionInternal<T>(taskData, result, false, partitionId);
                } else {
                    return submitToRandomInternal<T>(taskData, result, false);
                }
            }

            /**
             * Submits a value-returning task for execution and returns a
             * ICompletableFuture representing the pending results of the task. The
             * ICompletableFuture's {@code ICompletableFuture::get} method will return the task's result upon
             * successful completion.
             *
             * <p>
             * If you would like to immediately block waiting
             * for a task, you can use constructions of the form
             * {@code result = exec->submit<HazelcastSerializable, T>(aCallable)->get();}
             *
             *
             * @param task the task to submit
             * @param <T> the type of the task's result
             * @return a ICompletableFuture representing pending completion of the task
             * @throws RejectedExecutionException if the task cannot be
             *         scheduled for execution
             */
            template<typename HazelcastSerializable, typename T>
            std::shared_ptr<ICompletableFuture<T> > submit(const HazelcastSerializable &task) {
                return submit<HazelcastSerializable, T>(task, std::shared_ptr<T>());
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
             * Submits a task to a randomly selected member and returns a ICompletableFuture
             * representing that task.
             *
             * @param task           task submitted to a randomly selected member
             * @param memberSelector memberSelector
             * @param <T>            the result type of callable
             * @return a ICompletableFuture representing pending completion of the task
             * @throws RejectedExecutionException if no member is selected
             */
            template<typename HazelcastSerializable, typename T>
            std::shared_ptr<ICompletableFuture<T> >
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

                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    std::shared_ptr<ExecutionCallbackWrapper < T> >
                    executionCallback(new ExecutionCallbackWrapper<T>(multiExecutionCallbackWrapper, *it));
                    submitToMember<HazelcastSerializable, T>(task, *it, executionCallback);
                }
            }

            /**
             * Submits task to the selected members. Caller will be notified for the result of the each task by
             * {@link MultiExecutionCallback#onResponse(Member, Object)}, and when all tasks are completed,
             * {@link MultiExecutionCallback#onComplete(std::map)} will be called.
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
             * {@link MultiExecutionCallback#onComplete(std::map)} will be called.
             *
             * @param task     the task submitted to all the cluster members
             * @param callback callback
             */
            template<typename HazelcastSerializable, typename T>
            void submitToAllMembers(const HazelcastSerializable &task,
                                    const std::shared_ptr<MultiExecutionCallback<T> > &callback) {
                std::vector<Member> memberList = getContext().getClientClusterService().getMemberList();
                submitToMembers<HazelcastSerializable, T>(task, memberList, callback);
                std::shared_ptr<MultiExecutionCallbackWrapper < T> >
                multiExecutionCallbackWrapper(new MultiExecutionCallbackWrapper<T>((int) memberList.size(), callback));
                for (std::vector<Member>::const_iterator it = memberList.begin(); it != memberList.end(); ++it) {
                    std::shared_ptr<ExecutionCallbackWrapper < T> >
                    executionCallback(new ExecutionCallbackWrapper<T>(multiExecutionCallbackWrapper, *it));
                    submitToMember<HazelcastSerializable, T>(task, *it, executionCallback);
                }
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
            bool isShutdown();

            /**
             * Returns {@code true} if all tasks have completed following shut down.
             * Note that {@code isTerminated} is never {@code true} unless
             * either {@code shutdown}.
             *
             * @return {@code true} if all tasks have completed following shut down
             */
            bool isTerminated();

        private:
            IExecutorService(const std::string &name, spi::ClientContext *context);

            template<typename T>
            class MultiExecutionCallbackWrapper : MultiExecutionCallback<T> {
            public:
                MultiExecutionCallbackWrapper(
                        int memberSize, const std::shared_ptr<MultiExecutionCallback<T> > &multiExecutionCallback)
                        : multiExecutionCallback(multiExecutionCallback), members(memberSize) {
                }

            public:
                virtual void onResponse(const Member &member, const std::shared_ptr<T> &value) {
                    multiExecutionCallback->onResponse(member, value);
                    values.put(member, value);

                    int waitingResponse = --members;
                    if (waitingResponse == 0) {
                        complete();
                    }
                }

                virtual void
                onFailure(const Member &member, const std::shared_ptr<exception::IException> &exception) {
                    multiExecutionCallback->onFailure(member, exception);
                    exceptions.put(member, exception);

                    int waitingResponse = --members;
                    if (waitingResponse == 0) {
                        complete();
                    }
                }

                virtual void onComplete(const std::map<Member, std::shared_ptr<T> > &values,
                                        const std::map<Member, std::shared_ptr<exception::IException> > &exceptions) {
                    multiExecutionCallback->onComplete(values, exceptions);
                }

            private:
                void complete() {
                    std::map<Member, std::shared_ptr<T> > completedValues;
                    typedef std::vector<std::pair<Member, std::shared_ptr<T> > > ENTRYVECTOR;
                    ENTRYVECTOR entries = values.entrySet();
                    for (typename ENTRYVECTOR::const_iterator it = entries.begin();
                         it != entries.end(); ++it) {
                        completedValues[it->first] = it->second;
                    }

                    std::map<Member, std::shared_ptr<exception::IException> > completedExceptions;
                    typedef std::vector<std::pair<Member, std::shared_ptr<exception::IException> > > EXCEPTIONVECTOR;
                    EXCEPTIONVECTOR exceptionEntries = exceptions.entrySet();
                    for (typename EXCEPTIONVECTOR::const_iterator it = exceptionEntries.begin();
                         it != exceptionEntries.end(); ++it) {
                        completedExceptions[it->first] = it->second;
                    }

                    onComplete(completedValues, completedExceptions);
                }

                const std::shared_ptr<MultiExecutionCallback<T> > multiExecutionCallback;
                util::SynchronizedMap<Member, T> values;
                util::SynchronizedMap<Member, exception::IException> exceptions;
                util::AtomicInt members;
            };

            template<typename T>
            class ExecutionCallbackWrapper : public ExecutionCallback<T> {
            public:
                ExecutionCallbackWrapper(
                        const std::shared_ptr<MultiExecutionCallbackWrapper<T> > &multiExecutionCallbackWrapper,
                        const Member &member) : multiExecutionCallbackWrapper(multiExecutionCallbackWrapper),
                                                member(member) {}

                virtual void onResponse(const std::shared_ptr<T> &response) {
                    multiExecutionCallbackWrapper->onResponse(member, response);
                }

                virtual void onFailure(const std::shared_ptr<exception::IException> &e) {
                    multiExecutionCallbackWrapper->onFailure(member, e);
                }

            private:
                const std::shared_ptr<MultiExecutionCallbackWrapper<T> > multiExecutionCallbackWrapper;
                const Member member;
            };

            std::vector<Member> selectMembers(const cluster::memberselector::MemberSelector &memberSelector);

            template<typename T>
            std::shared_ptr<ICompletableFuture<T> >
            submitToPartitionInternal(const serialization::pimpl::Data &taskData,
                                      const std::shared_ptr<T> &defaultValue,
                                      bool preventSync, int partitionId) {
                std::string uuid = util::UuidUtil::newUnsecureUuidString();

                std::shared_ptr<spi::impl::ClientInvocationFuture> f = invokeOnPartitionInternal(taskData,
                                                                                                   partitionId, uuid);

                return checkSync(f, uuid, partitionId, preventSync, defaultValue);
            }

            template<typename T>
            void submitToPartitionInternal(const serialization::pimpl::Data &taskData, int partitionId,
                                           const std::shared_ptr<ExecutionCallback<T> > &callback) {
                std::string uuid = util::UuidUtil::newUnsecureUuidString();

                std::shared_ptr<spi::impl::ClientInvocationFuture> f = invokeOnPartitionInternal(taskData,
                                                                                                   partitionId, uuid);


                std::shared_ptr<ICompletableFuture<T> > delegatingFuture(
                        new internal::ClientDelegatingFuture<T>(f, getContext().getSerializationService(),
                                                                SUBMIT_TO_PARTITION_DECODER<T>(),
                                                                std::shared_ptr<T>()));

                delegatingFuture->andThen(callback);
            }

            std::shared_ptr<spi::impl::ClientInvocationFuture>
            invokeOnPartitionInternal(const Data &taskData, int partitionId, const std::string &uuid) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ExecutorServiceSubmitToPartitionCodec::encodeRequest(name, uuid, taskData,
                                                                                              partitionId);

                return invokeOnPartitionOwner(request, partitionId);
            }

            template<typename HazelcastSerializable, typename T, typename K>
            std::shared_ptr<ICompletableFuture<T> >
            submitToKeyOwnerInternal(const HazelcastSerializable &task, const K &key,
                                     const std::shared_ptr<T> &defaultValue, bool preventSync) {

                Data dataKey = toData<K>(key);

                int partitionId = getPartitionId(dataKey);

                return submitToPartitionInternal<T>(toData<HazelcastSerializable>(task), defaultValue, preventSync,
                                                    partitionId);
            }

            template<typename HazelcastSerializable, typename T, typename K>
            void submitToKeyOwnerInternal(const HazelcastSerializable &task, const K &key,
                                          const std::shared_ptr<ExecutionCallback<T> > &callback) {

                Data dataKey = toData<K>(key);

                int partitionId = getPartitionId(dataKey);

                submitToPartitionInternal<T>(toData<HazelcastSerializable>(task), partitionId, callback);
            }

            template<typename T>
            std::shared_ptr<ICompletableFuture<T> >
            submitToRandomInternal(const serialization::pimpl::Data &taskData, const std::shared_ptr<T> &defaultValue,
                                   bool preventSync) {

                int partitionId = randomPartitionId();

                return submitToPartitionInternal<T>(taskData, defaultValue, preventSync, partitionId);
            }

            template<typename T>
            void submitToRandomInternal(const serialization::pimpl::Data &taskData,
                                        const std::shared_ptr<ExecutionCallback<T> > &callback) {

                int partitionId = randomPartitionId();

                submitToPartitionInternal<T>(taskData, partitionId, callback);
            }

            template<typename HazelcastSerializable, typename T>
            std::shared_ptr<ICompletableFuture<T> >
            submitToTargetInternal(const HazelcastSerializable &task, const Address &address,
                                   const std::shared_ptr<T> &defaultValue, bool preventSync) {
                std::string uuid = util::UuidUtil::newUnsecureUuidString();

                std::shared_ptr<spi::impl::ClientInvocationFuture> f = invokeOnAddressInternal<HazelcastSerializable>(
                        task, address, uuid);

                return checkSync<T>(f, uuid, address, preventSync, defaultValue);
            }

            template<typename HazelcastSerializable, typename T>
            void submitToTargetInternal(const HazelcastSerializable &task, const Address &address,
                                        const std::shared_ptr<ExecutionCallback<T> > &callback) {
                std::string uuid = util::UuidUtil::newUnsecureUuidString();

                std::shared_ptr<spi::impl::ClientInvocationFuture> f = invokeOnAddressInternal<HazelcastSerializable>(
                        task, address, uuid);

                std::shared_ptr<ICompletableFuture<T> > delegatingFuture(
                        new internal::ClientDelegatingFuture<T>(f, getContext().getSerializationService(),
                                                                SUBMIT_TO_ADDRESS_DECODER<T>(),
                                                                std::shared_ptr<T>()));

                delegatingFuture->andThen(callback);
            }

            template<typename HazelcastSerializable>
            std::shared_ptr<spi::impl::ClientInvocationFuture>
            invokeOnAddressInternal(const HazelcastSerializable &task, const Address &address,
                                    const std::string &uuid) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::ExecutorServiceSubmitToAddressCodec::encodeRequest(name, uuid,
                                                                                            toData(
                                                                                                    task), address);

                std::shared_ptr<spi::impl::ClientInvocationFuture> f = invokeOnTarget(request, address);
                return f;
            }

            std::shared_ptr<spi::impl::ClientInvocationFuture>
            invokeOnPartitionOwner(std::unique_ptr<protocol::ClientMessage> &request, int partitionId);

            std::shared_ptr<spi::impl::ClientInvocationFuture>
            invokeOnTarget(std::unique_ptr<protocol::ClientMessage> &request, const Address &target);

            template<typename T>
            std::shared_ptr<T>
            retrieveResultFromMessage(const std::shared_ptr<spi::impl::ClientInvocationFuture> &f) {
                serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
                std::unique_ptr<serialization::pimpl::Data> data = protocol::codec::ExecutorServiceSubmitToAddressCodec::ResponseParameters::decode(
                        *f->get()).response;
                return std::shared_ptr<T>(serializationService.toObject<T>(data.get()));
            }

            template<typename T>
            std::shared_ptr<ICompletableFuture<T> >
            checkSync(const std::shared_ptr<spi::impl::ClientInvocationFuture> &f, const std::string &uuid,
                      int partitionId, bool preventSync, std::shared_ptr<T> defaultValue) {
                bool sync = isSyncComputation(preventSync);
                if (sync) {
                    return retrieveResultSync<T>(f);
                } else {
                    return std::shared_ptr<ICompletableFuture<T> >(
                            new proxy::IExecutorDelegatingFuture<T>(f, getContext(), uuid, defaultValue,
                                                                    SUBMIT_TO_PARTITION_DECODER<T>(), name,
                                                                    partitionId));
                }

            }

            template<typename T>
            std::shared_ptr<ICompletableFuture<T> >
            checkSync(const std::shared_ptr<spi::impl::ClientInvocationFuture> &f, const std::string &uuid,
                      const Address &address, bool preventSync, std::shared_ptr<T> defaultValue) {
                bool sync = isSyncComputation(preventSync);
                if (sync) {
                    return retrieveResultSync<T>(f);
                } else {
                    return std::shared_ptr<ICompletableFuture<T> >(
                            new proxy::IExecutorDelegatingFuture<T>(f, getContext(), uuid, defaultValue,
                                                                    SUBMIT_TO_ADDRESS_DECODER<T>(), name, address));
                }

            }

            template<typename T>
            std::shared_ptr<ICompletableFuture<T> >
            retrieveResultSync(const std::shared_ptr<spi::impl::ClientInvocationFuture> &f) {
                try {
                    std::shared_ptr<T> response = retrieveResultFromMessage<T>(f);
                    std::shared_ptr<ExecutorService> userExecutor = getContext().getClientExecutionService().getUserExecutor();
                    return std::shared_ptr<ICompletableFuture<T> >(
                            new internal::executor::CompletedFuture<T>(response, userExecutor));
                } catch (exception::IException &e) {
                    std::shared_ptr<ExecutorService> userExecutor = getContext().getClientExecutionService().getUserExecutor();
                    return std::shared_ptr<ICompletableFuture<T> >(
                            new internal::executor::CompletedFuture<T>(
                                    std::shared_ptr<exception::IException>(e.clone()), userExecutor));
                }
            }

            bool isSyncComputation(bool preventSync);

            Address getMemberAddress(const Member &member);

            int randomPartitionId();

            template<typename T>
            static const std::shared_ptr<impl::ClientMessageDecoder<T> > SUBMIT_TO_PARTITION_DECODER() {
                return impl::DataMessageDecoder<protocol::codec::ExecutorServiceSubmitToPartitionCodec, T>::instance();
            }

            template<typename T>
            static const std::shared_ptr<impl::ClientMessageDecoder<T> > SUBMIT_TO_ADDRESS_DECODER() {
                return impl::DataMessageDecoder<protocol::codec::ExecutorServiceSubmitToAddressCodec, T>::instance();
            }

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

#endif /* HAZELCAST_CLIENT_IEXECUTORSERVICE_H_ */
