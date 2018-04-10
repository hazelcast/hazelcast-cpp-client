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
//
// Created by sancar koyunlu on 31/03/14.
//

#ifndef HAZELCAST_UTIL_FUTURE_H_
#define HAZELCAST_UTIL_FUTURE_H_

#include <memory>
#include <vector>
#include <cassert>
#include <boost/shared_ptr.hpp>
#include <ostream>

#include "hazelcast/client/exception/IException.h"
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/LockGuard.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/Executor.h"
#include "hazelcast/util/concurrent/Cancellable.h"
#include "hazelcast/util/concurrent/CancellationException.h"
#include "hazelcast/client/impl/ExecutionCallback.h"
#include "hazelcast/client/exception/IException.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        template<typename T>
        class Future {
        public:
            class SuccessCallbackRunner : public Runnable {
            public:
                SuccessCallbackRunner(T &sharedObj,
                                      const boost::shared_ptr<client::impl::ExecutionCallback<T> > &callback,
                                      ILogger &logger, Future &future) : sharedObj(sharedObj), callback(callback),
                                                                         logger(logger), future(future) {}

                virtual ~SuccessCallbackRunner() {
                }

                virtual void run() {
                    try {
                        callback->onResponse(sharedObj);
                    } catch (client::exception::IException &cause) {
                        logger.severe() << "Failed asynchronous execution of execution success callback: " << callback
                                        << "for call " << future.invocationToString() << ", Cause:" << cause;
                    }
                }

                virtual const std::string getName() const {
                    return "SuccessCallbackRunner";
                }

            private:
                T sharedObj;
                const boost::shared_ptr<client::impl::ExecutionCallback<T> > callback;
                util::ILogger &logger;
                Future<T> &future;
            };

            class ExceptionCallbackRunner : public Runnable {
            public:
                ExceptionCallbackRunner(const boost::shared_ptr<client::exception::IException> &exception,
                                        const boost::shared_ptr<client::impl::ExecutionCallback<T> > &callback,
                                        ILogger &logger, Future &future)
                        : exception(exception), callback(callback), logger(logger), future(future) {}

                virtual ~ExceptionCallbackRunner() {
                }

                virtual void run() {
                    try {
                        callback->onFailure(boost::shared_ptr<client::exception::IException>(exception->clone()));
                    } catch (client::exception::IException &cause) {
                        logger.severe() << "Failed asynchronous execution of execution failure callback: " << callback
                                        << "for call " << future << ", Cause:" << cause;
                    }
                }

                virtual const std::string getName() const {
                    return "ExceptionCallbackRunner";
                }

            private:
                boost::shared_ptr<client::exception::IException> exception;
                const boost::shared_ptr<client::impl::ExecutionCallback<T> > callback;
                util::ILogger &logger;
                Future<T> &future;
            };

            Future(ILogger &logger) : resultReady(false), exceptionReady(false), cancelled(false), logger(logger) {}

            Future() : resultReady(false), exceptionReady(false), cancelled(false), logger(util::ILogger::getLogger()) {
            }

            virtual ~Future() {
            }

            void set_value(const T &value) {
                LockGuard guard(mutex);
                if (cancelled) {
                    return;
                }

                if (exceptionReady || resultReady) {
                    logger.warning(std::string("Future.set_value should not be called twice"));
                    return;
                }
                sharedObject = value;
                resultReady = true;
                conditionVariable.notify_all();
                for (typename std::vector<CallbackInfo>::const_iterator it = callbacks.begin();
                     it != callbacks.end(); ++it) {
                    (*it).getExecutor().execute(
                            boost::shared_ptr<Runnable>(
                                    new SuccessCallbackRunner(sharedObject, (*it).getCallback(), logger, *this)));
                }
                onComplete();
            }

            void set_exception(std::auto_ptr<client::exception::IException> exception) {
                LockGuard guard(mutex);
                if (cancelled) {
                    return;
                }

                if (exceptionReady || resultReady) {
                    logger.warning(std::string("Future.set_exception should not be called twice : details ") +
                                   exception->what());
                    return;
                }

                this->exception = exception;
                exceptionReady = true;
                conditionVariable.notify_all();
                for (typename std::vector<CallbackInfo>::const_iterator it = callbacks.begin();
                     it != callbacks.end(); ++it) {
                    (*it).getExecutor().execute(boost::shared_ptr<Runnable>(
                            new ExceptionCallbackRunner(this->exception, (*it).getCallback(), logger, *this)));
                }

                onComplete();
            }

            void complete(const T &value) {
                set_value(value);
            }

            void complete(const client::exception::IException &exception) {
                set_exception(exception.clone());
            }

            /**
             * Waits if necessary for the computation to complete, and then
             * retrieves its result.
             *
             * @return the computed result
             * @throws CancellationException if the computation was cancelled
             * @throws ExecutionException if the computation threw an
             * exception
             * @throws InterruptedException if the current thread was interrupted
             * while waiting
             */
            T get() {
                LockGuard guard(mutex);

                if (resultReady) {
                    return sharedObject;
                }
                if (exceptionReady) {
                    exception->raise();
                }

                // wait for condition variable to be notified
                while (!resultReady && !exceptionReady) {
                    conditionVariable.wait(mutex);
                }

                if (resultReady) {
                    return sharedObject;
                }
                if (exceptionReady) {
                    exception->raise();
                }
                assert(false && "InvalidState");
                return sharedObject;
            }

            /**
             *
             * @return true if result or exception is ready. git pufalse otherwise when timeout expires.
             *
             * Does not throw
             */
            bool waitFor(int64_t timeInMilliseconds) {
                int64_t endTime = util::currentTimeMillis() + timeInMilliseconds;

                LockGuard guard(mutex);
                while (!(resultReady || exceptionReady) && endTime > util::currentTimeMillis()) {
                    conditionVariable.waitFor(mutex, endTime - util::currentTimeMillis());
                }

                return resultReady || exceptionReady;
            }

            void reset() {
                LockGuard guard(mutex);

                resultReady = false;
                exceptionReady = false;
            }

            void
            andThen(const boost::shared_ptr<client::impl::ExecutionCallback<T> > &callback, util::Executor &executor) {
                LockGuard guard(mutex);
                if (resultReady) {
                    executor.execute(boost::shared_ptr<Runnable>(
                            new SuccessCallbackRunner(sharedObject, callback, logger, *this)));
                    return;
                }

                if (exceptionReady) {
                    executor.execute(boost::shared_ptr<Runnable>(
                            new ExceptionCallbackRunner(exception, callback, logger, *this)));
                    return;
                }

                callbacks.push_back(CallbackInfo(callback, &executor));
            }

            virtual void onComplete() {}

            virtual std::string invocationToString() {
                return "";
            }

            /**
             * Attempts to cancel execution of this task.  This attempt will
             * fail if the task has already completed, has already been cancelled,
             * or could not be cancelled for some other reason.
             *
             * <p>After this method returns, subsequent calls to {@link #isDone} will
             * always return {@code true}.  Subsequent calls to {@link #isCancelled}
             * will always return {@code true} if this method returned {@code true}.
             *
             * @return {@code false} if the task could not be cancelled,
             * typically because it has already completed normally;
             * {@code true} otherwise
             */
            bool cancel() {
                LockGuard guard(mutex);
                if (resultReady || exceptionReady) {
                    return false;
                }

                if (cancelled) {
                    return true;
                }

                cancelled = true;

                exception = boost::shared_ptr<client::exception::IException>(
                        new concurrent::CancellationException("Future::cancel", "Future is cancelled."));

                exceptionReady = true;

                return true;
            }

            bool isCancelled() {
                LockGuard guard(mutex);
                return cancelled;
            }

            /**
             * Returns {@code true} if this task completed.
             *
             * Completion may be due to normal termination, an exception, or
             * cancellation -- in all of these cases, this method will return
             * {@code true}.
             *
             * @return {@code true} if this task completed
             */
            bool isDone() {
                LockGuard guard(mutex);
                return resultReady || exceptionReady || cancelled;
            }

            friend std::ostream &operator<<(std::ostream &os, const Future &future) {
                os << "resultReady: " << future.resultReady << " exceptionReady: " << future.exceptionReady;
                return os;
            }

        protected:
            class CallbackInfo {
            public:
                CallbackInfo(const boost::shared_ptr<client::impl::ExecutionCallback<T> > &callback, Executor *executor)
                        : callback(callback), executor(executor) {}

                const boost::shared_ptr<client::impl::ExecutionCallback<T> > &getCallback() const {
                    return callback;
                }

                Executor &getExecutor() const {
                    return *executor;
                }

            private:
                boost::shared_ptr<client::impl::ExecutionCallback<T> > callback;
                Executor *executor;
            };

            bool resultReady;
            bool exceptionReady;
            bool cancelled;
            ConditionVariable conditionVariable;
            Mutex mutex;
            T sharedObject;
            boost::shared_ptr<client::exception::IException> exception;
            std::vector<CallbackInfo> callbacks;
            util::ILogger &logger;

            Future(const Future &rhs);

            void operator=(const Future &rhs);
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_FUTURE_H_

