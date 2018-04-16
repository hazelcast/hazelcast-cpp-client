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

#ifndef HAZELCAST_UTIL_IMPL_SIMPLEEXECUTOR_H_
#define HAZELCAST_UTIL_IMPL_SIMPLEEXECUTOR_H_

#include <stdint.h>
#include <vector>
#include <string>

#include "hazelcast/util/Thread.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"
#include "hazelcast/util/Atomic.h"
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

                void execute(const boost::shared_ptr<Runnable> &command);

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
                boost::shared_ptr<Future<T> > submit(const boost::shared_ptr<Callable<T> > &task) {
                    boost::shared_ptr<CallableRunnableAdaptor<T> > runnable(new CallableRunnableAdaptor<T>(task));
                    execute(runnable);
                    return runnable->getFuture();
                }

                virtual void schedule(const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis);

                virtual void
                scheduleAtFixedRate(const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis,
                                    int64_t periodInMillis);

                /**
                 * Shuts down this Executor.
                 * <p>
                 * No checking is done to see if the Executor already is shut down, so it should be called only once.
                 * <p>
                 * If there is any pending work, it will be thrown away.
                 */
                void shutdown();

            protected:
                template<typename T>
                class CallableRunnableAdaptor : public Runnable {
                public:
                    CallableRunnableAdaptor(const boost::shared_ptr<Callable<T> > &callable) : callable(callable),
                                                                                               future(new Future<T>()) {
                    }

                    virtual ~CallableRunnableAdaptor() {
                    }

                    virtual void run() {
                        try {
                            T result = callable->call();
                            future->set_value(result);
                        } catch (client::exception::IException &e) {
                            future->set_exception(e.clone());
                        }
                    }

                    virtual const std::string getName() const {
                        return callable->getName();
                    }

                    const boost::shared_ptr<Callable<T> > &getCallable() const {
                        return callable;
                    }

                    const boost::shared_ptr<Future<T> > &getFuture() const {
                        return future;
                    }

                private:
                    boost::shared_ptr<Callable<T> > callable;
                    boost::shared_ptr<Future<T> > future;
                };

                class Worker : public util::Runnable {
                public:
                    Worker(SimpleExecutorService &executorService, int32_t maximumQueueCapacity);

                    virtual ~Worker();

                    virtual void run();

                    virtual const std::string getName() const;

                    void schedule(const boost::shared_ptr<Runnable> &runnable);

                    void start();

                    void shutdown();
                private:
                    std::string generateThreadName(const std::string &prefix);

                    SimpleExecutorService &executorService;
                    std::string name;
                    // TODO: Should it be non blocking rejecting queue when compared to Java?
                    util::BlockingConcurrentQueue<boost::shared_ptr<Runnable> > workQueue;
                    util::Thread thread;
                };

                class DelayedRunner : public util::Runnable {
                public:
                    DelayedRunner(const boost::shared_ptr<Runnable> &command, int64_t initialDelayInMillis);

                    DelayedRunner(const boost::shared_ptr<Runnable> &command, int64_t initialDelayInMillis,
                                   int64_t periodInMillis);

                    virtual void run();

                    void shutdown();

                    void setRunnerThread(const boost::shared_ptr<util::Thread> &thread);

                    const boost::shared_ptr<util::Thread> &getRunnerThread() const;

                    virtual const std::string getName() const;

                protected:
                    const boost::shared_ptr<util::Runnable> command;
                    int64_t initialDelayInMillis;
                    int64_t periodInMillis;
                    util::AtomicBoolean live;
                    int64_t startTimeMillis;
                    boost::shared_ptr<util::Thread> runnerThread;
                };

                class RepeatingRunner : public DelayedRunner {
                public:
                    RepeatingRunner(const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis,
                                    int64_t periodInMillis);

                    virtual const std::string getName() const;
                };

                util::ILogger &logger;
                const std::string &threadNamePrefix;
                int threadCount;
                util::AtomicBoolean live;
                util::Atomic<int64_t> threadIdGenerator;
                std::vector<boost::shared_ptr<Worker> > workers;
                int32_t maximumQueueCapacity;
                util::SynchronizedQueue<DelayedRunner> delayedRunners;

                virtual boost::shared_ptr<Worker> getWorker(const boost::shared_ptr<Runnable> &runnable);

                void startWorkers() ;
            };

        }

        class HAZELCAST_API Executors {
        public:
            static boost::shared_ptr<ExecutorService> newSingleThreadExecutor(const std::string &name);
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_IMPL_SIMPLEEXECUTOR_H_
