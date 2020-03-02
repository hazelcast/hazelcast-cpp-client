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



#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/util/RuntimeAvailableProcessors.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                const int ClientExecutionServiceImpl::SHUTDOWN_CHECK_INTERVAL_SECONDS = 30;

                ClientExecutionServiceImpl::ClientExecutionServiceImpl(const std::string &name,
                                                                       const ClientProperties &clientProperties,
                                                                       int32_t poolSize, util::ILogger &logger)
                        : logger(logger) {

                    int internalPoolSize = clientProperties.getInteger(clientProperties.getInternalExecutorPoolSize());
                    if (internalPoolSize <= 0) {
                        internalPoolSize = util::IOUtil::to_value<int>(
                                ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT);
                    }

                    int32_t executorPoolSize = poolSize;
                    if (executorPoolSize <= 0) {
                        executorPoolSize = util::RuntimeAvailableProcessors::get();
                    }
                    if (executorPoolSize <= 0) {
                        executorPoolSize = 4; // hard coded thread pool count in case we could not get the processor count
                    }

                    internalExecutor.reset(
                            new util::impl::SimpleExecutorService(logger, name + ".internal-", internalPoolSize,
                                                                  INT32_MAX));

                    userExecutor.reset(
                            new util::impl::SimpleExecutorService(logger, name + ".user-", executorPoolSize,
                                                                  INT32_MAX));
                }

                void ClientExecutionServiceImpl::execute(const std::shared_ptr<util::Runnable> &command) {
                    internalExecutor->execute(command);
                }

                void ClientExecutionServiceImpl::start() {
                    userExecutor->start();
                    internalExecutor->start();
                }

                void ClientExecutionServiceImpl::shutdown() {
                    shutdownExecutor("user", *userExecutor, logger);
                    shutdownExecutor("internal", *internalExecutor, logger);
                }

                void
                ClientExecutionServiceImpl::shutdownExecutor(const std::string &name, util::ExecutorService &executor,
                                                             util::ILogger &logger) {
                    try {
                        int64_t startTimeMilliseconds = util::currentTimeMillis();
                        bool success = false;
                        // Wait indefinitely until the threads gracefully shutdown an log the problem periodically.
                        while (!success) {
                            int64_t waitTimeMillis = 100;
                            while (!success && util::currentTimeMillis() - startTimeMilliseconds <
                                               1000 * SHUTDOWN_CHECK_INTERVAL_SECONDS) {
                                executor.shutdown();
                                auto &executorService = static_cast<util::impl::SimpleExecutorService &>(executor);
                                success = executorService.awaitTerminationMilliseconds(waitTimeMillis);
                            }

                            if (!success) {
                                logger.warning() << name << " executor awaitTermination could not be completed in "
                                                 << (util::currentTimeMillis() - startTimeMilliseconds) << " msecs.";
                            }
                        }
                    } catch (exception::InterruptedException &e) {
                        logger.warning() << name << " executor await termination is interrupted. " << e;
                    }
                }

                void
                ClientExecutionServiceImpl::scheduleWithRepetition(const std::shared_ptr<util::Runnable> &command,
                                                                   int64_t initialDelayInMillis,
                                                                   int64_t periodInMillis) {
                    internalExecutor->scheduleAtFixedRate(command, initialDelayInMillis, periodInMillis);
                }

                void ClientExecutionServiceImpl::schedule(const std::shared_ptr<util::Runnable> &command,
                                                          int64_t initialDelayInMillis) {
                    internalExecutor->schedule(command, initialDelayInMillis);
                }

                const std::shared_ptr<util::ExecutorService> ClientExecutionServiceImpl::getUserExecutor() const {
                    return userExecutor;
                }
            }
        }
    }
}
