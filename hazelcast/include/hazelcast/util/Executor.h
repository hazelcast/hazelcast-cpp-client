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

#ifndef HAZELCAST_UTIL_EXECUTOR_H_
#define HAZELCAST_UTIL_EXECUTOR_H_

#include <memory>

#include "hazelcast/util/Runnable.h"

namespace hazelcast {
    namespace util {
        class HAZELCAST_API Executor {
        public:
            virtual ~Executor();

            virtual void execute(const std::shared_ptr<Runnable> &command) = 0;
        };

        class HAZELCAST_API ExecutorService : public Executor {
        public:
            /**
             * Does the initial active start action (e.g. if needed starts the threads)
             */
            virtual void start() = 0;

            /**
             * Initiates an orderly shutdown in which previously submitted
             * tasks are executed, but no new tasks will be accepted.
             * Invocation has no additional effect if already shut down.
             *
             * <p>This method does not wait for previously submitted tasks to
             * complete execution.  Use {@link #awaitTermination awaitTermination}
             * to do that.
             *
             */
            virtual void shutdown() = 0;

            /**
             * Blocks until all tasks have completed execution after a shutdown
             * request, or the timeout occurs, or the current thread is
             * interrupted, whichever happens first.
             *
             * @param timeoutSeconds the maximum time to wait
             * @return {@code true} if this executor terminated and
             *         {@code false} if the timeout elapsed before termination
             * @throws InterruptedException if interrupted while waiting
             */
            virtual bool awaitTerminationSeconds(int timeoutSeconds) = 0;
        };

        /**
         * An {@link ExecutorService} that can schedule commands to run after a given
         * delay, or to execute periodically.
         *
         * <p>The {@code schedule} methods create tasks with various delays
         * and return a task object that can be used to cancel or check
         * execution. The {@code scheduleAtFixedRate} and
         * {@code scheduleWithFixedDelay} methods create and execute tasks
         * that run periodically until cancelled.
         *
         * <p>Commands submitted using the {@link Executor#execute(Runnable)}
         * and {@link ExecutorService} {@code submit} methods are scheduled
         * with a requested delay of zero. Zero and negative delays (but not
         * periods) are also allowed in {@code schedule} methods, and are
         * treated as requests for immediate execution.
         *
         *
         * <p>The {@link Executors} class provides convenient factory methods for
         * the ScheduledExecutorService implementations provided in this package.
         *
         */
        class HAZELCAST_API ScheduledExecutorService : public ExecutorService {
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
            virtual void schedule(const std::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis) = 0;

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
            virtual void scheduleAtFixedRate(const std::shared_ptr<util::Runnable> &command,
                                             int64_t initialDelayInMillis, int64_t periodInMillis) = 0;

        };
    }
}


#endif //HAZELCAST_UTIL_EXECUTOR_H_
