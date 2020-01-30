/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_ABSTRACTINVOCATIONFUTURE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_ABSTRACTINVOCATIONFUTURE_H_

#include <hazelcast/util/ConditionVariable.h>
#include <hazelcast/util/ExceptionUtil.h>
#include <hazelcast/util/Sync.h>
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/spi/InternalCompletableFuture.h"
#include "hazelcast/util/Executor.h"

#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/util/concurrent/TimeUnit.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/util/concurrent/CancellationException.h"

using namespace hazelcast::util;
using namespace hazelcast::client::exception;


namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                /**
                 *
                 * @param <T>
                 */
                template<typename T>
                class AbstractInvocationFuture
                        : public InternalCompletableFuture<T>,
                          public std::enable_shared_from_this<AbstractInvocationFuture<T> > {
                public:
                    virtual ~AbstractInvocationFuture() {
                    }

                    virtual void andThen(const std::shared_ptr<ExecutionCallback<T> > &callback) {
                        andThen(callback, defaultExecutor);
                    }

                    virtual void andThen(const std::shared_ptr<ExecutionCallback<T> > &callback,
                                         const std::shared_ptr<Executor> &executor) {
                        Preconditions::isNotNull(callback, "callback");
                        Preconditions::isNotNull(executor, "executor");

                        std::shared_ptr<BaseState> waiter(new ExecutionCallbackState(callback));
                        std::shared_ptr<BaseState> response = registerWaiter(waiter, executor);
                        if (response != VOIDOBJECT) {
                            unblock(callback, executor);
                        }
                    }

                    virtual bool cancel(bool mayInterruptIfRunning) {
                        return complete(std::shared_ptr<IException>(
                                new concurrent::CancellationException("AbstractInvocationFuture::cancel",
                                                                      "Task was cancelled.")));
                    }

                    virtual bool isCancelled() {
                        const std::shared_ptr<BaseState> currentState = state.get();
                        return (currentState->getType() == BaseState::Exception &&
                                std::static_pointer_cast<ExceptionState>(
                                        currentState)->getException()->getErrorCode() == protocol::CANCELLATION);
                    }

                    virtual bool isDone() {
                        return isDone(state);
                    }

                    virtual std::shared_ptr<T> get() {
                        std::shared_ptr<ThreadState> thread = std::shared_ptr<ThreadState>(
                                new ThreadState(util::getCurrentThreadId()));
                        std::shared_ptr<BaseState> response = registerWaiter(thread, std::shared_ptr<Executor>());
                        if (response != VOIDOBJECT) {
                            // no registration was done since a value is available.
                            return resolveAndThrowIfException(response);
                        }

                        for (;;) {
                            thread->park();
                            if (isDone()) {
                                return resolveAndThrowIfException(state);
                            }
                        }
                    }

                    virtual std::shared_ptr<T> get(int64_t timeout, const TimeUnit &unit) {
                        std::shared_ptr<ThreadState> thread = std::shared_ptr<ThreadState>(
                                new ThreadState(util::getCurrentThreadId()));
                        std::shared_ptr<BaseState> response = registerWaiter(thread, std::shared_ptr<Executor>());
                        if (response != VOIDOBJECT) {
                            return resolveAndThrowIfException(response);
                        }

                        int64_t deadlineNanos = util::currentTimeNanos() + unit.toNanos(timeout);
                        int64_t timeoutNanos = unit.toNanos(timeout);
                        while (timeoutNanos > 0) {
                            thread->parkNanos(timeoutNanos);
                            timeoutNanos = deadlineNanos - util::currentTimeNanos();

                            if (isDone()) {
                                return resolveAndThrowIfException(state);
                            }
                        }

                        unregisterWaiter(thread);
                        throw (ExceptionBuilder<exception::TimeoutException>("AbstractInvocationFuture::get(timeout, unit)")
                                << "Timeout: " << unit.toMillis(timeout) << " msecs").build();
                    }

                    virtual std::shared_ptr<T> join() {
                        try {
                            return get();
                        } catch (exception::IException &e) {
                            util::ExceptionUtil::rethrow(e);
                        }
                        return std::shared_ptr<T>();
                    }

                    /**
                     * Can be called multiple times, but only the first answer will lead to the
                     * future getting triggered. All subsequent complete calls are ignored.
                     *
                     * @param value The type of response to offer.
                     * @return <tt>true</tt> if offered response, either a final response or an
                     * internal response, is set/applied, <tt>false</tt> otherwise. If <tt>false</tt>
                     * is returned, that means offered response is ignored because a final response
                     * is already set to this future.
                     */
                    virtual bool complete(const std::shared_ptr<T> &value) {
                        std::shared_ptr<BaseState> newState = std::shared_ptr<BaseState>(new ValueState(value));

                        return innerComplete(newState);
                    }

                    virtual bool complete(const std::shared_ptr<exception::IException> &exception) {
                        std::shared_ptr<BaseState> newState = std::shared_ptr<BaseState>(
                                new ExceptionState(exception));

                        return innerComplete(newState);
                    }

                protected:
                    typedef ExecutionCallback<T> CALLBACKTYPE;

                    class BaseState {
                    public:
                        enum Type {
                            VOIDTYPE,
                            WaitNode,
                            Thread,
                            ExecutionCallback,
                            Value,
                            Exception
                        };

                        BaseState(Type type) : type(type) {}

                        virtual ~BaseState() {
                        }

                        Type getType() const {
                            return type;
                        }

                    private:
                        Type type;
                    };

                    class ThreadState : public BaseState {
                    public:
                        ThreadState(int64_t threadId) : BaseState(BaseState::Thread), threadId(threadId), unparked(false) {}

                        void park() {
                            parkNanos(INT64_MAX);
                        }

                        void parkNanos(int64_t nanos) {
                            util::LockGuard guard(mutex);
                            if (unparked) {
                                return;
                            }
                            conditionVariable.waitNanos(mutex, nanos);
                        }

                        void unpark() {
                            util::LockGuard guard(mutex);
                            unparked = true;
                            conditionVariable.notify_all();
                        }

                    private:
                        int64_t threadId;
                        util::Mutex mutex;
                        util::ConditionVariable conditionVariable;
                        bool unparked;
                    };

                    class ValueState : public BaseState {
                    public:
                        ValueState(const std::shared_ptr<T> &value) : BaseState(BaseState::Value), value(value) {}

                        const std::shared_ptr<T> &getValue() const {
                            return value;
                        }

                    private:
                        const std::shared_ptr<T> value;
                    };

                    class ExceptionState : public BaseState {
                    public:
                        ExceptionState(const std::shared_ptr<IException> &exception) : BaseState(
                                BaseState::Exception), exception(exception) {}

                        const std::shared_ptr<IException> &getException() const {
                            return exception;
                        }

                    private:
                        const std::shared_ptr<exception::IException> exception;
                    };

                    class ExecutionCallbackState : public BaseState {
                    public:
                        ExecutionCallbackState(const std::shared_ptr<CALLBACKTYPE> &callback)
                                : BaseState(BaseState::ExecutionCallback), callback(callback) {}

                        const std::shared_ptr<CALLBACKTYPE> &getCallback() const {
                            return callback;
                        }

                    private:
                        const std::shared_ptr<CALLBACKTYPE> callback;
                    };

                    class VoidState : public BaseState {
                    public:
                        VoidState() : BaseState(BaseState::VOIDTYPE) {}
                    };

                    /**
                     * Linked nodes to record waiting {@link Thread} or {@link ExecutionCallback}
                     * instances using a Treiber stack.
                     * <p>
                     * A waiter is something that gets triggered when a response comes in. There
                     * are 2 types of waiters:
                     * <ol>
                     * <li>Thread: when a future.get is done.</li>
                     * <li>ExecutionCallback: when a future.andThen is done</li>
                     * </ol>
                     * The waiter is either a Thread or an ExecutionCallback.
                     * <p>
                     * The {@link WaitNode} is effectively immutable. Once the WaitNode is set in
                     * the 'state' field, it will not be modified. Also updating the state,
                     * introduces a happens before relation so the 'next' field can be read safely.
                     */
                    class WaitNode : public BaseState {
                    public:
                        WaitNode(const std::shared_ptr<BaseState> &waiter,
                                 const std::shared_ptr<Executor> &executor) : BaseState(BaseState::WaitNode),
                                                                                waiter(waiter), executor(executor) {}

                        const std::shared_ptr<BaseState> &getWaiter() const {
                            return waiter;
                        }

                        const std::shared_ptr<Executor> &getExecutor() const {
                            return executor;
                        }

                        const std::shared_ptr<BaseState> waiter;
                        util::Sync<std::shared_ptr<BaseState> > next;
                        std::shared_ptr<Executor> executor;
                    };

                    AbstractInvocationFuture(const std::shared_ptr<Executor> &defaultExecutor, ILogger &logger)
                            : defaultExecutor(defaultExecutor), logger(logger), state(VOIDOBJECT) {}

                    const std::shared_ptr<BaseState> getState() {
                        return state.get();
                    }

                    virtual const std::shared_ptr<BaseState> resolve(const std::shared_ptr<BaseState> &value) {
                        if (value->getType() == BaseState::Exception) {
                            return std::shared_ptr<BaseState>(new ExceptionState(
                                    std::shared_ptr<exception::IException>(
                                            new ExecutionException("AbstractInvocationFuture::resolve",
                                                                   "ExecutionException for the future.",
                                                                   (std::static_pointer_cast<ExceptionState>(
                                                                           value))->getException()))));
                        }
                        return value;
                    }

                    virtual std::shared_ptr<T>
                    resolveAndThrowIfException(const std::shared_ptr<BaseState> &response) = 0;

                    virtual std::string invocationToString() const = 0;

                    virtual void onComplete() {
                    }

                    // this method should not be needed; but there is a difference between client and server how it handles async throwables
                    static std::shared_ptr<exception::IException>
                    unwrap(const std::shared_ptr<exception::IException> &throwable) {
                        if (throwable->getErrorCode() == exception::ExecutionException::ERROR_CODE &&
                            throwable->getCause().get() != NULL) {
                            return throwable->getCause();
                        }
                        return throwable;
                    }

                    const std::shared_ptr<Executor> defaultExecutor;
                    util::ILogger &logger;

                    static const std::shared_ptr<BaseState> VOIDOBJECT;
                private:

                    bool compareAndSetState(const std::shared_ptr<BaseState> &oldState,
                                            const std::shared_ptr<BaseState> &newState) {
                        return state.compareAndSet(oldState, newState);
                    }

                    static bool isDone(const std::shared_ptr<BaseState> &state) {
                        if (state.get() == NULL) {
                            return true;
                        }

                        typename BaseState::Type type = state->getType();
                        return !(type == BaseState::VOIDTYPE || type == BaseState::WaitNode ||
                                 type == BaseState::Thread ||
                                 type == BaseState::ExecutionCallback);
                    }

                    /**
                     * Registers a waiter (thread/ExecutionCallback) that gets notified when
                     * the future completes.
                     *
                     * @param waiter   the waiter
                     * @param executor the {@link Executor} to use in case of an
                     *                 {@link ExecutionCallback}.
                     * @return VOID if the registration was a success, anything else but void
                     * is the response.
                     */
                    std::shared_ptr<BaseState> registerWaiter(const std::shared_ptr<BaseState> &waiter,
                                                                const std::shared_ptr<Executor> &executor) {
                        std::shared_ptr<WaitNode> waitNode;
                        for (;;) {
                            const std::shared_ptr<BaseState> oldState = state.get();
                            if (isDone(oldState)) {
                                return oldState;
                            }

                            std::shared_ptr<BaseState> newState;
                            if (oldState->getType() == BaseState::VOIDTYPE &&
                                (executor.get() == NULL || executor == defaultExecutor)) {
                                // nothing is syncing on this future, so instead of creating a WaitNode, we just set the waiter
                                newState = waiter;
                            } else {
                                // something already has been registered for syncing, so we need to create a WaitNode
                                waitNode.reset(new WaitNode(waiter, executor));

                                waitNode->next = oldState;
                                newState = waitNode;
                            }

                            if (compareAndSetState(oldState, newState)) {
                                // we have successfully registered
                                return VOIDOBJECT;
                            }
                        }
                    }

                    void unregisterWaiter(const std::shared_ptr<ThreadState> &waiter) {
                        std::shared_ptr<WaitNode> prev;
                        std::shared_ptr<BaseState> current = state;

                        while (current.get() != NULL) {
                            std::shared_ptr<BaseState> currentWaiter =
                                    current->getType() == BaseState::WaitNode ? (std::static_pointer_cast<WaitNode>(
                                            current))->waiter : current;
                            std::shared_ptr<BaseState> next =
                                    current->getType() == BaseState::WaitNode ? (std::static_pointer_cast<WaitNode>(
                                            current))->next.get() : std::shared_ptr<BaseState>();

                            if (currentWaiter == waiter) {
                                // it is the item we are looking for, so lets try to remove it
                                if (prev.get() == NULL) {
                                    // it's the first item of the stack, so we need to change the head to the next
                                    std::shared_ptr<BaseState> n = next.get() == NULL ? VOIDOBJECT : next;
                                    // if we manage to CAS we are done, else we need to restart
                                    current = compareAndSetState(current, n) ? std::shared_ptr<BaseState>()
                                                                             : state.get();
                                } else {
                                    // remove the current item (this is done by letting the prev.next point to the next instead of current)
                                    prev->next = next;
                                    // end the loop
                                    current.reset();
                                }
                            } else {
                                // it isn't the item we are looking for, so lets move on to the next
                                prev = current->getType() == BaseState::WaitNode ? std::static_pointer_cast<WaitNode>(
                                        current) : std::shared_ptr<WaitNode>();
                                current = next;
                            }
                        }
                    }

                class CallbackRunner : public util::Runnable {
                    public:
                        CallbackRunner(const std::shared_ptr<AbstractInvocationFuture> &future,
                                       const std::shared_ptr<CALLBACKTYPE> &callback) : future(future),
                                                                                          callback(callback) {}

                        virtual const std::string getName() const {
                            return "ExecutionCallback Runner for Future";
                        }

                        virtual void run() {
                            try {
                                std::shared_ptr<BaseState> value = future->resolve(future->state.get());
                                if (value->getType() == BaseState::Exception) {
                                    std::shared_ptr<ExceptionState> exceptionState = std::static_pointer_cast<ExceptionState>(
                                            value);

                                    std::shared_ptr<exception::IException> error = AbstractInvocationFuture<T>::unwrap(
                                            exceptionState->getException());
                                    callback->onFailure(error);
                                } else {
                                    std::shared_ptr<ValueState> valueState = std::static_pointer_cast<ValueState>(
                                            value);
                                    callback->onResponse(valueState->getValue());
                                }
                            } catch (exception::IException &cause) {
                                future->logger.severe()
                                        << "Failed asynchronous execution of execution callback: for call "
                                        << future->invocationToString() << cause;
                            }

                        }

                    private:
                        const std::shared_ptr<AbstractInvocationFuture> future;
                        const std::shared_ptr<CALLBACKTYPE> callback;
                    };

                    void unblock(const std::shared_ptr<CALLBACKTYPE> &callback,
                                 const std::shared_ptr<Executor> &executor) {
                        try {
                            executor->execute(std::shared_ptr<util::Runnable>(
                                    new CallbackRunner(this->shared_from_this(), callback)));
                        } catch (RejectedExecutionException &e) {
                            callback->onFailure(std::shared_ptr<exception::IException>(e.clone()));
                        }
                    }

                    void unblockAll(const std::shared_ptr<BaseState> &waiter,
                                    const std::shared_ptr<Executor> &executor) {
                        while (waiter.get() != NULL) {
                            switch (waiter->getType()) {
                                case BaseState::Thread: {
                                    std::shared_ptr<ThreadState> threadState = std::static_pointer_cast<ThreadState>(
                                            waiter);
                                    threadState->unpark();
                                    return;
                                }
                                case BaseState::ExecutionCallback: {
                                    unblock(std::static_pointer_cast<ExecutionCallbackState>(waiter)->getCallback(),
                                            executor);
                                    return;
                                }
                                case BaseState::WaitNode: {
                                    std::shared_ptr<WaitNode> waitNode = std::static_pointer_cast<WaitNode>(waiter);
                                    unblockAll(waitNode->getWaiter(), waitNode->getExecutor());
                                }
                                default:
                                    return;
                            }
                        }
                    }

                    bool innerComplete(const std::shared_ptr<BaseState> &newState) {
                        for (;;) {
                            const std::shared_ptr<BaseState> oldState = state;
                            if (isDone(oldState)) {
                                warnIfSuspiciousDoubleCompletion(oldState, newState);
                                return false;
                            }
                            if (compareAndSetState(oldState, newState)) {
                                onComplete();
                                unblockAll(oldState, defaultExecutor);
                                return true;
                            }
                        }
                    }

                    // it can be that this future is already completed, e.g. when an invocation already
                    // received a response, but before it cleans up itself, it receives a HazelcastInstanceNotActiveException
                    void warnIfSuspiciousDoubleCompletion(const std::shared_ptr<BaseState> &s0,
                                                          const std::shared_ptr<BaseState> &s1) {
                        if (s0 != s1 && !(s0->getType() == BaseState::Exception &&
                                          std::static_pointer_cast<ExceptionState>(
                                                  s0)->getException()->getErrorCode() == protocol::CANCELLATION) &&
                            !(s1->getType() == BaseState::Exception &&
                              std::static_pointer_cast<ExceptionState>(s1)->getException()->getErrorCode() ==
                              protocol::CANCELLATION)) {
                            logger.warning() << "Future.complete(Object) on completed future. Request: "
                                             << invocationToString();// << ", current value: " << *s0 << ", offered value: " << *s1;
                        }
                    }

                    /**
                     * This field contain the state of the future. If the future is not
                     * complete, the state can be:
                     * <ol>
                     * <li>{@link #VOID}: no response is available.</li>
                     * <li>Thread instance: no response is available and a thread has
                     * blocked on completion (e.g. future.get)</li>
                     * <li>{@link ExecutionCallback} instance: no response is available
                     * and 1 {@link #andThen(ExecutionCallback)} was done using the default
                     * executor</li>
                     * <li>{@link WaitNode} instance: in case of multiple andThen
                     * registrations or future.gets or andThen with custom Executor. </li>
                     * </ol>
                     * If the state is anything else, it is completed.
                     * <p>
                     * The reason why a single future.get or registered ExecutionCallback
                     * doesn't create a WaitNode is that we don't want to cause additional
                     * litter since most of our API calls are a get or a single ExecutionCallback.
                     * <p>
                     * The state field is replaced using a cas, so registration or setting a
                     * response is an atomic operation and therefore not prone to data-races.
                     * There is no need to use synchronized blocks.
                     */
                    util::Sync<std::shared_ptr<BaseState> > state;

                };

                template<typename T>
                const std::shared_ptr<typename AbstractInvocationFuture<T>::BaseState> AbstractInvocationFuture<T>::VOIDOBJECT(
                        new typename AbstractInvocationFuture<T>::VoidState());
            }
        }
    }
}

#endif //HAZELCAST_CLIENT_SPI_IMPL_ABSTRACTINVOCATIONFUTURE_H_
