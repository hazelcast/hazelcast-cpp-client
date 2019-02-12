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
#ifndef HAZELCAST_CLIENT_IFUTURE_H_
#define HAZELCAST_CLIENT_IFUTURE_H_

#include <boost/shared_ptr.hpp>

#include "hazelcast/client/exception/InterruptedException.h"
#include "hazelcast/util/concurrent/TimeUnit.h"

using namespace hazelcast::client;
using namespace hazelcast::util::concurrent;

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {

        /**
         * A {@code Future} represents the result of an asynchronous
         * computation.  Methods are provided to check if the computation is
         * complete, to wait for its completion, and to retrieve the result of
         * the computation.  The result can only be retrieved using method
         * {@code get} when the computation has completed, blocking if
         * necessary until it is ready.  Cancellation is performed by the
         * {@code cancel} method.  Additional methods are provided to
         * determine if the task completed normally or was cancelled. Once a
         * computation has completed, the computation cannot be cancelled.
         *
         *
         * @param <V> The result type returned by this Future's {@code get} method
         */
        template<typename V>
        class IFuture {
        public:
            virtual ~IFuture() {
            }

            /**
             * Attempts to cancel execution of this task.  This attempt will
             * fail if the task has already completed, has already been cancelled,
             * or could not be cancelled for some other reason. If successful,
             * and this task has not started when {@code cancel} is called,
             * this task should never run.  If the task has already started,
             * then the {@code mayInterruptIfRunning} parameter determines
             * whether the thread executing this task should be interrupted in
             * an attempt to stop the task.
             *
             * <p>After this method returns, subsequent calls to {@link #isDone} will
             * always return {@code true}.  Subsequent calls to {@link #isCancelled}
             * will always return {@code true} if this method returned {@code true}.
             *
             * @param mayInterruptIfRunning {@code true} if the thread executing this
             * task should be interrupted; otherwise, in-progress tasks are allowed
             * to complete
             * @return {@code false} if the task could not be cancelled,
             * typically because it has already completed normally;
             * {@code true} otherwise
             */
            virtual bool cancel(bool mayInterruptIfRunning) = 0;

            /**
             * Returns {@code true} if this task was cancelled before it completed
             * normally.
             *
             * @return {@code true} if this task was cancelled before it completed
             */
            virtual bool isCancelled() = 0;

            /**
             * Returns {@code true} if this task completed.
             *
             * Completion may be due to normal termination, an exception, or
             * cancellation -- in all of these cases, this method will return
             * {@code true}.
             *
             * @return {@code true} if this task completed
             */
            virtual bool isDone() = 0;

            /**
             * Waits if necessary for the computation to complete, and then
             * retrieves its result.
             *
             * @return the computed result
             * @throws CancellationException if the computation was cancelled
             * @throws ExecutionException if the computation threw an
             * exception
             * @throws InterruptedException if the current thread was interrupted
             * while waiting
             */
            virtual boost::shared_ptr<V> get() = 0;

            /**
             * Waits if necessary for at most the given time for the computation
             * to complete, and then retrieves its result, if available.
             *
             * @param timeout the maximum time to wait
             * @param unit the time unit of the timeout argument
             * @return the computed result
             * @throws CancellationException if the computation was cancelled
             * @throws ExecutionException if the computation threw an
             * exception
             * @throws InterruptedException if the current thread was interrupted
             * while waiting
             * @throws TimeoutException if the wait timed out
             */
            virtual boost::shared_ptr<V> get(int64_t timeout,
                                             const TimeUnit &unit) = 0;

        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_IFUTURE_H_ */
