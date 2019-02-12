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
#ifndef HAZELCAST_CLIENT_INTERNAL_EXECUTOR_COMPLETEDFUTURE_H_
#define HAZELCAST_CLIENT_INTERNAL_EXECUTOR_COMPLETEDFUTURE_H_

#include "hazelcast/client/spi/InternalCompletableFuture.h"
#include "hazelcast/util/Runnable.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace executor {
                template<typename V>
                class CompletedFuture : public spi::InternalCompletableFuture<V> {
                public:
                    CompletedFuture(const boost::shared_ptr<V> &value,
                                    const boost::shared_ptr<Executor> &userExecutor) : value(value),
                                                                                       userExecutor(userExecutor) {}

                    CompletedFuture(const boost::shared_ptr<exception::IException> &exception,
                                    const boost::shared_ptr<Executor> &userExecutor) : exception(exception),
                                                                                       userExecutor(userExecutor) {}

                    virtual void andThen(const boost::shared_ptr<ExecutionCallback<V> > &callback) {
                        andThen(callback, userExecutor);
                    }

                    virtual void andThen(const boost::shared_ptr<ExecutionCallback<V> > &callback,
                                         const boost::shared_ptr<Executor> &executor) {
                        executor->execute(boost::shared_ptr<hazelcast::util::Runnable>(
                                new CallbackExecutor(callback, value, exception)));
                    }

                    virtual bool cancel(bool mayInterruptIfRunning) {
                        return false;
                    }

                    virtual bool isCancelled() {
                        return false;
                    }

                    virtual bool isDone() {
                        return true;
                    }

                    virtual boost::shared_ptr<V> get() {
                        if (exception.get()) {
                            if (exception->getErrorCode() == protocol::EXECUTION) {
                                exception->raise();
                            }
                            throw exception::ExecutionException("CompletedFuture::get",
                                                                "Future returned exception.", exception);
                        }

                        return value;
                    }

                    virtual boost::shared_ptr<V> get(int64_t timeout, const TimeUnit &unit) {
                        return get();
                    }

                    virtual boost::shared_ptr<V> join() {
                        try {
                            // this method is quite inefficient when there is unchecked exception, because it will be wrapped
                            // in a ExecutionException, and then it is unwrapped again.
                            return get();
                        } catch (exception::IException &e) {
                            util::ExceptionUtil::rethrow(e);
                        }
                        return boost::shared_ptr<V>();
                    }

                    virtual bool complete(const boost::shared_ptr<V> &value) {
                        return false;
                    }

                    virtual bool complete(const boost::shared_ptr<exception::IException> &value) {
                        return false;
                    }

                private:
                    class CallbackExecutor : public hazelcast::util::Runnable {
                    public:
                        CallbackExecutor(const boost::shared_ptr<ExecutionCallback<V> > &callback,
                                         const boost::shared_ptr<V> &value,
                                         const boost::shared_ptr<exception::IException> &exception) : callback(
                                callback), value(value), exception(exception) {}

                        virtual const std::string getName() const {
                            return "CompletedFuture Callback Executor";
                        }

                        virtual void run() {
                            if (exception.get()) {
                                callback->onFailure(exception);
                            } else {
                                callback->onResponse(value);
                            }
                        }

                    private:
                        const boost::shared_ptr<ExecutionCallback<V> > callback;
                        const boost::shared_ptr<V> value;
                        const boost::shared_ptr<exception::IException> exception;
                    };

                    const boost::shared_ptr<V> value;
                    const boost::shared_ptr<exception::IException> exception;
                    const boost::shared_ptr<hazelcast::util::Executor> userExecutor;
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_INTERNAL_EXECUTOR_COMPLETEDFUTURE_H_ */

