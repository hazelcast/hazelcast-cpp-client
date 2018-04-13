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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_CLIENTEXECUTIONSERVICEIMPL_H_
#define HAZELCAST_CLIENT_SPI_IMPL_CLIENTEXECUTIONSERVICEIMPL_H_

#include <vector>

#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/util/SynchronizedQueue.h"
#include "hazelcast/util/Atomic.h"

#include "hazelcast/client/spi/ClientExecutionService.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class Runnable;

        class Thread;
    }

    namespace client {
        class ClientProperties;

        namespace spi {
            namespace impl {
                class HAZELCAST_API ClientExecutionServiceImpl : public ClientExecutionService {
                public:
                    ClientExecutionServiceImpl(const std::string &name, const ClientProperties &clientProperties,
                                               int32_t poolSize);

                    void execute(const boost::shared_ptr<util::Runnable> &command);

                    void shutdown();

                    static void
                    shutdownExecutor(const std::string &name, util::ExecutorService &executor, util::ILogger &logger);

                    virtual void
                    schedule(const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis);

                    virtual void
                    scheduleWithRepetition(const boost::shared_ptr<util::Runnable> &command,
                                           int64_t initialDelayInMillis,
                                           int64_t periodInMillis);

                private:
                    class AbstractRunner : public util::Runnable {
                    public:
                        AbstractRunner(const boost::shared_ptr<Runnable> &command, int64_t initialDelayInMillis);

                        AbstractRunner(const boost::shared_ptr<Runnable> &command, int64_t initialDelayInMillis,
                                       int64_t periodInMillis);

                        virtual void run();

                        void shutdown();

                        void setRunnerThread(const boost::shared_ptr<util::Thread> &thread);

                        const boost::shared_ptr<util::Thread> &getRunnerThread() const;
                    protected:
                        const boost::shared_ptr<util::Runnable> command;
                        int64_t initialDelayInMillis;
                        int64_t periodInMillis;
                        util::AtomicBoolean live;
                        int64_t startTimeMillis;
                        boost::shared_ptr<util::Thread> runnerThread;
                    };

                    class RepeatingRunner : public AbstractRunner {
                    public:
                        RepeatingRunner(const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis,
                                        int64_t periodInMillis);

                        virtual const std::string getName() const;
                    };

                    class DelayedRunner : public AbstractRunner {
                    public:
                        DelayedRunner(const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis);

                    private:
                        virtual const std::string getName() const;
                    };

                    util::ILogger &logger;
                    // TODO: Change with ScheduledExecutorService
                    boost::shared_ptr<util::ExecutorService> internalExecutor;

                    util::SynchronizedQueue<AbstractRunner> delayedRunners;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_CLIENTEXECUTIONSERVICEIMPL_H_
