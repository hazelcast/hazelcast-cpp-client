//
// Created by sancar koyunlu on 8/14/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_IExecutorService
#define HAZELCAST_IExecutorService

#include "ClientContext.h"
#include "Future.h"
#include "Data.h"
#include "SerializationService.h"
#include "PartitionService.h"
#include "InvocationService.h"
#include "TargetCallableRequest.h"
#include "RunnableAdapter.h"
#include "ShutdownRequest.h"
#include "Member.h"
#include "IsShutdownRequest.h"

namespace hazelcast {
    namespace client {
        /**
        * Distributed implementation of {@link ExecutorService}.
        * IExecutorService provides additional methods like executing tasks
        * on a specific member, on a member who is owner of a specific key,
        * executing a tasks on multiple members and listening execution result using a callback.
        *
        * @see ExecutorService
        * @see ExecutionCallback
        * @see MultiExecutionCallback
        */

        class IExecutorService {
            friend class HazelcastClient;

        public:

            /* Initiates an orderly shutdown in which previously submitted
            * tasks are executed, but no new tasks will be accepted.
            * Invocation has no additional effect if already shut down.
            *
            * @throws SecurityException if a security manager exists and
            *         shutting down this ExecutorService may manipulate
                    *         threads that the caller is not permitted to modify
                    *         because it does not hold {@link
                        *         java.lang.RuntimePermission}<tt>("modifyThread")</tt>,
            *         or the security manager's <tt>checkAccess</tt> method
            *         denies access.
            */
            void shutdown() {
                std::vector<connection::Member>::iterator it;
                std::vector<connection::Member> memberList = context->getClusterService().getMemberList();
                executor::ShutdownRequest request(instanceName);
                for (it = memberList.begin(); it != memberList.end(); ++it) {
                    invoke<bool>(request, (*it).getAddress());
                }
            };


            /**
             * Returns <tt>true</tt> if this executor has been shut down.
             *
             * @return <tt>true</tt> if this executor has been shut down
             */
            bool isShutdown() {
                executor::IsShutdownRequest request(instanceName);
                return invoke<bool>(request);
            };


            /**
             * Submits a value-returning task for execution and returns a
             * Future representing the pending results of the task. The
             * Future's <tt>get</tt> method will return the task's result upon
             * successful completion.
             *
             * <p>
             * If you would like to immediately block waiting
             * for a task, you can use constructions of the form
             * <tt>result = exec.submit(aCallable).get();</tt>
             *
             * <p> Note: The {@link Executors} class includes a set of methods
             * that can convert some other common closure-like objects,
             * for example, {@link java.security.PrivilegedAction} to
             * {@link Callable} form so they can be submitted.
             *
             * @param task the task to submit
             * @return a Future representing pending completion of the task
             * @throws RejectedExecutionException if the task cannot be
             *         scheduled for execution
             * @throws NullPointerException if the task is null
             */

            template<typename Result, typename Callable>
            Future<Result> submit(const Callable& task) {
                serialization::Data partitionKey = getTaskPartitionKey(task);
                return submitToKeyOwnerInternal<Result>(task, partitionKey);
            };

            /**
             * Submits a Runnable task for execution and returns a Future
             * representing that task. The Future's <tt>get</tt> method will
             * return the given result upon successful completion.
             *
             * @param task the task to submit
             * @param result the result to return
             * @return a Future representing pending completion of the task
             * @throws RejectedExecutionException if the task cannot be
             *         scheduled for execution
             * @throws NullPointerException if the task is null
             */
            template<typename Result, typename Runnable>
            Future<Result> submit(const Runnable& command, const Result& result) {
                serialization::Data key = getTaskPartitionKey(command);
                executor::RunnableAdapter<Runnable> callable;
                return submitToKeyOwnerInternalWithDefaultResult(callable, key, result);
            }

            /**
             * Submits a Runnable task for execution and returns a Future
             * representing that task. The Future's <tt>get</tt> method will
             * return <tt>null</tt> upon <em>successful</em> completion.
             *
             * @param task the task to submit
             * @return a Future representing pending completion of the task
             * @throws RejectedExecutionException if the task cannot be
             *         scheduled for execution
             * @throws NullPointerException if the task is null
             */
//            template<typename Result, typename Runnable> //TODO
//            Future<Result> submit(const Runnable& command) {
//                serialization::Data key = getTaskPartitionKey(command);
//                executor::RunnableAdapter<Runnable> callable;
//                return submitToKeyOwnerInternal(callable, key);
//            }

            /**
             * Executes the given tasks, returning a list of Futures holding
             * their status and results when all complete.
             * {@link Future#isDone} is <tt>true</tt> for each
             * element of the returned list.
             * Note that a <em>completed</em> task could have
             * terminated either normally or by throwing an exception.
             * The results of this method are undefined if the given
             * collection is modified while this operation is in progress.
             *
             * @param tasks the collection of tasks
             * @return A list of Futures representing the tasks, in the same
             *         sequential order as produced by the iterator for the
             *         given task list, each of which has completed.
             * @throws InterruptedException if interrupted while waiting, in
             *         which case unfinished tasks are cancelled.
             * @throws NullPointerException if tasks or any of its elements are <tt>null</tt>
             * @throws RejectedExecutionException if any task cannot be
             *         scheduled for execution
             */

            template<typename Result, typename Callable>
            std::vector< Future<Result> > invokeAll(const std::vector<Callable>& tasks) {
                std::vector< Future<Result> > futures(tasks.size());
                std::vector< Future<Result> > result(tasks.size());
                typename std::vector<Callable>::iterator it;
                int i = 0;
                for (it = tasks.begin(); it != tasks.end(); ++it) {
                    futures[i++] = submit<Result>(*it);
                }
                typename std::vector< Future<Result> >::iterator future_it;
                i = 0;
                for (future_it = futures.begin(); future_it != futures.end(); ++future_it) {
                    try {
                        result[i] = (*it).get();
                    } catch (std::exception& e) {
                        //TODO add has_exception method to Future
                    }
                    i++;
                }
                return result;
            }

            /**
             * Executes the given command at some time in the future.  The command
             * may execute in a new thread, in a pooled thread, or in the calling
             * thread, at the discretion of the <tt>Executor</tt> implementation.
             *
             * @param command the runnable task
             * @throws RejectedExecutionException if this task cannot be
             * accepted for execution.
             * @throws NullPointerException if command is null
             */
            template <typename Runnable>
            void execute(const Runnable& command) {
                executor::RunnableAdapter<Runnable> callable(command);
                serialization::Data partitionKey = getTaskPartitionKey(command);
                submitToKeyOwnerInternal<bool>(callable, partitionKey);

            };

            /**
             * Executes task on owner of the specified key
             *
             * @param command task
             * @param key key
             */
            template<typename Runnable, typename Key>
            void executeOnKeyOwner(const Runnable& command, const Key& key) {
                executor::RunnableAdapter<Runnable> callable(command);
                serialization::Data partitionKey = toData(key);
                submitToKeyOwnerInternal<bool>(callable, partitionKey);
            }

            /**
             * Executes task on the specified member
             *
             * @param command task
             * @param member member
             */
            template<typename Runnable>
            void executeOnMember(const Runnable& command, const connection::Member& member) {
                executor::RunnableAdapter<Runnable> callable(command);
                if (context->getClusterService().isMemberExists(member.getAddress())) {
                    submitToTargetInternal<bool>(callable, member.getAddress());
                } else {
                    throw exception::IException("IExecuterService::executeOnMember()", "Member is not available!!!");
                }
            }

            /**
             * Executes task on each of the specified members
             *
             * @param command task
             * @param members members
             */
            template<typename Runnable>
            void executeOnMembers(const Runnable& command, const std::vector<connection::Member>& members) {
//                std::vector<connection::Member>::const_iterator it;
//                for (it = members.begin(); it != members.end(); ++it) {
//                    executeOnMember(command, *it);
//                }
                std::for_each(members.begin(), members.end(), boost::bind(&IExecutorService::executeOnMember, boost::cref(command)));
            }

            /**
             * Executes task on all of known cluster members
             *
             * @param command task
             */
            template<typename Runnable>
            void executeOnAllMembers(const Runnable& command) {
                std::vector<connection::Member> members = context->getClusterService().getMemberList();
                executeOnMembers(command, members);
            };

            /**
             * Submits task to owner of the specified key and returns a Future
             * representing that task.
             *
             * @param task task
             * @param key key
             * @return a Future representing pending completion of the task
             */
            template<typename Result, typename Callable, typename Key>
            Future<Result> submitToKeyOwner(const Callable& task, const Key& result) {
                serialization::Data partitionKey = toData(result);
                return submitToKeyOwnerInternal<Result>(task, partitionKey);
            };

            /**
             * Submits task to specified member and returns a Future
             * representing that task.
             *
             * @param task task
             * @param member member
             * @return a Future representing pending completion of the task
             */
            template<typename Result, typename Callable>
            Future<Result> submitToMember(const Callable& task, const connection::Member& member) {
                if (context->getClusterService().isMemberExists(member.getAddress())) {
                    return submitToTargetInternal<Result>(task, member.getAddress());
                } else {
                    throw exception::IException("IExecuterService::executeOnMember()", "Member is not available!!!");
                }
            }

            /**
             * Submits task to given members and returns
             * map of Member-Future pairs representing pending completion of the task on each member
             *
             * @param task task
             * @param members members
             * @return map of Member-Future pairs representing pending completion of the task on each member
             */
            template<typename Result, typename Callable>
            std::map<connection::Member, Future<Result> > submitToMembers(const Callable& task, const std::vector<connection::Member>& members) {
                std::map<connection::Member, Future<Result> > result;
                std::vector<connection::Member>::const_iterator it;
                for (it = members.begin(); it != members.end(); ++it) {
                    result[*it] = submitToMember<Result>(task, *it);
                }
                return result;
            };


            /**
             * Submits task to all cluster members and returns
             * map of Member-Future pairs representing pending completion of the task on each member
             *
             * @param task task
             * @return map of Member-Future pairs representing pending completion of the task on each member
             */

            template<typename Result, typename Callable>
            std::map<connection::Member, Future<Result> > submitAllToMembers(const Callable& task) {
                std::vector<connection::Member> members = context->getClusterService().getMemberList();
                submitToMembers(task, members);
            }

            /**
             * Submits task to a random member. Caller will be notified for the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
             *
             * @param task task
             * @param callback callback
             */
//                    void submit(Runnable task, ExecutionCallback callback);

            /**
             * Submits task to owner of the specified key. Caller will be notified for the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
             *
             * @param task task
             * @param callback callback
             */
//                    void submitToKeyOwner(Runnable task, Object key, ExecutionCallback callback);

            /**
             * Submits task to the specified member. Caller will be notified for the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
             *
             * @param task task
             * @param callback callback
             */
//                    void submitToMember(Runnable task, Member member, ExecutionCallback callback);

            /**
             * Submits task to the specified members. Caller will be notified for the result of the each task by
             * {@link MultiExecutionCallback#onResponse(Member, Object)} and when all tasks are completed
             * {@link MultiExecutionCallback#onResponse(Member, Object)} will be called.
             *
             * @param task task
             * @param callback callback
             */
//                    void submitToMembers(Runnable task, Collection<Member> members, MultiExecutionCallback callback);

            /**
             * Submits task to the all cluster members. Caller will be notified for the result of the each task by
             * {@link MultiExecutionCallback#onResponse(Member, Object)} and when all tasks are completed
             * {@link MultiExecutionCallback#onResponse(Member, Object)} will be called.
             *
             * @param task task
             * @param callback callback
             */
//                    void submitToAllMembers(Runnable task, MultiExecutionCallback callback);

            /**
             * Submits task to a random member. Caller will be notified for the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
             *
             * @param task task
             * @param callback callback
             */
//                    <T> void submit(Callable<T> task, ExecutionCallback<T> callback);

            /**
             * Submits task to owner of the specified key. Caller will be notified for the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
             *
             * @param task task
             * @param callback callback
             */
//                    <T> void submitToKeyOwner(Callable<T> task, Object key, ExecutionCallback<T> callback);

            /**
             * Submits task to the specified member. Caller will be notified for the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
             *
             * @param task task
             * @param callback callback
             */
//                    <T> void submitToMember(Callable<T> task, Member member, ExecutionCallback<T> callback);

            /**
             * Submits task to the specified members. Caller will be notified for the result of the each task by
             * {@link MultiExecutionCallback#onResponse(Member, Object)} and when all tasks are completed
             * {@link MultiExecutionCallback#onResponse(Member, Object)} will be called.
             *
             * @param task task
             * @param callback callback
             */
//                    <T> void submitToMembers(Callable<T> task, Collection<Member> members, MultiExecutionCallback callback);

            /**
             * Submits task to the all cluster members. Caller will be notified for the result of the each task by
             * {@link MultiExecutionCallback#onResponse(Member, Object)} and when all tasks are completed
             * {@link MultiExecutionCallback#onResponse(Member, Object)} will be called.
             *
             * @param task task
             * @param callback callback
             */
//                    <T> void submitToAllMembers(Callable<T> task, MultiExecutionCallback callback);


        private:
            void init(const std::string& instanceName, spi::ClientContext *clientContext) {
                context = clientContext;
                this->instanceName = instanceName;
            };

            template<typename Result, typename Callable >
            Future<Result> submitToKeyOwnerInternal(const Callable& task, const serialization::Data& partitionKey) {
                spi::PartitionService & partitionService = context->getPartitionService();
                int partitionId = partitionService.getPartitionId(partitionKey);
                util::AtomicPointer<Address> pointer = partitionService.getPartitionOwner(partitionId);
                return submitToTargetInternal(task, *pointer);
            }

            template<typename Result, typename Callable >
            Future<Result> submitToKeyOwnerInternalWithDefaultResult(const Callable& task, const serialization::Data& partitionKey, const Result& result) {
                spi::PartitionService & partitionService = context->getPartitionService();
                int partitionId = partitionService.getPartitionId(partitionKey);
                util::AtomicPointer<Address> pointer = partitionService.getPartitionOwner(partitionId);
                return submitToTargetInternalWithDefaultResult(task, *pointer, result);
            }


            template<typename Result, typename Callable >
            Future<Result> submitToTargetInternal(const Callable& task, const Address& address) {
                executor::TargetCallableRequest<Callable> request(instanceName, task, address);
                util::AtomicPointer<pImpl::FutureBase<Result> > futureBase(new pImpl::FutureBase<Result>);
                Future<Result> future(futureBase);
                boost::thread asyncInvokeThread(boost::bind(&IExecutorService::asyncInvoke, boost::cref(request), boost::cref(address)), boost::cref(future));
                return future;
            }

            template<typename Result, typename Callable >
            Future<Result> submitToTargetInternalWithDefaultResult(const Callable& task, const Address& address, const Result& result) {
                executor::TargetCallableRequest<Callable> request(instanceName, task, address);
                util::AtomicPointer<pImpl::FutureBase<Result> > futureBase(new pImpl::FutureBase<Result>);
                Future<Result> future(futureBase);
                boost::thread asyncInvokeThread(boost::bind(&IExecutorService::asyncInvokeWithDefaultResult, boost::cref(request), boost::cref(address)), boost::cref(future), boost::cref(result));
                return future;
            }

            template<typename Result, typename Callable>
            void asyncInvoke(const executor::TargetCallableRequest<Callable> & request, const Address& address, const Future<Result>& future) {
                util::AtomicPointer<pImpl::FutureBase<Result> > futureBase(new pImpl::FutureBase<Result>(invoke<Result>(request, address)));
                future.updateBasePtr(futureBase);
            }

            template<typename Result, typename Callable>
            void asyncInvokeWithDefaultResult(const executor::TargetCallableRequest<Callable> & request, const Address& address, const Future<Result>& future, const Result& result) {
                invoke<bool>(request, address);
                util::AtomicPointer<pImpl::FutureBase<Result> > futureBase(new pImpl::FutureBase<Result>(result));
                future.updateBasePtr(futureBase);
            }

            template<typename Result, typename Request>
            Result invoke(const Request& request, const Address& target) {
                return context->getInvocationService().invokeOnTarget<Result>(request, target);
            }

            template<typename Result, typename Request>
            Result invoke(const Request& request) {
                return context->getInvocationService().invokeOnRandomTarget<Result>(request);
            }

            template<typename Task>
            serialization::Data getTaskPartitionKey(const Task& task) {
                return toData(task.getPartitionKey());
            }

            template<typename T>
            serialization::Data toData(const T& o) {
                return context->getSerializationService().toData<T>(&o);
            }

            std::string instanceName;
            spi::ClientContext *context;
        };
    }
}

#endif //HAZELCAST_IExecutorService
