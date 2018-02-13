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
//
// Created by sancar koyunlu on 8/5/13.

#ifndef HAZELCAST_TransactionalQueue
#define HAZELCAST_TransactionalQueue

#include "hazelcast/client/proxy/TransactionalQueueImpl.h"

namespace hazelcast {
    namespace client {
        namespace adaptor {
            template<typename E>
            class RawPointerTransactionalQueue;
        }

        /**
        * Transactional implementation of IQueue.
        *
        * @see IQueue
        * @param <E> element type
        */
        template<typename E>
        class TransactionalQueue : public proxy::TransactionalQueueImpl {
            friend class TransactionContext;
            friend class adaptor::RawPointerTransactionalQueue<E>;

        public:
            /**
            * Transactional implementation of IQueue::offer(const E &e)
            *
            * @see IQueue::offer(const E &e)
            */
            bool offer(const E& e) {
                return offer(e, 0);
            }

            /**
            * Transactional implementation of IQueue::offer(const E &e, long timeoutInMillis)
            *
            * @see IQueue::offer(const E &e, long timeoutInMillis)
            */
            bool offer(const E& e, long timeoutInMillis) {
                return proxy::TransactionalQueueImpl::offer(toData(&e), timeoutInMillis);
            }

            /**
            * Transactional implementation of IQueue::poll()
            *
            * @see IQueue::poll()
            */
            boost::shared_ptr<E> poll() {
                return poll(0);
            }

            /**
            * Transactional implementation of IQueue::poll(long timeoutInMillis)
            *
            * @see IQueue::poll(long timeoutInMillis)
            */
            boost::shared_ptr<E> poll(long timeoutInMillis) {
                return boost::shared_ptr<E>(toObject<E>(proxy::TransactionalQueueImpl::pollData(timeoutInMillis)));
            }

            /**
            * Transactional implementation of IQueue::size()
            *
            * @see IQueue::size()
            */
            int size() {
                return proxy::TransactionalQueueImpl::size();
            }

        private:
            TransactionalQueue(const std::string& name, txn::TransactionProxy *transactionProxy)
            : proxy::TransactionalQueueImpl(name, transactionProxy) {

            }
        };
    }
}

#endif //HAZELCAST_TransactionalQueue

