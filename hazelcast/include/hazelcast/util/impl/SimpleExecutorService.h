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

#ifndef HAZELCAST_UTIL_IMPL_SIMPLEEXECUTOR_H_
#define HAZELCAST_UTIL_IMPL_SIMPLEEXECUTOR_H_

#include <stdint.h>
#include <vector>
#include <string>
#include <atomic>

#include "hazelcast/util/Thread.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"
#include "hazelcast/util/Future.h"
#include "hazelcast/util/Executor.h"
#include "hazelcast/util/Callable.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/util/SynchronizedQueue.h"
#include "hazelcast/util/SynchronizedQueue.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class ILogger;

        namespace impl {
            class HAZELCAST_API SimpleExecutorService : public ScheduledExecutorService {
            public:
                static int32_t DEFAULT_EXECUTOR_QUEUE_CAPACITY;

                SimpleExecutorService(ILogger &logger, const std::string &threadNamePrefix, int32_t threadCount,
                               int32_t maximumQueueCapacity);

                SimpleExecutorService(ILogger &logger, const std::string &threadNamePrefix, int32_t threadCount);

                virtual ~SimpleExecutorService();

                void execute(const std::shared_ptr<Runnable> &command);

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
                 * @throws NullPointerException if the task is null
                 */
                template<typename T>
                std::shared_ptr<Future<T> > submit(const std::shared_ptr<Callable<T> > &task) {
                    std::shared_ptr<CallableRunnableAdaptor<T> > runnable(new CallableRunnableAdaptor<T>(task, logger));
                    execute(runnable);
                    return runnable->getFuture();
                }

                virtual void schedule(const std::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis);

                virtual void
                scheduleAtFixedRate(const std::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis,
                                    int64_t periodInMillis);

                void start() ;

                /**
                 * Shuts down this Executor.
                 * <p>
                 * No checking is done to see if the Executor already is shut down, so it should be called only once.
                 * <p>
                 * If there is any pending work, it will be thrown away.
                 */
                virtual void shutdown();

                virtual bool awaitTerminationSeconds(int timeoutSeconds);

                virtual bool awaitTerminationMilliseconds(int64_t timeoutMilliseconds);

                const std::string &getThreadNamePrefix() const;

            protected:
                template<typename T>
                class CallableRunnableAdaptor : public Runnable {
                public:
                    CallableRunnableAdaptor(const std::shared_ptr<Callable<T> > &callable, util::ILogger &logger)
                            : callable(callable), future(new Future<T>(logger)) {
                    }

                    virtual ~CallableRunnableAdaptor() {
                    }

                    virtual void run() {
                        try {
                            std::shared_ptr<T> result = callable->call();
                            future->set_value(result);
                        } catch (client::exception::IException &e) {
                            future->complete(e);
                        }
                    }

                    virtual const std::string getName() const {
                        return callable->getName();
                    }

                    const std::shared_ptr<Callable<T> > &getCallable() const {
                        return callable;
                    }

                    const std::shared_ptr<Future<T> > &getFuture() const {
                        return future;
                    }

                private:
                    std::shared_ptr<Callable<T> > callable;
                    std::shared_ptr<Future<T> > future;
                };

                class Worker : public util::Runnable {
                public:
                    Worker(SimpleExecutorService &executorService, int32_t maximumQueueCapacity);

                    virtual ~Worker();

                    virtual void run();

                    virtual const std::string getName() const;

                    void schedule(const std::shared_ptr<Runnable> &runnable);

                    void start();

                    void shutdown();

                    Thread &getThread();

                private:
                    std::string generateThreadName(const std::string &prefix);

                    SimpleExecutorService &executorService;
                    std::string name;
                    // TODO: Should it be non blocking rejecting queue when compared to Java?
                    util::BlockingConcurrentQueue<std::shared_ptr<Runnable> > workQueue;
                    util::Thread thread;
                };

                class DelayedRunner : public util::Runnable {
                public:
                    DelayedRunner(const std::string &threadNamePrefix, const std::shared_ptr<Runnable> &command,
                            int64_t initialDelayInMillis, util::ILogger &logger);

                    DelayedRunner(const std::string &threadNamePrefix, const std::shared_ptr<Runnable> &command,
                            int64_t initialDelayInMillis, int64_t periodInMillis, util::ILogger &logger);

                    virtual void run();

                    void shutdown();

                    virtual const std::string getName() const;

                    void setStartTimeMillis(Thread *pThread);

                protected:
                    const std::shared_ptr<util::Runnable> command;
                    int64_t initialDelayInMillis;
                    int64_t periodInMillis;
                    util::AtomicBoolean live;
                    int64_t startTimeMillis;
                    util::Thread *runnerThread;
                    util::ILogger &logger;
                    const std::string threadNamePrefix;
                };

                virtual std::shared_ptr<Worker> getWorker(const std::shared_ptr<Runnable> &runnable);

                util::ILogger &logger;
                const std::string threadNamePrefix;
                int threadCount;
                util::AtomicBoolean live;
                util::AtomicBoolean isStarted;
                std::atomic<int64_t> threadIdGenerator;
                std::vector<std::shared_ptr<Worker> > workers;
                int32_t maximumQueueCapacity;
                util::SynchronizedQueue<util::Thread> delayedRunners;
            };

        }

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_IMPL_SIMPLEEXECUTOR_H_
