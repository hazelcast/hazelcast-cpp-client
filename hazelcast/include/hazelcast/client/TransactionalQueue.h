//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionalQueue
#define HAZELCAST_TransactionalQueue

#include "hazelcast/client/proxy/TransactionalQueueImpl.h"

namespace hazelcast {
    namespace client {
        /**
        * Transactional implementation of IQueue.
        *
        * @see IQueue
        * @param <E> element type
        */
        template<typename E>
        class HAZELCAST_API TransactionalQueue : public proxy::TransactionalQueueImpl {
            friend class TransactionContext;

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
                return proxy::TransactionalQueueImpl::offer(toData(e), timeoutInMillis);
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
                return toObject<E>(proxy::TransactionalQueueImpl::poll(timeoutInMillis));
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

