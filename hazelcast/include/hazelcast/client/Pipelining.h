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
#ifndef HAZELCAST_CLIENT_PIPELINING_H_
#define HAZELCAST_CLIENT_PIPELINING_H_

#include <vector>

#include "hazelcast/client/ICompletableFuture.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/concurrent/ConcurrencyUtil.h"

namespace hazelcast {
    namespace client {
        /**
         * @Beta
         *
         * The Pipelining can be used to speed up requests. It is build on top of asynchronous
         * requests like e.g. {@link IMap#getAsync(const K&)} or any other asynchronous call.
         *
         * The main purpose of the Pipelining is to control the number of concurrent requests
         * when using asynchronous invocations. This can be done by setting the depth using
         * the constructor. So you could set the depth to e.g 100 and do 1000 calls. That means
         * that at any given moment, there will only be 100 concurrent requests.
         *
         * It depends on the situation what the optimal depth (number of invocations in
         * flight) should be. If it is too high, you can run into memory related problems.
         * If it is too low, it will provide little or no performance advantage at all. In
         * most cases a Pipelining and a few hundred map/cache puts/gets should not lead to any
         * problems. For testing purposes we frequently have a Pipelining of 1000 or more
         * concurrent requests to be able to saturate the system.
         *
         * The Pipelining can't be used for transaction purposes. So you can't create a
         * Pipelining, add a set of asynchronous request and then not call {@link #results()}
         * to prevent executing these requests. Invocations can be executed before the
         * {@link #results()} is called.
         *
         * Pipelining can be used by both clients or members.
         *
         * The Pipelining isn't threadsafe. So only a single thread should add requests to
         * the Pipelining and wait for results.
         *
         * Currently all {@link ICompletableFuture} and their responses are stored in the
         * Pipelining. So be careful executing a huge number of request with a single Pipelining
         * because it can lead to a huge memory bubble. In this cases it is better to
         * periodically, after waiting for completion, to replace the Pipelining by a new one.
         * In the future we might provide this as an out of the box experience, but currently
         * we do not.
         *
         * A Pipelining provides its own backpressure on the system. So there will not be more
         * in flight invocations than the depth of the Pipelining. This means that the Pipelining
         * will work fine when backpressure on the client/member is disabled (default). Also
         * when it is  enabled it will work fine, but keep in mind that the number of concurrent
         * invocations in the Pipelining could be lower than the configured number of invocation
         * of the Pipelining because the backpressure on the client/member is leading.
         *
         * The Pipelining has been marked as Beta since we need to see how the API needs to
         * evolve. But there is no problem using it in production. We use similar techniques
         * to achieve high performance.
         *
         * @param <E>
         */
        template<typename E>
        class Pipelining : public boost::enable_shared_from_this<Pipelining<E> > {
        public:
            /**
             * Creates a Pipelining with the given depth.
             *
             * We use this factory create method and hide the constructor from the user, because the private
             * {@link up()} and {@link down()} methods of this class may be called from the executor thread, and
             * we need to make sure that the `Pipelining` instance is not destructed when these methods are accessed.
             *
             * @param depth the maximum number of concurrent calls allowed in this Pipelining.
             * @throws IllegalArgumentException if depth smaller than 1. But if you use depth 1, it means that
             *                                  every call is sync and you will not benefit from pipelining at all.
             */
            static boost::shared_ptr<Pipelining> create(int depth) {
                util::Preconditions::checkPositive(depth, "depth must be positive");

                return boost::shared_ptr<Pipelining>(new Pipelining(depth));
            }

            /**
             * Returns the results.
             * <p>
             * The results are returned in the order the requests were done.
             * <p>
             * This call waits till all requests have completed.
             *
             * @return the List of results.
             * @throws IException if something fails getting the results.
             */
            std::vector<boost::shared_ptr<E> > results() {
                std::vector<boost::shared_ptr<E> > result(futures.size());
                size_t index = 0;
                for (typename FuturesVector::const_iterator it = futures.begin(); it != futures.end(); ++it, ++index) {
                    result[index] = (*it)->get();
                }
                return result;
            }

            /**
             * Adds a future to this Pipelining or blocks until there is capacity to add the future to the Pipelining.
             * <p>
             * This call blocks until there is space in the Pipelining, but it doesn't mean that the invocation that
             * returned the ICompletableFuture got blocked.
             *
             * @param future the future to add.
             * @return the future added.
             * @throws NullPointerException if future is null.
             */
            const boost::shared_ptr<ICompletableFuture<E> >
            add(const boost::shared_ptr<ICompletableFuture<E> > &future) {
                util::Preconditions::checkNotNull<ICompletableFuture<E> >(future, "future can't be null");

                down();
                futures.push_back(future);
                future->andThen(boost::shared_ptr<ExecutionCallback<E> >(
                        new PipeliningExecutionCallback(this->shared_from_this())),
                                util::concurrent::ConcurrencyUtil::CALLER_RUNS());
                return future;
            }

        private:
            class PipeliningExecutionCallback : public ExecutionCallback<E> {
            public:
                PipeliningExecutionCallback(const boost::shared_ptr<Pipelining> &pipelining) : pipelining(pipelining) {}

                virtual void onResponse(const boost::shared_ptr<E> &response) {
                    pipelining->up();
                }

                virtual void onFailure(const boost::shared_ptr<exception::IException> &e) {
                    pipelining->down();
                }

            private:
                const boost::shared_ptr<Pipelining> pipelining;
            };

            Pipelining(int depth) : permits(util::Preconditions::checkPositive(depth, "depth must be positive")) {
            }

            // TODO: Change with lock-free implementation when atomic is integrated into the library
            void down() {
                util::LockGuard lockGuard(mutex);
                while (permits == 0) {
                    conditionVariable.wait(mutex);
                }
                --permits;
            }

            void up() {
                util::LockGuard lockGuard(mutex);
                if (permits == 0) {
                    conditionVariable.notify_all();
                }
                ++permits;
            }

            typedef std::vector<boost::shared_ptr<ICompletableFuture<E> > > FuturesVector;
            int permits;
            std::vector<boost::shared_ptr<ICompletableFuture<E> > > futures;
            util::ConditionVariable conditionVariable;
            util::Mutex mutex;
        };
    }
}

#endif //HAZELCAST_CLIENT_PIPELINING_H_
