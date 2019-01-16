/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/internal/util/executor/CompletedFuture.h"
#include "hazelcast/client/proxy/IExecutorDelegatingFuture.h"

// CODECs
#include "hazelcast/client/protocol/codec/ExecutorServiceSubmitToPartitionCodec.h"
#include "hazelcast/client/protocol/codec/ExecutorServiceSubmitToAddressCodec.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {

        /**
         * Distributed implementation of {@link java.util.concurrent.ExecutorService}.
         * IExecutorService provides additional methods like executing tasks
         * on a specific member, on a member who is owner of a specific key,
         * executing a tasks on multiple members and listening execution result using a callback.
         *
         *
         * @see ExecutionCallback
         * @see MultiExecutionCallback
         */
        class HAZELCAST_API IExecutorService : public proxy::ProxyImpl {
        public:
            static const std::string SERVICE_NAME;

            IExecutorService(const std::string &name, spi::ClientContext *context);

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
                submit<HazelcastSerializable>(command);
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
                    submitToMember<HazelcastSerializable>(command, *it);
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
                    submitToMember<HazelcastSerializable>(command, *it);
                }
            }

            /**
             * Submits a task to the owner of the specified key and returns a Future
             * representing that task.
             *
             * @param task task submitted to the owner of the specified key
             * @param key  the specified key
             * @param <T>  the result type of callable
             * @return a Future representing pending completion of the task
             */
            template<typename HazelcastSerializable, typename T, typename K>
            boost::shared_ptr<ICompletableFuture<T> >
            submitToKeyOwner(const HazelcastSerializable &task, const K &key) {
                return submitToKeyOwnerInternal<HazelcastSerializable, T, K>(task, key, boost::shared_ptr<T>(), false);
            }

            /**
             * Submits a task to the specified member and returns a Future
             * representing that task.
             *
             * @param task   the task submitted to the specified member
             * @param member the specified member
             * @param <T>    the result type of callable
             * @return a Future representing pending completion of the task
             */
            template<typename HazelcastSerializable, typename T>
            boost::shared_ptr<ICompletableFuture<T> >
            submitToMember(const HazelcastSerializable &task, const Member &member) {
                const Address memberAddress = getMemberAddress(member);
                return submitToTargetInternal<HazelcastSerializable, T>(task, memberAddress, boost::shared_ptr<T>(),
                                                                        false);
            }

            /**
             * Submits a task to given members and returns
             * map of Member-Future pairs representing pending completion of the task on each member
             *
             * @param task    the task submitted to given members
             * @param members the given members
             * @param <T>     the result type of callable
             * @return map of Member-Future pairs representing pending completion of the task on each member
             */
            template<typename HazelcastSerializable, typename T>
            std::map<Member, boost::shared_ptr<ICompletableFuture<T> > >
            submitToMembers(const HazelcastSerializable &task, const std::vector<Member> &members) {
                std::map<Member, boost::shared_ptr<ICompletableFuture<T> > > futureMap;
                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    Address memberAddress = getMemberAddress(*it);
                    boost::shared_ptr<ICompletableFuture<T> > f = submitToTargetInternal<HazelcastSerializable, T>(task,
                                                                                                                   memberAddress,
                                                                                                                   boost::shared_ptr<T>(),
                                                                                                                   true);
                    futureMap[*it] = f;
                }
                return futureMap;
            }

            /**
             * Submits a task to selected members and returns a
             * map of Member-Future pairs representing pending completion of the task on each member.
             *
             * @param task           the task submitted to selected members
             * @param memberSelector memberSelector
             * @param <T>            the result type of callable
             * @return map of Member-Future pairs representing pending completion of the task on each member
             * @throws RejectedExecutionException if no member is selected
             */
            template<typename HazelcastSerializable, typename T>
            std::map<Member, boost::shared_ptr<ICompletableFuture<T> > >
            submitToMembers(const HazelcastSerializable &task,
                            const cluster::memberselector::MemberSelector &memberSelector) {
                std::vector<Member> members = selectMembers(memberSelector);
                return submitToMembers<HazelcastSerializable, T>(task, members);
            }

            /**
             * Submits task to all cluster members and returns a
             * map of Member-Future pairs representing pending completion of the task on each member.
             *
             * @param task the task submitted to all cluster members
             * @param <T>  the result type of callable
             * @return map of Member-Future pairs representing pending completion of the task on each member
             */
            template<typename HazelcastSerializable, typename T>
            std::map<Member, boost::shared_ptr<ICompletableFuture<T> > >
            submitToAllMembers(const HazelcastSerializable &task) {
                std::vector<Member> members = getContext().getClientClusterService().getMemberList();
                std::map<Member, boost::shared_ptr<ICompletableFuture<T> > > futureMap;
                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    Address memberAddress = getMemberAddress(*it);
                    boost::shared_ptr<ICompletableFuture<T> > f = submitToTargetInternal<HazelcastSerializable, T>(task,
                                                                                                                   memberAddress,
                                                                                                                   boost::shared_ptr<T>(),
                                                                                                                   true);
                    futureMap[*it] = f;
                }
                return futureMap;
            }

            /**
             * Submits a Runnable task for execution and returns a Future
             * representing that task. The Future's {@code get} method will
             * return the given result upon successful completion.
             *
             * @param task the task to submit
             * @param result the result to return
             * @param <T> the type of the result
             * @return a Future representing pending completion of the task
             * @throws RejectedExecutionException if the task cannot be
             *         scheduled for execution
             * @throws NullPointerException if the task is null
             */
            template<typename HazelcastSerializable, typename T>
            boost::shared_ptr<ICompletableFuture<T> >
            submit(const HazelcastSerializable &task, const boost::shared_ptr<T> &result) {
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
             * Future representing the pending results of the task. The
             * Future's {@code get} method will return the task's result upon
             * successful completion.
             *
             * <p>
             * If you would like to immediately block waiting
             * for a task, you can use constructions of the form
             * {@code result = exec.submit(aCallable).get();}
             *
             *
             * @param task the task to submit
             * @param <T> the type of the task's result
             * @return a Future representing pending completion of the task
             * @throws RejectedExecutionException if the task cannot be
             *         scheduled for execution
             */
            template<typename HazelcastSerializable, typename T>
            boost::shared_ptr<ICompletableFuture<T> > submit(const HazelcastSerializable &task) {
                return submit<HazelcastSerializable, T>(task, boost::shared_ptr<T>());
            }

            /**
             * Submits a task to a random member. Caller will be notified of the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
             *
             * @param task     a task submitted to a random member
             * @param callback callback
             * @param <T>      the response type of callback
             */
            template<typename HazelcastSerializable, typename T>
            void submit(const HazelcastSerializable &task, const boost::shared_ptr<ExecutionCallback<T> > &callback) {
                Data taskData = toData<HazelcastSerializable>(task);

                if (taskData.hasPartitionHash()) {
                    int partitionId = getPartitionId(taskData);

                    submitToPartitionInternal<T>(taskData, partitionId, callback);
                } else {
                    submitToRandomInternal<T>(taskData, callback);
                }
            }

            /**
             * Submits a task to a randomly selected member and returns a Future
             * representing that task.
             *
             * @param task           task submitted to a randomly selected member
             * @param memberSelector memberSelector
             * @param <T>            the result type of callable
             * @return a Future representing pending completion of the task
             * @throws RejectedExecutionException if no member is selected
             */
            template<typename HazelcastSerializable, typename T>
            boost::shared_ptr<ICompletableFuture<T> >
            submit(const HazelcastSerializable &task, const cluster::memberselector::MemberSelector &memberSelector) {
                std::vector<Member> members = selectMembers(memberSelector);
                int selectedMember = rand() % (int) members.size();
                return submitToMember<HazelcastSerializable, T>(task, members[selectedMember]);
            }

            /**
             * Submits a task to randomly selected members. Caller will be notified for the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
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
                   const boost::shared_ptr<ExecutionCallback<T> > &callback) {
                std::vector<Member> members = selectMembers(memberSelector);
                int selectedMember = rand() % (int) members.size();
                return submitToMember(task, members[selectedMember], callback);
            }

            /**
             * Submits a task to the owner of the specified key. Caller will be notified for the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
             *
             * @param task     task submitted to the owner of the specified key
             * @param key      the specified key
             * @param callback callback
             * @param <T>      the response type of callback
             */
            template<typename HazelcastSerializable, typename T, typename K>
            void submitToKeyOwner(const HazelcastSerializable &task, const K &key,
                                  const boost::shared_ptr<ExecutionCallback<T> > &callback) {
                submitToKeyOwnerInternal<HazelcastSerializable, T, K>(task, key, callback);
            }

            /**
             * Submits a task to the specified member. Caller will be notified for the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
             *
             * @param task     the task submitted to the specified member
             * @param member   the specified member
             * @param callback callback
             * @param <T>      the response type of callback
             */
            template<typename HazelcastSerializable, typename T>
            void submitToMember(const HazelcastSerializable &task, const Member &member,
                                const boost::shared_ptr<ExecutionCallback<T> > &callback) {
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
                                 const boost::shared_ptr<MultiExecutionCallback<T> > &callback) {
                boost::shared_ptr<MultiExecutionCallbackWrapper < T> >
                multiExecutionCallbackWrapper(new MultiExecutionCallbackWrapper<T>(members.size(), callback));

                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    boost::shared_ptr<ExecutionCallbackWrapper<T> >
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
                                 const boost::shared_ptr<MultiExecutionCallback<T> > &callback) {
                std::vector<Member> members = selectMembers(memberSelector);
                submitToMembers<HazelcastSerializable, T>(task, members, callback);
            }

            /**
             * Submits task to all the cluster members. Caller will be notified for the result of each task by
             * {@link MultiExecutionCallback#onResponse(const Member &, const boost::shared_ptr<V> &)}, and when all tasks are completed,
             * {@link MultiExecutionCallback#onComplete(std::map)} will be called.
             *
             * @param task     the task submitted to all the cluster members
             * @param callback callback
             */
            template<typename HazelcastSerializable, typename T>
            void submitToAllMembers(const HazelcastSerializable &task,
                                    const boost::shared_ptr<MultiExecutionCallback<T> > &callback) {
                std::vector<Member> memberList = getContext().getClientClusterService().getMemberList();
                submitToMembers<HazelcastSerializable, T>(task, memberList, callback);
                boost::shared_ptr<MultiExecutionCallbackWrapper<T> > multiExecutionCallbackWrapper(new MultiExecutionCallbackWrapper<T>((int) memberList.size(), callback));
                for (std::vector<Member>::const_iterator it = memberList.begin(); it != memberList.end(); ++it) {
                    boost::shared_ptr<ExecutionCallbackWrapper <T> >
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
             * complete execution.  Use {@link #awaitTermination awaitTermination}
             * to do that.
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
             * either {@code shutdown} or {@code shutdownNow} was called first.
             *
             * @return {@code true} if all tasks have completed following shut down
             */
            bool isTerminated();

        private:
            template<typename T>
            class MultiExecutionCallbackWrapper : MultiExecutionCallback<T> {
            public:
                MultiExecutionCallbackWrapper(
                        int memberSize, const boost::shared_ptr<MultiExecutionCallback<T> > &multiExecutionCallback)
                        : multiExecutionCallback(multiExecutionCallback), members(memberSize) {
                }

            public:
                virtual void onResponse(const Member &member, const boost::shared_ptr<T> &value) {
                    multiExecutionCallback->onResponse(member, value);
                    values.put(member, value);

                    int waitingResponse = --members;
                    if (waitingResponse == 0) {
                        complete();
                    }
                }

                virtual void
                onFailure(const Member &member, const boost::shared_ptr<exception::IException> &exception) {
                    multiExecutionCallback->onFailure(member, exception);
                    exceptions.put(member, exception);

                    int waitingResponse = --members;
                    if (waitingResponse == 0) {
                        complete();
                    }
                }

                virtual void onComplete(const std::map<Member, boost::shared_ptr<T> > &values,
                                        const std::map<Member, boost::shared_ptr<exception::IException> > &exceptions) {
                    multiExecutionCallback->onComplete(values, exceptions);
                }

            private:
                void complete() {
                    std::map<Member, boost::shared_ptr<T> > completedValues;
                    typedef std::vector<std::pair<Member, boost::shared_ptr<T> > > ENTRYVECTOR;
                    ENTRYVECTOR entries = values.entrySet();
                    for (typename ENTRYVECTOR::const_iterator it = entries.begin();
                             it != entries.end(); ++it) {
                            completedValues[it->first] = it->second;
                        }

                    std::map<Member, boost::shared_ptr<exception::IException> > completedExceptions;
                    typedef std::vector<std::pair<Member, boost::shared_ptr<exception::IException> > > EXCEPTIONVECTOR;
                    EXCEPTIONVECTOR exceptionEntries = exceptions.entrySet();
                    for (typename EXCEPTIONVECTOR::const_iterator it = exceptionEntries.begin();
                             it != exceptionEntries.end(); ++it) {
                            completedExceptions[it->first] = it->second;
                        }

                    onComplete(completedValues, completedExceptions);
                }

                const boost::shared_ptr<MultiExecutionCallback<T> > multiExecutionCallback;
                util::SynchronizedMap<Member, T> values;
                util::SynchronizedMap<Member, exception::IException> exceptions;
                util::AtomicInt members;
            };

            template<typename T>
            class ExecutionCallbackWrapper : public ExecutionCallback<T> {
            public:
                ExecutionCallbackWrapper(
                        const boost::shared_ptr<MultiExecutionCallbackWrapper<T> > &multiExecutionCallbackWrapper,
                        const Member &member) : multiExecutionCallbackWrapper(multiExecutionCallbackWrapper),
                                                member(member) {}

                virtual void onResponse(const boost::shared_ptr<T> &response) {
                    multiExecutionCallbackWrapper->onResponse(member, response);
                }

                virtual void onFailure(const boost::shared_ptr<exception::IException> &e) {
                    multiExecutionCallbackWrapper->onFailure(member, e);
                }

            private:
                const boost::shared_ptr<MultiExecutionCallbackWrapper<T> > multiExecutionCallbackWrapper;
                const Member member;
            };


            class SubmitToPartitionDecoder : public impl::ClientMessageDecoder {
            public:
                virtual boost::shared_ptr<serialization::pimpl::Data>
                decodeClientMessage(const boost::shared_ptr<protocol::ClientMessage> &clientMessage);
            };

            class SubmitToAddressDecoder : public impl::ClientMessageDecoder {
            public:
                virtual boost::shared_ptr<serialization::pimpl::Data>
                decodeClientMessage(const boost::shared_ptr<protocol::ClientMessage> &clientMessage);
            };

            std::vector<Member> selectMembers(const cluster::memberselector::MemberSelector &memberSelector);

            template<typename T>
            boost::shared_ptr<ICompletableFuture<T> >
            submitToPartitionInternal(const serialization::pimpl::Data &taskData,
                                      const boost::shared_ptr<T> &defaultValue,
                                      bool preventSync, int partitionId) {
                std::string uuid = util::UuidUtil::newUnsecureUuidString();

                boost::shared_ptr<spi::impl::ClientInvocationFuture> f = invokeOnPartitionInternal(taskData,
                                                                                                   partitionId, uuid);

                return checkSync(f, uuid, partitionId, preventSync, defaultValue);
            }

            template<typename T>
            void submitToPartitionInternal(const serialization::pimpl::Data &taskData, int partitionId,
                                           const boost::shared_ptr<ExecutionCallback<T> > &callback) {
                std::string uuid = util::UuidUtil::newUnsecureUuidString();

                boost::shared_ptr<spi::impl::ClientInvocationFuture> f = invokeOnPartitionInternal(taskData,
                                                                                                   partitionId, uuid);


                boost::shared_ptr<ICompletableFuture<T> > delegatingFuture(
                        new internal::util::ClientDelegatingFuture<T>(f, getContext().getSerializationService(),
                                                                      SUBMIT_TO_PARTITION_DECODER(),
                                                                      boost::shared_ptr<T>()));

                delegatingFuture->andThen(callback);
            }

            boost::shared_ptr<spi::impl::ClientInvocationFuture>
            invokeOnPartitionInternal(const Data &taskData, int partitionId, const std::string &uuid) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ExecutorServiceSubmitToPartitionCodec::encodeRequest(name, uuid, taskData,
                                                                                              partitionId);

                return invokeOnPartitionOwner(request, partitionId);
            }

            template<typename HazelcastSerializable, typename T, typename K>
            boost::shared_ptr<ICompletableFuture<T> >
            submitToKeyOwnerInternal(const HazelcastSerializable &task, const K &key,
                                     const boost::shared_ptr<T> &defaultValue, bool preventSync) {

                Data dataKey = toData<K>(key);

                int partitionId = getPartitionId(dataKey);

                return submitToPartitionInternal<T>(toData<HazelcastSerializable>(task), defaultValue, preventSync,
                                                    partitionId);
            }

            template<typename HazelcastSerializable, typename T, typename K>
            void submitToKeyOwnerInternal(const HazelcastSerializable &task, const K &key,
                    const boost::shared_ptr<ExecutionCallback<T> > &callback) {

                Data dataKey = toData<K>(key);

                int partitionId = getPartitionId(dataKey);

                submitToPartitionInternal<T>(toData<HazelcastSerializable>(task), partitionId, callback);
            }

            template<typename T>
            boost::shared_ptr<ICompletableFuture<T> >
            submitToRandomInternal(const serialization::pimpl::Data &taskData, const boost::shared_ptr<T> &defaultValue,
                                   bool preventSync) {

                int partitionId = randomPartitionId();

                return submitToPartitionInternal<T>(taskData, defaultValue, preventSync, partitionId);
            }

            template<typename T>
            void submitToRandomInternal(const serialization::pimpl::Data &taskData,
                                        const boost::shared_ptr<ExecutionCallback<T> > &callback) {

                int partitionId = randomPartitionId();

                submitToPartitionInternal<T>(taskData, partitionId, callback);
            }

            template<typename HazelcastSerializable, typename T>
            boost::shared_ptr<ICompletableFuture<T> >
            submitToTargetInternal(const HazelcastSerializable &task, const Address &address,
                                   const boost::shared_ptr<T> &defaultValue, bool preventSync) {
                std::string uuid = util::UuidUtil::newUnsecureUuidString();

                boost::shared_ptr<spi::impl::ClientInvocationFuture> f = invokeOnAddressInternal<HazelcastSerializable>(
                        task, address, uuid);

                return checkSync<T>(f, uuid, address, preventSync, defaultValue);
            }

            template<typename HazelcastSerializable, typename T>
            void submitToTargetInternal(const HazelcastSerializable &task, const Address &address,
                                        const boost::shared_ptr<ExecutionCallback<T> > &callback) {
                std::string uuid = util::UuidUtil::newUnsecureUuidString();

                boost::shared_ptr<spi::impl::ClientInvocationFuture> f = invokeOnAddressInternal<HazelcastSerializable>(
                        task, address, uuid);

                boost::shared_ptr<ICompletableFuture<T> > delegatingFuture(
                        new internal::util::ClientDelegatingFuture<T>(f, getContext().getSerializationService(),
                                                                      SUBMIT_TO_ADDRESS_DECODER(),
                                                                      boost::shared_ptr<T>()));

                delegatingFuture->andThen(callback);
            }

            template<typename HazelcastSerializable>
            boost::shared_ptr<spi::impl::ClientInvocationFuture>
            invokeOnAddressInternal(const HazelcastSerializable &task, const Address &address,
                                    const std::string &uuid) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ExecutorServiceSubmitToAddressCodec::encodeRequest(name, uuid,
                                                                                            toData(
                                                                                                    task), address);

                boost::shared_ptr<spi::impl::ClientInvocationFuture> f = invokeOnTarget(request, address);
                return f;
            }

            boost::shared_ptr<spi::impl::ClientInvocationFuture>
            invokeOnPartitionOwner(std::auto_ptr<protocol::ClientMessage> &request, int partitionId);

            boost::shared_ptr<spi::impl::ClientInvocationFuture>
            invokeOnTarget(std::auto_ptr<protocol::ClientMessage> &request, const Address &target);

            template<typename T>
            boost::shared_ptr<T>
            retrieveResultFromMessage(const boost::shared_ptr<spi::impl::ClientInvocationFuture> &f) {
                serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
                std::auto_ptr<serialization::pimpl::Data> data = protocol::codec::ExecutorServiceSubmitToAddressCodec::ResponseParameters::decode(
                        *f->get()).response;
                return boost::shared_ptr<T>(serializationService.toObject<T>(data.get()));
            }

            template<typename T>
            boost::shared_ptr<ICompletableFuture<T> >
            checkSync(const boost::shared_ptr<spi::impl::ClientInvocationFuture> &f, const std::string &uuid,
                      int partitionId, bool preventSync, boost::shared_ptr<T> defaultValue) {
                bool sync = isSyncComputation(preventSync);
                if (sync) {
                    return retrieveResultSync<T>(f);
                } else {
                    return boost::shared_ptr<ICompletableFuture<T> >(
                            new proxy::IExecutorDelegatingFuture<T>(f, getContext(), uuid, defaultValue,
                                                                    SUBMIT_TO_PARTITION_DECODER(), name, partitionId));
                }

            }

            template<typename T>
            boost::shared_ptr<ICompletableFuture<T> >
            checkSync(const boost::shared_ptr<spi::impl::ClientInvocationFuture> &f, const std::string &uuid,
                      const Address &address, bool preventSync, boost::shared_ptr<T> defaultValue) {
                bool sync = isSyncComputation(preventSync);
                if (sync) {
                    return retrieveResultSync<T>(f);
                } else {
                    return boost::shared_ptr<ICompletableFuture<T> >(
                            new proxy::IExecutorDelegatingFuture<T>(f, getContext(), uuid, defaultValue,
                                                                    SUBMIT_TO_ADDRESS_DECODER(), name, address));
                }

            }

            template<typename T>
            boost::shared_ptr<ICompletableFuture<T> >
            retrieveResultSync(const boost::shared_ptr<spi::impl::ClientInvocationFuture> &f) {
                try {
                    boost::shared_ptr<T> response = retrieveResultFromMessage<T>(f);
                    boost::shared_ptr<ExecutorService> userExecutor = getContext().getClientExecutionService().getUserExecutor();
                    return boost::shared_ptr<ICompletableFuture<T> >(
                            new internal::util::executor::CompletedFuture<T>(response, userExecutor));
                } catch (exception::IException &e) {
                    boost::shared_ptr<ExecutorService> userExecutor = getContext().getClientExecutionService().getUserExecutor();
                    return boost::shared_ptr<ICompletableFuture<T> >(
                            new internal::util::executor::CompletedFuture<T>(
                                    boost::shared_ptr<exception::IException>(e.clone()), userExecutor));
                }
            }

            bool isSyncComputation(bool preventSync);

            Address getMemberAddress(const Member &member);

            int randomPartitionId();

            static const boost::shared_ptr<impl::ClientMessageDecoder> SUBMIT_TO_PARTITION_DECODER();

            static const boost::shared_ptr<impl::ClientMessageDecoder> SUBMIT_TO_ADDRESS_DECODER();

            static const int32_t MIN_TIME_RESOLUTION_OF_CONSECUTIVE_SUBMITS = 10;
            static const int32_t MAX_CONSECUTIVE_SUBMITS = 100;

            static const boost::shared_ptr<impl::ClientMessageDecoder> submitToAddressDecoder;
            static const boost::shared_ptr<impl::ClientMessageDecoder> submitToPartitionDecoder;

            util::Atomic<int32_t> consecutiveSubmits;
            util::Atomic<int64_t> lastSubmitTime;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_IEXECUTORSERVICE_H_ */
