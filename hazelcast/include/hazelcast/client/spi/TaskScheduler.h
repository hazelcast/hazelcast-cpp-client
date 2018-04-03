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

#ifndef HAZELCAST_CLIENT_SPI_TASKSCHEDULER_H_
#define HAZELCAST_CLIENT_SPI_TASKSCHEDULER_H_

#include <stdint.h>
#include <boost/shared_ptr.hpp>

#include "hazelcast/util/Executor.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            /**
             * {@link ExecutorService} can schedule a command to run after a given delay or execute periodically.
             *
             * The {@link #scheduleWithRepetition(Runnable, long, long)} has similar semantic
             * to {@link java.util.concurrent.ScheduledExecutorService#scheduleAtFixedRate(Runnable, long, long, TimeUnit)}. It
             * guarantees a task won't be executed by multiple threads concurrently. The difference is that this service will
             * skip a scheduled execution if another thread is still running the same task, instead of postponing its execution.
             * To emphasize this difference the method is called <code>scheduleAtFixedRate</code>
             * instead of <code>scheduleAtFixedRate</code>
             *
             * The other difference is this service does not offer an equivalent of
             * {@link java.util.concurrent.ScheduledExecutorService#scheduleWithFixedDelay(Runnable, long, long, TimeUnit)}
             *
             */
            class HAZELCAST_API TaskScheduler : public util::Executor {
            public:
                /**
                 * Creates and executes a one-shot action that becomes enabled
                 * after the given delay.
                 *
                 * @param command the task to execute
                 * @param delay the time from now to delay execution
                 * @throws RejectedExecutionException if the task cannot be
                 *         scheduled for execution
                 * @throws NullPointerException if command is null
                 */
                virtual void schedule(const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis) = 0;

                /**
                 * Creates and executes a periodic action that becomes enabled first
                 * after the given initial delay, and subsequently with the given
                 * period. Executions will commence after
                 * {@code initialDelay} then {@code initialDelay+period}, then
                 * {@code initialDelay + 2 * period}, and so on.
                 * If any execution of this task
                 * takes longer than its period, then subsequent execution will be skipped.
                 *
                 * @param command the task to execute
                 * @param initialDelay the time to delay first execution
                 * @param period the period between successive executions
                 * @param unit the time unit of the initialDelay and period parameters
                 * @throws RejectedExecutionException if the task cannot be
                 *         scheduled for execution
                 * @throws NullPointerException if command is null
                 * @throws IllegalArgumentException if period is less than or equal to zero
                 */
                virtual void scheduleWithRepetition(const boost::shared_ptr<util::Runnable> &command,
                                                    int64_t initialDelayInMillis, int64_t periodInMillis) = 0;

            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_TASKSCHEDULER_H_

