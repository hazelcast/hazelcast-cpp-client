//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionalQueue
#define HAZELCAST_TransactionalQueue

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/queue/TxnOfferRequest.h"
#include "hazelcast/client/queue/TxnPollRequest.h"
#include "hazelcast/client/queue/TxnSizeRequest.h"

namespace hazelcast {
    namespace client {
        /**
         * Transactional implementation of IQueue.
         *
         * @see IQueue
         * @param <E> element type
         */
        template <typename E>
        class HAZELCAST_API TransactionalQueue : public proxy::TransactionalObject {
            friend class TransactionContext;

        public:
            /**
             * Transactional implementation of IQueue::offer(const E &e)
             *
             * @see IQueue::offer(const E &e)
             */
            bool offer(const E &e) {
                return offer(e, 0);
            };

            /**
             * Transactional implementation of IQueue::offer(const E &e, long timeoutInMillis)
             *
             * @see IQueue::offer(const E &e, long timeoutInMillis)
             */
            bool offer(const E &e, long timeoutInMillis) {
                serialization::Data data = toData(e);
                queue::TxnOfferRequest *request = new queue::TxnOfferRequest(getName(), timeoutInMillis, data);
                return *(invoke<bool>(request));
            };

            /**
             * Transactional implementation of IQueue::poll()
             *
             * @see IQueue::poll()
             */
            boost::shared_ptr<E> poll() {
                return poll(0);
            };

            /**
             * Transactional implementation of IQueue::poll(long timeoutInMillis)
             *
             * @see IQueue::poll(long timeoutInMillis)
             */
            boost::shared_ptr<E> poll(long timeoutInMillis) {
                queue::TxnPollRequest *request = new queue::TxnPollRequest(getName(), timeoutInMillis);
                return invoke<E>(request);
            };

            /**
             * Transactional implementation of IQueue::size()
             *
             * @see IQueue::size()
             */
            int size() {
                queue::TxnSizeRequest *request = new queue::TxnSizeRequest(getName());
                boost::shared_ptr<int> s = invoke<int>(request);
                return *s;
            }

        private:
            TransactionalQueue(const std::string &name, txn::TransactionProxy *transactionProxy)
            :TransactionalObject("hz:impl:queueService", name, transactionProxy) {

            }


            void onDestroy() {
            }

        };

    }
}

#endif //HAZELCAST_TransactionalQueue
