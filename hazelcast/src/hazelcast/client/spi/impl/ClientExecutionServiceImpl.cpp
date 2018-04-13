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

#include "hazelcast/util/IOUtil.h"
#include <boost/foreach.hpp>
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Runnable.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                ClientExecutionServiceImpl::ClientExecutionServiceImpl(const std::string &name,
                                                                       const ClientProperties &clientProperties,
                                                                       int32_t poolSize)
                        : logger(util::ILogger::getLogger()) {

                    int internalPoolSize = clientProperties.getInternalExecutorPoolSize().getInteger();
                    if (internalPoolSize <= 0) {
                        internalPoolSize = util::IOUtil::to_value<int>(
                                ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT);
                    }

                    int executorPoolSize = poolSize;
                    if (executorPoolSize <= 0) {
                        executorPoolSize = util::getAvailableCoreCount();
                    }

                    internalExecutor.reset(
                            new util::impl::SimpleExecutorService(logger, name + ".internal-", internalPoolSize,
                                                                  INT32_MAX));
                 }

                void ClientExecutionServiceImpl::execute(const boost::shared_ptr<util::Runnable> &command) {
                    internalExecutor->execute(command);
                }

                void ClientExecutionServiceImpl::shutdown() {
                    shutdownExecutor("internal", *internalExecutor, logger);

                    boost::shared_ptr<AbstractRunner> runner;
                    while ((runner = delayedRunners.poll()).get()) {
                        runner->shutdown();
                        const boost::shared_ptr<util::Thread> &runnerThread = runner->getRunnerThread();
                        runnerThread->cancel();
                        runnerThread->join();
                    }
                }

                void
                ClientExecutionServiceImpl::shutdownExecutor(const std::string &name, util::ExecutorService &executor,
                                                             util::ILogger &logger) {
                    executor.shutdown();
                    // TODO: implement await
/*
                    try {
                        bool success = executor.awaitTermination(TERMINATE_TIMEOUT_SECONDS, TimeUnit.SECONDS);
                        if (!success) {
                            logger.warning(name + " executor awaitTermination could not complete in " + TERMINATE_TIMEOUT_SECONDS
                                           + " seconds");
                        }
                    } catch (InterruptedException e) {
                        logger.warning(name + " executor await termination is interrupted", e);
                    }
*/

                }

                void
                ClientExecutionServiceImpl::scheduleWithRepetition(const boost::shared_ptr<util::Runnable> &command,
                                                                   int64_t initialDelayInMillis,
                                                                   int64_t periodInMillis) {
                    if (!command.get()) {
                        throw exception::NullPointerException("ClientExecutionServiceImpl::scheduleWithRepetition",
                                                              "command can't be null");
                    }
                    boost::shared_ptr<RepeatingRunner> repeatingRunner(
                            new RepeatingRunner(command, initialDelayInMillis, periodInMillis));
                    boost::shared_ptr<util::Thread> thread(new util::Thread(repeatingRunner));
                    repeatingRunner->setRunnerThread(thread);
                    thread->start();
                    delayedRunners.offer(repeatingRunner);
                }

                void ClientExecutionServiceImpl::schedule(const boost::shared_ptr<util::Runnable> &command,
                                                          int64_t initialDelayInMillis) {
                    if (!command.get()) {
                        throw exception::NullPointerException("ClientExecutionServiceImpl::scheduleWithRepetition",
                                                              "command can't be null");
                    }

                    boost::shared_ptr<DelayedRunner> delayedRunner(new DelayedRunner(command, initialDelayInMillis));
                    boost::shared_ptr<util::Thread> thread(new util::Thread(delayedRunner));
                    delayedRunner->setRunnerThread(thread);
                    thread->start();
                    delayedRunners.offer(delayedRunner);
                }

                ClientExecutionServiceImpl::RepeatingRunner::RepeatingRunner(
                        const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis,
                        int64_t periodInMillis) : AbstractRunner(command, initialDelayInMillis, periodInMillis) {
                }

                const std::string ClientExecutionServiceImpl::RepeatingRunner::getName() const {
                    return command->getName();
                }

                ClientExecutionServiceImpl::AbstractRunner::AbstractRunner(
                        const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis) : command(
                        command), initialDelayInMillis(initialDelayInMillis), periodInMillis(-1), live(true) {
                }

                ClientExecutionServiceImpl::AbstractRunner::AbstractRunner(
                        const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis,
                        int64_t periodInMillis) : command(command), initialDelayInMillis(initialDelayInMillis),
                                                  periodInMillis(periodInMillis), live(true) {}

                void ClientExecutionServiceImpl::AbstractRunner::shutdown() {
                    live = false;
                }

                void ClientExecutionServiceImpl::AbstractRunner::setRunnerThread(
                        const boost::shared_ptr<util::Thread> &thread) {
                    runnerThread = thread;
                }

                const boost::shared_ptr<util::Thread> &
                ClientExecutionServiceImpl::AbstractRunner::getRunnerThread() const {
                    return runnerThread;
                }

                void ClientExecutionServiceImpl::AbstractRunner::run() {
                    startTimeMillis = util::currentTimeMillis() + initialDelayInMillis;
                    while (live) {
                        int64_t waitTimeMillis = startTimeMillis - util::currentTimeMillis();
                        if (waitTimeMillis > 0) {
                            if (runnerThread.get()) {
                                runnerThread->interruptibleSleepMillis(waitTimeMillis);
                            } else {
                                util::sleepmillis(waitTimeMillis);
                            }
                        }

                        if (!live) {
                            return;
                        }

                        try {
                            command->run();
                        } catch (exception::IException &e) {
                            util::ILogger::getLogger().warning() << "Repeated runnable " << getName()
                                                                 << " run method caused exception:" << e;
                        }

                        if (periodInMillis < 0) {
                            return;
                        }

                        startTimeMillis += periodInMillis;
                    }
                }

                const std::string ClientExecutionServiceImpl::DelayedRunner::getName() const {
                    return command->getName();
                }

                ClientExecutionServiceImpl::DelayedRunner::DelayedRunner(
                        const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis)
                        : AbstractRunner(command, initialDelayInMillis) {

                }
            }
        }
    }
}
