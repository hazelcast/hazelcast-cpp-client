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

#ifndef HAZELCAST_Future
#define HAZELCAST_Future

#include <memory>
#include <vector>
#include <cassert>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include "hazelcast/client/exception/IException.h"
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/LockGuard.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/Executor.h"
#include "hazelcast/client/impl/ExecutionCallback.h"

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
                                      const boost::shared_ptr<client::impl::ExecutionCallback<T> > &callback)
                        : sharedObj(sharedObj), callback(callback) {}

                virtual ~SuccessCallbackRunner() {
                }

                virtual void run() {
                    callback->onResponse(&sharedObj);
                }

                virtual const std::string getName() const {
                    return "SuccessCallbackRunner";
                }

            private:
                T &sharedObj;
                const boost::shared_ptr<client::impl::ExecutionCallback<T> > callback;
            };

            class ExceptionCallbackRunner : public Runnable {
            public:
                ExceptionCallbackRunner(const boost::shared_ptr<client::exception::IException> &exception,
                                        const boost::shared_ptr<client::impl::ExecutionCallback<T> > &callback)
                        : exception(exception), callback(callback) {}

                virtual ~ExceptionCallbackRunner() {
                }

                virtual void run() {
                    callback->onFailure(exception.get());
                }

                virtual const std::string getName() const {
                    return "SuccessCallbackRunner";
                }

            private:
                boost::shared_ptr<client::exception::IException> exception;
                const boost::shared_ptr<client::impl::ExecutionCallback<T> > callback;
            };

            Future()
            : resultReady(false)
            , exceptionReady(false) {

            }

            virtual ~Future() {
            }

            void set_value(T& value) {
                LockGuard guard(mutex);
                if (exceptionReady || resultReady) {
                    util::ILogger::getLogger().warning(std::string("Future.set_value should not be called twice"));
                }
                sharedObject = value;
                resultReady = true;
                conditionVariable.notify_all();
                BOOST_FOREACH(CallbackInfo & callbackInfo, callbacks) {
                                callbackInfo.executor.execute(boost::shared_ptr<Runnable>(
                                        new SuccessCallbackRunner(sharedObject, callbackInfo.callback)));
                            }
                onComplete();
            }

            void set_exception(std::auto_ptr<client::exception::IException> exception) {
                LockGuard guard(mutex);

                if (exceptionReady || resultReady) {
                    util::ILogger::getLogger().warning(std::string("Future.set_exception should not be called twice : details ") + exception->what());
                }

                this->exception = exception;
                exceptionReady = true;
                conditionVariable.notify_all();
                BOOST_FOREACH(CallbackInfo & callbackInfo, callbacks) {
                                callbackInfo.executor.execute(boost::shared_ptr<Runnable>(
                                        new ExceptionCallbackRunner(this->exception, callbackInfo.callback)));
                            }
                onComplete();
            }

            void reset_exception(std::auto_ptr<client::exception::IException> exception) {
                LockGuard guard(mutex);

                this->exception = exception;
                exceptionReady = true;
                conditionVariable.notify_all();
                onComplete();
            }

            void complete(T &value) {
                set_value(value);
            }

            void complete(const client::exception::IException &exception) {
                set_exception(exception);
            }

            T get() {
                LockGuard guard(mutex);

                if (resultReady) {
                    return sharedObject;
                }
                if (exceptionReady) {
                    exception->raise();
                }

                // wait for condition variable to be notified
                conditionVariable.wait(mutex);

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
             * @return true if result or exception is ready. false otherwise when timeout expires.
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

            void andThen(const boost::shared_ptr<client::impl::ExecutionCallback<T> > &callback, util::Executor &executor) {
                LockGuard guard(mutex);
                if (resultReady) {
                    executor.execute(boost::shared_ptr<Runnable>(new SuccessCallbackRunner(sharedObject, callback)));
                    return;
                }

                if (exceptionReady) {
                    executor.execute(boost::shared_ptr<Runnable>(new ExceptionCallbackRunner(exception, callback)));
                    return;
                }

                callbacks.push_back(CallbackInfo(callback, executor));
            }

            virtual void onComplete() {}
        protected:
            struct CallbackInfo {
                CallbackInfo(const boost::shared_ptr<client::impl::ExecutionCallback<T> > &callback, Executor &executor)
                        : callback(callback), executor(executor) {}

                const boost::shared_ptr<client::impl::ExecutionCallback<T> > callback;
                util::Executor &executor;
            };
            bool resultReady;
            bool exceptionReady;
            ConditionVariable conditionVariable;
            Mutex mutex;
            T sharedObject;
            boost::shared_ptr<client::exception::IException> exception;
            std::vector<CallbackInfo> callbacks;

            Future(const Future& rhs);

            void operator=(const Future& rhs);
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_Future

