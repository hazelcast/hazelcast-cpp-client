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

#include <cmath>
#include <cassert>

#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/util/HashUtil.h"

namespace hazelcast {
    namespace util {
        namespace impl {
            int32_t SimpleExecutorService::DEFAULT_EXECUTOR_QUEUE_CAPACITY = INT32_MAX;

            SimpleExecutorService::SimpleExecutorService(ILogger &logger, const std::string &threadNamePrefix,
                                                         int threadCount,
                                                         int32_t maximumQueueCapacity)
                    : logger(logger), threadNamePrefix(threadNamePrefix), threadCount(threadCount), live(true),
                      threadIdGenerator(0), workers(threadCount), maximumQueueCapacity(maximumQueueCapacity) {
                startWorkers();
            }

            SimpleExecutorService::SimpleExecutorService(ILogger &logger, const std::string &threadNamePrefix,
                                                         int threadCount)
                    : logger(logger), threadNamePrefix(threadNamePrefix), threadCount(threadCount), live(true),
                      threadIdGenerator(0), workers(threadCount),
                      maximumQueueCapacity(DEFAULT_EXECUTOR_QUEUE_CAPACITY) {
                startWorkers();
            }

            void SimpleExecutorService::startWorkers() {
                // `maximumQueueCapacity` is the given max capacity for this executor. Each worker in this executor should consume
                // only a portion of that capacity. Otherwise we will have `threadCount * maximumQueueCapacity` instead of
                // `maximumQueueCapacity`.
                int32_t perThreadMaxQueueCapacity = static_cast<int32_t>(ceil(
                        (double) 1.0 * maximumQueueCapacity / threadCount));
                for (int i = 0; i < threadCount; i++) {
                    workers[i].reset(new Worker(*this, perThreadMaxQueueCapacity));
                    workers[i]->start();
                }

                if (logger.isFinestEnabled()) {
                    logger.finest() << "ExecutorService " << threadNamePrefix << " started " << threadCount << " workers.";
                }
            }

            void SimpleExecutorService::execute(const std::shared_ptr<Runnable> &command) {
                if (command.get() == NULL) {
                    throw client::exception::NullPointerException("SimpleExecutor::execute", "command can't be null");
                }

                if (!live) {
                    throw client::exception::RejectedExecutionException("SimpleExecutor::execute",
                                                                        "Executor is terminated!");
                }

                std::shared_ptr<Worker> worker = getWorker(command);
                worker->schedule(command);
            }

            std::shared_ptr<SimpleExecutorService::Worker>
            SimpleExecutorService::getWorker(const std::shared_ptr<Runnable> &runnable) {
                int32_t key;
                if (runnable->isStriped()) {
                    key = std::static_pointer_cast<StripedRunnable>(runnable)->getKey();
                } else {
                    key = (int32_t) rand();
                }
                int index = HashUtil::hashToIndex(key, threadCount);
                return workers[index];
            }

            void SimpleExecutorService::shutdown() {
                live.store(false);

                size_t numberOfWorkers = workers.size();
                size_t numberOfDelayedRunners = delayedRunners.size();

                if (logger.isFinestEnabled()) {
                    logger.finest() << "ExecutorService " << threadNamePrefix << " has " << numberOfWorkers << " workers and "
                                    << numberOfDelayedRunners << " delayed runners " << " to shutdown.";
                }

                for (std::shared_ptr<Worker> &worker : workers) {
                                worker->shutdown();
                            }

                std::shared_ptr<util::Thread> runner;
                while ((runner = delayedRunners.poll()).get()) {
                    std::static_pointer_cast<DelayedRunner>(runner->getTarget())->shutdown();
                    runner->wakeup();
                }
            }

            bool SimpleExecutorService::awaitTerminationSeconds(int timeoutSeconds) {
                return awaitTerminationMilliseconds(timeoutSeconds * CountDownLatch::MILLISECONDS_IN_A_SECOND);
            }

            bool SimpleExecutorService::awaitTerminationMilliseconds(int64_t timeoutMilliseconds) {
                int64_t endTimeMilliseconds = currentTimeMillis() + timeoutMilliseconds;

                for (std::shared_ptr<Worker> &worker : workers) {
                                int64_t waitMilliseconds = endTimeMilliseconds - currentTimeMillis();

                                if (logger.isFinestEnabled()) {
                                    logger.finest() << "ExecutorService is waiting worker thread " << worker->getName()
                                                    << " for a maximum of " << waitMilliseconds << " msecs.";
                                }

                                if (!worker->getThread().waitMilliseconds(waitMilliseconds)) {
                                    logger.warning() << "ExecutorService could not stop worker thread " << worker->getName()
                                                    << " in " << timeoutMilliseconds << " msecs.";

                                    return false;
                                }
                            }

                for (const std::shared_ptr<Thread> &t : delayedRunners.values()) {
                                int64_t waitMilliseconds = endTimeMilliseconds - currentTimeMillis();

                                if (logger.isFinestEnabled()) {
                                    logger.finest() << "ExecutorService is waiting delayed runner thread " << t->getName()
                                                    << " for a maximum of " << waitMilliseconds << " msecs.";
                                }

                                if (!t->waitMilliseconds(waitMilliseconds)) {
                                    logger.warning() << "ExecutorService could not stop delayed runner thread " << t->getName()
                                                    << " in " << timeoutMilliseconds << " msecs.";

                                    return false;
                                }
                            }

                return true;
            }

            SimpleExecutorService::~SimpleExecutorService() {
            }

            void SimpleExecutorService::schedule(const std::shared_ptr<util::Runnable> &command,
                                                 int64_t initialDelayInMillis) {
                if (command.get() == NULL) {
                    throw client::exception::NullPointerException("SimpleExecutor::schedule", "command can't be null");
                }

                if (!live) {
                    throw client::exception::RejectedExecutionException("SimpleExecutor::schedule",
                                                                        "Executor is terminated!");
                }

                std::shared_ptr<DelayedRunner> delayedRunner(
                        new DelayedRunner(threadNamePrefix, command, initialDelayInMillis, logger));
                std::shared_ptr<util::Thread> thread(new util::Thread(delayedRunner, logger));
                delayedRunner->setStartTimeMillis(thread.get());
                thread->start();
                delayedRunners.offer(thread);
            }

            void SimpleExecutorService::scheduleAtFixedRate(const std::shared_ptr<util::Runnable> &command,
                                                            int64_t initialDelayInMillis, int64_t periodInMillis) {
                if (command.get() == NULL) {
                    throw client::exception::NullPointerException("SimpleExecutor::scheduleAtFixedRate",
                                                                  "command can't be null");
                }

                if (!live) {
                    throw client::exception::RejectedExecutionException("SimpleExecutor::scheduleAtFixedRate",
                                                                        "Executor is terminated!");
                }

                std::shared_ptr<DelayedRunner> repeatingRunner(
                        new DelayedRunner(threadNamePrefix, command, initialDelayInMillis, periodInMillis, logger));
                std::shared_ptr<util::Thread> thread(new util::Thread(repeatingRunner, logger));
                repeatingRunner->setStartTimeMillis(thread.get());
                thread->start();
                delayedRunners.offer(thread);
            }

            void SimpleExecutorService::Worker::run() {
                std::shared_ptr<Runnable> task;
                while (executorService.live) {
                    try {
                        task = workQueue.pop();
                        if (task.get()) {
                            task->run();
                        }
                    } catch (client::exception::InterruptedException &) {
                        if (executorService.logger.isFinestEnabled()) {
                            executorService.logger.finest() << getName() << " is interrupted.";
                        }
                    } catch (client::exception::IException &t) {
                        executorService.logger.warning() << getName() << " caused an exception" << t;
                    }
                }
            }

            SimpleExecutorService::Worker::~Worker() {
            }

            void SimpleExecutorService::Worker::schedule(const std::shared_ptr<Runnable> &runnable) {
                workQueue.push(runnable);
            }

            void SimpleExecutorService::Worker::start() {
                thread.start();
            }

            const std::string SimpleExecutorService::Worker::getName() const {
                return name;
            }

            std::string SimpleExecutorService::Worker::generateThreadName(const std::string &prefix) {
                std::ostringstream out;
                out << prefix << (++executorService.threadIdGenerator);
                return out.str();
            }

            void SimpleExecutorService::Worker::shutdown() {
                workQueue.interrupt();
            }

            SimpleExecutorService::Worker::Worker(SimpleExecutorService &executorService, int32_t maximumQueueCapacity)
                    : executorService(executorService), name(generateThreadName(executorService.threadNamePrefix)),
                    workQueue((size_t) maximumQueueCapacity),
                    thread(std::shared_ptr<util::Runnable>(new util::RunnableDelegator(*this)), executorService.logger) {
            }

            Thread &SimpleExecutorService::Worker::getThread() {
                return thread;
            }

            SimpleExecutorService::DelayedRunner::DelayedRunner(const std::string &threadNamePrefix,
                    const std::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis,
                    util::ILogger &logger) : command(command), initialDelayInMillis(initialDelayInMillis),
                                             periodInMillis(-1), live(true), startTimeMillis(0), runnerThread(NULL),
                                             logger(logger), threadNamePrefix(threadNamePrefix) {
            }

            SimpleExecutorService::DelayedRunner::DelayedRunner(const std::string &threadNamePrefix,
                                                                const std::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis,
                    int64_t periodInMillis, util::ILogger &logger) : command(command), initialDelayInMillis(initialDelayInMillis),
                                              periodInMillis(periodInMillis), live(true), startTimeMillis(0),
                                              runnerThread(NULL), logger(logger), threadNamePrefix(threadNamePrefix) {}

            void SimpleExecutorService::DelayedRunner::shutdown() {
                live.store(false);
            }

            void SimpleExecutorService::DelayedRunner::run() {
                bool isNotRepeating = periodInMillis < 0;
                while (live || isNotRepeating) {
                    if (live) {
                        int64_t waitTimeMillis = startTimeMillis - util::currentTimeMillis();
                        if (waitTimeMillis > 0) {
                            assert(runnerThread != NULL);
                            runnerThread->interruptibleSleepMillis(waitTimeMillis);
                        }
                    }


                    try {
                        command->run();
                    } catch (client::exception::IException &e) {
                        if (isNotRepeating) {
                            logger.warning() << "Runnable " << getName() << " run method caused exception:" << e;
                        } else {
                            logger.warning() << "Repeated runnable " << getName() << " run method caused exception:" << e;
                        }
                    }

                    if (isNotRepeating) {
                        return;
                    }

                    startTimeMillis += periodInMillis;
                }
            }

            const std::string SimpleExecutorService::DelayedRunner::getName() const {
                return threadNamePrefix + command->getName();
            }

            void SimpleExecutorService::DelayedRunner::setStartTimeMillis(Thread *pThread) {
                runnerThread = pThread;
                startTimeMillis = util::currentTimeMillis() + initialDelayInMillis;
            }

        }

        std::shared_ptr<ExecutorService> Executors::newSingleThreadExecutor(const std::string &name,
                util::ILogger &logger) {
            return std::shared_ptr<ExecutorService>(new impl::SimpleExecutorService(logger, name, 1));
        }

        Executor::~Executor() {
        }
    }
}

