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

#include <boost/foreach.hpp>

#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/util/IOUtil.h"
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

                    internalExecutor.reset(
                            new util::impl::SimpleExecutorService(logger, name + ".internal-", internalPoolSize,
                                                                  INT32_MAX));
                 }

                void ClientExecutionServiceImpl::execute(const boost::shared_ptr<util::Runnable> &command) {
                    internalExecutor->execute(command);
                }

                void ClientExecutionServiceImpl::shutdown() {
                    shutdownExecutor("internal", *internalExecutor, logger);
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
                    internalExecutor->scheduleAtFixedRate(command, initialDelayInMillis, periodInMillis);
                }

                void ClientExecutionServiceImpl::schedule(const boost::shared_ptr<util::Runnable> &command,
                                                          int64_t initialDelayInMillis) {
                    internalExecutor->schedule(command, initialDelayInMillis);
                }
            }
        }
    }
}
