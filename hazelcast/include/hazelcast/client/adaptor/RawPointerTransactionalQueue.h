/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by ihsan demir on 24/3/16.

#pragma once
#include "hazelcast/client/TransactionalQueue.h"

namespace hazelcast {
    namespace client {
        namespace adaptor {
            /**
            * Transactional implementation of IQueue.
            *
            * @see IQueue
            * @param <T> element type
            */
            template<typename T>
            class RawPointerTransactionalQueue {
            public:
                RawPointerTransactionalQueue(TransactionalQueue<T> &q) : queue(q), serializationService(
                        q.context->getSerializationService()) {
                }

                /**
                * Transactional implementation of IQueue::offer(const T &e)
                *
                * @see IQueue::offer(const T &e)
                */
                bool offer(const T &e) {
                    return queue.offer(e);
                }

                /**
                * Transactional implementation of IQueue::offer(const T &e, long timeoutInMillis)
                *
                * @see IQueue::offer(const T &e, long timeoutInMillis)
                */
                bool offer(const T &e, long timeoutInMillis) {
                    return queue.offer(e, timeoutInMillis);
                }

                /**
                * Transactional implementation of IQueue::poll()
                *
                * @see IQueue::poll()
                */
                std::unique_ptr<T> poll() {
                    return poll(0);
                }

                /**
                * Transactional implementation of IQueue::poll(long timeoutInMillis)
                *
                * @see IQueue::poll(long timeoutInMillis)
                */
                std::unique_ptr<T> poll(long timeoutInMillis) {
                    return serializationService.toObject<T>(queue.pollData(timeoutInMillis).get());
                }

                /**
                * Transactional implementation of IQueue::size()
                *
                * @see IQueue::size()
                */
                int size() {
                    return queue.size();
                }

            private:
                TransactionalQueue<T> &queue;
                serialization::pimpl::SerializationService &serializationService;
            };
        }
    }
}


