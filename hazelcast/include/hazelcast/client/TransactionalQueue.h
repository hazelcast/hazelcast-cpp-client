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
#pragma once

#include "hazelcast/client/proxy/TransactionalQueueImpl.h"

namespace hazelcast {
    namespace client {
        /**
        * Transactional implementation of IQueue.
        *
        * @see IQueue
        */
        class HAZELCAST_API TransactionalQueue : public proxy::TransactionalQueueImpl {
            friend class TransactionContext;
        public:
            /**
            * Transactional implementation of IQueue::offer(const E &e)
            *
            * @see IQueue::offer(const E &e)
            */
            template<typename E>
            boost::future<bool> offer(const E &e) {
                return offer(e, std::chrono::milliseconds::zero());
            }

            /**
            * Transactional implementation of IQueue::offer(const E &e, std::chrono::milliseconds timeout)
            *
            * @see IQueue::offer(const E &e, std::chrono::milliseconds timeout)
            */
            template<typename E>
            boost::future<bool> offer(const E &e, std::chrono::milliseconds timeout) {
                return proxy::TransactionalQueueImpl::offer(to_data(e), timeout);
            }

            /**
            * Transactional implementation of IQueue::poll()
            *
            * @see IQueue::poll()
            */
            template<typename E>
            boost::future<boost::optional<E>> poll() {
                return poll<E>(std::chrono::milliseconds::zero());
            }

            /**
            * Transactional implementation of IQueue::poll(std::chrono::milliseconds timeout)
            *
            * @see IQueue::poll(std::chrono::milliseconds timeout)
            */
            template<typename E>
            boost::future<boost::optional<E>> poll(std::chrono::milliseconds timeout) {
                return to_object<E>(proxy::TransactionalQueueImpl::poll_data(timeout));
            }

        private:
            TransactionalQueue(const std::string &name, txn::TransactionProxy &transaction_proxy)
                    : proxy::TransactionalQueueImpl(name, transaction_proxy) {

            }
        };
    }
}

