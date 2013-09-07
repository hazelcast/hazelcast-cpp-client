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
#include "MultiExecutionCallbackWrapper.h"
#include "LocalTargetCallableRequest.h"
#include "ExecutorDefaultImpl.h"
#include "ExecutorDefaultResultImpl.h"
#include "ExecutorWithCallbackImpl.h"

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
            Future<Result> submit(Callable& task) {
                return executorDefault.submit<Result>(task);
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
            Future<Result> submit(Runnable& command, const Result& result) {
                executor::RunnableAdapter<Runnable> callable(command);
                return executorDefaultResult.submit(callable, result);
            }

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
                typename std::vector<Callable>::iterator it;
                int i = 0;
                for (it = tasks.begin(); it != tasks.end(); ++it) {
                    futures[i++] = submit<Result>(*it);
                }
                typename std::vector< Future<Result> >::iterator future_it;
                for (future_it = futures.begin(); future_it != futures.end(); ++future_it) {
                    (*future_it).get();
                }
                return futures;
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
            void execute(Runnable& command) {
                executor::RunnableAdapter<Runnable> callable(command);
                executorDefault.submit<bool>(callable);

            };

            /**
             * Executes task on owner of the specified key
             *
             * @param command task
             * @param key key
             */
            template<typename Runnable, typename Key>
            void executeOnKeyOwner(Runnable& command, const Key& key) {
                executor::RunnableAdapter<Runnable> callable(command);
                serialization::Data partitionKey = toData(key);
                executorDefault.submit<bool>(callable, partitionKey);
            }

            /**
             * Executes task on the specified member
             *
             * @param command task
             * @param member member
             */
            template<typename Runnable>
            void executeOnMember(Runnable& command, const connection::Member& member) {
                executor::RunnableAdapter<Runnable> callable(command);
                if (context->getClusterService().isMemberExists(member.getAddress())) {
                    executorDefault.submit<bool>(callable, member.getAddress());
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
            void executeOnMembers(Runnable& command, const std::vector<connection::Member>& members) {
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
            void executeOnAllMembers(Runnable& command) {
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
            Future<Result> submitToKeyOwner(Callable& task, const Key& key) {
                serialization::Data partitionKey = toData(key);
                return executorDefault.submit<Result>(task, partitionKey);
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
            Future<Result> submitToMember(Callable& task, const connection::Member& member) {
                if (context->getClusterService().isMemberExists(member.getAddress())) {
                    return executorDefault.submit<Result>(task, member.getAddress());
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
            std::map<connection::Member, Future<Result> > submitToMembers(Callable& task, const std::vector<connection::Member>& members) {
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
            std::map<connection::Member, Future<Result> > submitToAllMembers(Callable& task) {
                std::vector<connection::Member> members = context->getClusterService().getMemberList();
                submitToMembers<Result>(task, members);
            };

            /**
             * Submits task to a random member. Caller will be notified for the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
             *
             * @param task task
             * @param callback callback
             */

            template<typename Result, typename Callable, typename ExecutionCallback>
            void submit(Callable& task, ExecutionCallback& callback) {
                return executorWithCallback.submit<Result>(task, callback);
            };

            /**
             * Submits task to owner of the specified key. Caller will be notified for the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
             *
             * @param task task
             * @param callback callback
             */
            template<typename Result, typename Callable, typename Key, typename ExecutionCallback>
            void submitToKeyOwner(Callable& task, const Key& result, ExecutionCallback& callback) {
                serialization::Data partitionKey = toData(result);
                executorWithCallback.submit<Result>(task, partitionKey, callback);
            };

            /**
             * Submits task to the specified member. Caller will be notified for the result of the task by
             * {@link ExecutionCallback#onResponse(Object)} or {@link ExecutionCallback#onFailure(Throwable)}.
             *
             * @param task task
             * @param callback callback
             */

            template<typename Result, typename Callable, typename ExecutionCallback>
            void submitToMember(Callable& task, const connection::Member& member, ExecutionCallback& callback) {
                if (context->getClusterService().isMemberExists(member.getAddress())) {
                    executorWithCallback.submit<Result>(task, member.getAddress(), callback);
                } else {
                    throw exception::IException("IExecuterService::executeOnMember()", "Member is not available!!!");
                }
            };

            /**
             * Submits task to the specified members. Caller will be notified for the result of the each task by
             * {@link MultiExecutionCallback#onResponse(Member, Object)} and when all tasks are completed
             * {@link MultiExecutionCallback#onResponse(Member, Object)} will be called.
             *
             * @param task task
             * @param callback callback
             */
            template<typename Result, typename Callable, typename MultiExecutionCallback>
            void submitToMembers(Callable& task, const std::vector<connection::Member>& members, MultiExecutionCallback& callback) {
                std::map<connection::Member, Future<Result> > result;
                std::vector<connection::Member>::const_iterator it;
                for (it = members.begin(); it != members.end(); ++it) {
                    executorWithCallback.submitMulti<Result>(task, *it, callback);
                }
            };

            /**
             * Submits task to the all cluster members. Caller will be notified for the result of the each task by
             * {@link MultiExecutionCallback#onResponse(Member, Object)} and when all tasks are completed
             * {@link MultiExecutionCallback#onResponse(Member, Object)} will be called.
             *
             * @param task task
             * @param callback callback
             */
            template<typename Result, typename Callable, typename MultiExecutionCallback>
            void submitToAllMembers(Callable& task, MultiExecutionCallback& callback) {
                std::vector<connection::Member> members = context->getClusterService().getMemberList();
                submitToMembers<Result>(task, members);
            }


        private:
            IExecutorService() {

            }

            void init(const std::string& instanceName, spi::ClientContext *clientContext) {
                context = clientContext;
                this->instanceName = instanceName;
                executorDefault.init(&this->instanceName, clientContext);
                executorDefaultResult.init(&this->instanceName, clientContext);
                executorWithCallback.init(&this->instanceName, clientContext);
            };


            template<typename Result, typename Request>
            Result invoke(const Request& request, const Address& target) {
                return context->getInvocationService().invokeOnTarget<Result>(request, target);
            }

            template<typename Result, typename Request>
            Result invoke(const Request& request) {
                return context->getInvocationService().invokeOnRandomTarget<Result>(request);
            }

            template<typename T>
            serialization::Data toData(const T& o) {
                return context->getSerializationService().toData<T>(&o);
            }

            executor::ExecutorDefaultImpl executorDefault;
            executor::ExecutorDefaultResultImpl executorDefaultResult;
            executor::ExecutorWithCallbackImpl executorWithCallback;
            std::string instanceName;
            spi::ClientContext *context;
        };
    }
}

#endif //HAZELCAST_IExecutorService
