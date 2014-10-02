//
// Created by sancar koyunlu on 01/10/14.
//

#include "hazelcast/client/proxy/TransactionalQueueImpl.h"
#include "hazelcast/client/queue/TxnOfferRequest.h"
#include "hazelcast/client/queue/TxnPollRequest.h"
#include "hazelcast/client/queue/TxnSizeRequest.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalQueueImpl::TransactionalQueueImpl(const std::string& name, txn::TransactionProxy *transactionProxy)
            : TransactionalObject("hz:impl:queueService", name, transactionProxy) {

            }

            bool TransactionalQueueImpl::offer(const serialization::pimpl::Data& e, long timeoutInMillis) {
                queue::TxnOfferRequest *request = new queue::TxnOfferRequest(getName(), timeoutInMillis, e);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            serialization::pimpl::Data TransactionalQueueImpl::poll(long timeoutInMillis) {
                queue::TxnPollRequest *request = new queue::TxnPollRequest(getName(), timeoutInMillis);
                return invoke(request);
            }

            int TransactionalQueueImpl::size() {
                queue::TxnSizeRequest *request = new queue::TxnSizeRequest(getName());
                boost::shared_ptr<int> result = toObject<int>(invoke(request));
                return *result;
            }
        }
    }
}