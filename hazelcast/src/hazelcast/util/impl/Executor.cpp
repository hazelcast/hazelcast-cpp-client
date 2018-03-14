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

#include <cmath>
#include <boost/foreach.hpp>

#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/client/exception/IException.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/HashUtil.h"
#include <sstream>

namespace hazelcast {
    namespace util {
        namespace impl {
            int32_t SimpleExecutorService::DEFAULT_EXECUTOR_QUEUE_CAPACITY = 100 * 1000;
            util::Atomic<int64_t> SimpleExecutorService::THREAD_ID_GENERATOR;

            SimpleExecutorService::SimpleExecutorService(ILogger &logger, const std::string &threadNamePrefix,
                                                         int threadCount,
                                                         int32_t maximumQueueCapacity)
                    : logger(logger), threadNamePrefix(threadNamePrefix), threadCount(threadCount), live(true),
                      workers(threadCount), maximumQueueCapacity(maximumQueueCapacity) {
                startWorkers();
            }

            SimpleExecutorService::SimpleExecutorService(ILogger &logger, const std::string &threadNamePrefix,
                                                         int threadCount)
                    : logger(logger), threadNamePrefix(threadNamePrefix), threadCount(threadCount), live(true),
                      workers(threadCount), maximumQueueCapacity(DEFAULT_EXECUTOR_QUEUE_CAPACITY) {
                startWorkers();
            }

            void SimpleExecutorService::startWorkers() {
                // `maximumQueueCapacity` is the given max capacity for this executor. Each worker in this executor should consume
                // only a portion of that capacity. Otherwise we will have `threadCount * maximumQueueCapacity` instead of
                // `maximumQueueCapacity`.
                int32_t perThreadMaxQueueCapacity = static_cast<int32_t>(ceil(
                        (double) 1.0 * maximumQueueCapacity / threadCount));
                for (int i = 0; i < threadCount; i++) {
                    workers[i].reset(new Worker(threadNamePrefix, perThreadMaxQueueCapacity, live, logger));
                    workers[i]->start();
                }
            }

            void SimpleExecutorService::execute(const boost::shared_ptr<Runnable> &command) {
                if (command.get() == NULL) {
                    throw client::exception::IException("SimpleExecutor::execute", "command can't be null");
                }

                if (!live) {
                    throw client::exception::RejectedExecutionException("SimpleExecutor::execute",
                                                                        "Executor is terminated!");
                }

                boost::shared_ptr<Worker> worker = getWorker(command);
                worker->schedule(command);
            }

            boost::shared_ptr<SimpleExecutorService::Worker>
            SimpleExecutorService::getWorker(const boost::shared_ptr<Runnable> &runnable) {
                int32_t key;
                if (runnable->isStriped()) {
                    key = boost::static_pointer_cast<StripedRunnable>(runnable)->getKey();
                } else {
                    key = (int32_t) rand();
                }
                int index = HashUtil::hashToIndex(key, threadCount);
                return workers[index];
            }

            void SimpleExecutorService::shutdown() {
                if (!live.compareAndSet(true, false)) {
                    return;
                }

                BOOST_FOREACH(boost::shared_ptr<Worker> &worker, workers) {
                                worker->workQueue.clear();
                                worker->wakeup();
                                worker->join();
                            }
            }

            SimpleExecutorService::~SimpleExecutorService() {
                shutdown();
            }

            void SimpleExecutorService::Worker::run() {
                boost::shared_ptr<Runnable> task;
                while (live) {
                    try {
                        task = workQueue.pop();
                        if (task.get()) {
                            task->run();
                        }
                    } catch (client::exception::IException &t) {
                        logger.warning() << getName() << " caught an exception" << t;
                    }
                }
            }

            SimpleExecutorService::Worker::Worker(const std::string &threadNamePrefix,
                                                  int32_t queueCapacity,
                                                  util::AtomicBoolean &live, util::ILogger &logger)
                    : Thread(generateThreadName(threadNamePrefix)), workQueue(queueCapacity), live(live),
                      logger(logger) {
            }

            void SimpleExecutorService::Worker::schedule(const boost::shared_ptr<Runnable> &runnable) {
                workQueue.push(runnable);
            }

            std::string SimpleExecutorService::Worker::generateThreadName(const std::string &prefix) {
                std::ostringstream out;
                out << prefix << "-" << (++THREAD_ID_GENERATOR);
                return out.str();
            }
        }

        boost::shared_ptr<ExecutorService> Executors::newSingleThreadExecutor(const std::string &name) {
            return boost::shared_ptr<ExecutorService>(
                    new impl::SimpleExecutorService(util::ILogger::getLogger(), name, 1));
        }

    }
}

