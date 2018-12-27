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

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/cluster/memberselector/MemberSelectors.h"
#include "hazelcast/client/Future.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/MultiExecutionCallback.h"

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
     * Supports Quorum {@link com.hazelcast.config.QuorumConfig} since 3.10 in cluster versions 3.10 and higher.
     *
     *
     * @see ExecutionCallback
     * @see MultiExecutionCallback
     */
    class HAZELCAST_API IExecutorService : public proxy::ProxyImpl {
        public:
        /**
         * Executes a task on a randomly selected member.
         *
         * @param command        the task that is executed on a randomly selected member
         * @param memberSelector memberSelector
         * @throws RejectedExecutionException if no member is selected
         */
        template <typename HazelcastSerializable>
        void execute(const HazelcastSerializable &command, const cluster::memberselector::MemberSelector &memberSelector);

        /**
         * Executes a task on the owner of the specified key.
         *
         * @param command a task executed on the owner of the specified key
         * @param key     the specified key
         */
        template <typename HazelcastSerializable, typename K>
        void executeOnKeyOwner(const HazelcastSerializable &command, const K &key);

        /**
         * Executes a task on the specified member.
         *
         * @param command the task executed on the specified member
         * @param member  the specified member
         */
        template <typename HazelcastSerializable>
        void executeOnMember(const HazelcastSerializable &command, const Member &member);

        /**
         * Executes a task on each of the specified members.
         *
         * @param command the task executed on the specified members
         * @param members the specified members
         */
        template <typename HazelcastSerializable>
        void executeOnMembers(const HazelcastSerializable &command, const std::vector<Member> &members);

        /**
         * Executes a task on each of the selected members.
         *
         * @param command        a task executed on each of the selected members
         * @param memberSelector memberSelector
         * @throws RejectedExecutionException if no member is selected
         */
        template <typename HazelcastSerializable>
        void executeOnMembers(const HazelcastSerializable &command, const cluster::memberselector::MemberSelector &memberSelector);

        /**
         * Executes a task on all of the known cluster members.
         *
         * @param command a task executed  on all of the known cluster members
         */
        template <typename HazelcastSerializable>
        void executeOnAllMembers(const HazelcastSerializable &command);

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
        template <typename HazelcastSerializable, typename T>
        Future<T> submit(const HazelcastSerializable &task, const cluster::memberselector::MemberSelector &memberSelector);

        /**
         * Submits a task to the owner of the specified key and returns a Future
         * representing that task.
         *
         * @param task task submitted to the owner of the specified key
         * @param key  the specified key
         * @param <T>  the result type of callable
         * @return a Future representing pending completion of the task
         */
        template <typename HazelcastSerializable, typename T, typename K>
        Future<T> submitToKeyOwner(const HazelcastSerializable &task, const K &key);

        /**
         * Submits a task to the specified member and returns a Future
         * representing that task.
         *
         * @param task   the task submitted to the specified member
         * @param member the specified member
         * @param <T>    the result type of callable
         * @return a Future representing pending completion of the task
         */
        template <typename HazelcastSerializable, typename T>
        Future<T> submitToMember(const HazelcastSerializable &task, const Member &member);

        /**
         * Submits a task to given members and returns
         * map of Member-Future pairs representing pending completion of the task on each member
         *
         * @param task    the task submitted to given members
         * @param members the given members
         * @param <T>     the result type of callable
         * @return map of Member-Future pairs representing pending completion of the task on each member
         */
        template <typename HazelcastSerializable, typename T>
        std::map<Member, boost::shared_ptr<Future<T> > > submitToMembers(const HazelcastSerializable &task, const std::vector<Member> &members);

        /**
         * Submits a task to selected members and returns a
         * map of Member-Future pairs representing pending completion of the task on each member.
         *
         * @param task           the task submitted to selected members
         * @param memberSelector memberSelector
         * @param <T>            the result type of callable
         * @return map of Member-Future pairs representing pending completion of the task on each member
         * @throws java.util.concurrent.RejectedExecutionException if no member is selected
         */
        template <typename HazelcastSerializable, typename T>
        std::map<Member, boost::shared_ptr<Future<T> > > submitToMembers(const HazelcastSerializable &task, const cluster::memberselector::MemberSelector &memberSelector);

        /**
         * Submits task to all cluster members and returns a
         * map of Member-Future pairs representing pending completion of the task on each member.
         *
         * @param task the task submitted to all cluster members
         * @param <T>  the result type of callable
         * @return map of Member-Future pairs representing pending completion of the task on each member
         */
        template <typename HazelcastSerializable, typename T>
        std::map<Member, boost::shared_ptr<Future<T> > > submitToAllMembers(const HazelcastSerializable &task);

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
         * <p>Note: The {@link Executors} class includes a set of methods
         * that can convert some other common closure-like objects,
         * for example, {@link java.security.PrivilegedAction} to
         * {@link Callable} form so they can be submitted.
         *
         * @param task the task to submit
         * @param <T> the type of the task's result
         * @return a Future representing pending completion of the task
         * @throws RejectedExecutionException if the task cannot be
         *         scheduled for execution
         */
        template <typename HazelcastSerializable, typename T>
        Future<T> submit(const HazelcastSerializable &task);

        /**
         * Executes the given command at some time in the future.  The command
         * may execute in a new thread, in a pooled thread, or in the calling
         * thread, at the discretion of the {@code Executor} implementation.
         *
         * @param command the runnable task
         * @throws RejectedExecutionException if this task cannot be
         * accepted for execution
         */
        template <typename HazelcastSerializable>
        void execute(const HazelcastSerializable &command);

        /**
         * Submits a task to a random member. Caller will be notified of the result of the task by
         * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
         *
         * @param task     a task submitted to a random member
         * @param callback callback
         * @param <T>      the response type of callback
         */
        template <typename HazelcastSerializable, typename T>
        void submit(const HazelcastSerializable &task, const boost::shared_ptr<ExecutionCallback<T> > &callback);

        /**
         * Submits a task to randomly selected members. Caller will be notified for the result of the task by
         * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
         *
         * @param task           the task submitted to randomly selected members
         * @param memberSelector memberSelector
         * @param callback       callback
         * @param <T>            the response type of callback
         * @throws java.util.concurrent.RejectedExecutionException if no member is selected
         */
        template <typename HazelcastSerializable, typename T>
        void submit(const HazelcastSerializable &task, const cluster::memberselector::MemberSelector &memberSelector, const boost::shared_ptr<ExecutionCallback<T> > &callback);

        /**
         * Submits a task to the owner of the specified key. Caller will be notified for the result of the task by
         * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
         *
         * @param task     task submitted to the owner of the specified key
         * @param key      the specified key
         * @param callback callback
         * @param <T>      the response type of callback
         */
        template <typename HazelcastSerializable, typename T, typename K>
        void submitToKeyOwner(const HazelcastSerializable &task, const K &key, const boost::shared_ptr<ExecutionCallback<T> > &callback);

        /**
         * Submits a task to the specified member. Caller will be notified for the result of the task by
         * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
         *
         * @param task     the task submitted to the specified member
         * @param member   the specified member
         * @param callback callback
         * @param <T>      the response type of callback
         */
        template <typename HazelcastSerializable, typename T>
        void submitToMember(const HazelcastSerializable &task, Member member, const boost::shared_ptr<ExecutionCallback<T> > &callback);

        /**
         * Submits a task to the specified members. Caller will be notified for the result of the each task by
         * {@link MultiExecutionCallback#onResponse(Member, Object)}, and when all tasks are completed,
         * {@link MultiExecutionCallback#onComplete(java.util.Map)} will be called.
         *
         * @param task     the task submitted to the specified members
         * @param members  the specified members
         * @param callback callback
         */
        template <typename HazelcastSerializable, typename T>
        void submitToMembers(const HazelcastSerializable &task, const std::vector<Member> &members, const boost::shared_ptr<MultiExecutionCallback<T> > &callback);

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
        template <typename HazelcastSerializable, typename T>
        void submitToMembers(const HazelcastSerializable &task, const cluster::memberselector::MemberSelector &memberSelector, const boost::shared_ptr<MultiExecutionCallback<T> > &callback);

        /**
         * Submits task to all the cluster members. Caller will be notified for the result of each task by
         * {@link MultiExecutionCallback#onResponse(const Member &, const boost::shared_ptr<V> &)}, and when all tasks are completed,
         * {@link MultiExecutionCallback#onComplete(std::map)} will be called.
         *
         * @param task     the task submitted to all the cluster members
         * @param callback callback
         */
        template <typename HazelcastSerializable, typename T>
        void submitToAllMembers(const HazelcastSerializable &task, const boost::shared_ptr<MultiExecutionCallback<T> > &callback);

        /**
         * Initiates an orderly shutdown in which previously submitted
         * tasks are executed, but no new tasks will be accepted.
         * Invocation has no additional effect if already shut down.
         *
         * <p>This method does not wait for previously submitted tasks to
         * complete execution.  Use {@link #awaitTermination awaitTermination}
         * to do that.
         *
         * @throws SecurityException if a security manager exists and
         *         shutting down this ExecutorService may manipulate
         *         threads that the caller is not permitted to modify
         *         because it does not hold {@link
         *         java.lang.RuntimePermission}{@code ("modifyThread")},
         *         or the security manager's {@code checkAccess} method
         *         denies access.
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

        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_IEXECUTORSERVICE_H_ */
