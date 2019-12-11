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
#ifndef HAZELCAST_CLIENT_ICOMPLETABLEFUTURE_H_
#define HAZELCAST_CLIENT_ICOMPLETABLEFUTURE_H_

#include <boost/shared_ptr.hpp>

#include "hazelcast/client/IFuture.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/util/Executor.h"

using namespace hazelcast::client;
using namespace hazelcast::util;

namespace hazelcast {
    namespace client {

        /**
         * A Future where one can asynchronously listen on completion. This functionality is needed for the
         * reactive programming model.
         *
         * @param <V> The result type returned by this Future's {@code get} method
         */
        template<typename V>
        class ICompletableFuture : public IFuture<V> {
        public:
            /**
             * Registers a callback that will run after this future is completed. If
             * this future is already completed, it runs immediately.
             *
             * <p>Please note that there is no ordering guarantee for running multiple
             * callbacks. It is also not guaranteed that the callback will run within
             * the same thread that completes the future.
             *
             * @param callback the callback to execute
             */
            virtual void andThen(const boost::shared_ptr<ExecutionCallback<V> > &callback) = 0;

            /**
             * Registers a callback that will run with the provided executor after this
             * future is completed. If this future is already completed, it runs
             * immediately. The callback is called using the given {@code executor}.
             *
             * Please note that there is no ordering guarantee for executing multiple
             * callbacks.
             *
             * @param callback the callback to execute
             * @param executor the executor in which the callback will be run
             */
            virtual void andThen(const boost::shared_ptr<ExecutionCallback<V> > &callback,
                                 const boost::shared_ptr<Executor> &executor) = 0;
        };
    }
}

#endif /* HAZELCAST_CLIENT_ICOMPLETABLEFUTURE_H_ */
