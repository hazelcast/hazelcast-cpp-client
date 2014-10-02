//
// Created by sancar koyunlu on 01/10/14.
//

#include "hazelcast/client/proxy/TransactionalSetImpl.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/collection/TxnSetAddRequest.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/collection/TxnSetRemoveRequest.h"
#include "hazelcast/client/collection/TxnSetSizeRequest.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalSetImpl::TransactionalSetImpl(const std::string& name, txn::TransactionProxy *transactionProxy)
            : TransactionalObject("hz:impl:setService", name, transactionProxy) {

            }

            bool TransactionalSetImpl::add(const serialization::pimpl::Data& e) {
                collection::TxnSetAddRequest *request = new collection::TxnSetAddRequest(getName(), e);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            bool TransactionalSetImpl::remove(const serialization::pimpl::Data& e) {
                collection::TxnSetRemoveRequest *request = new collection::TxnSetRemoveRequest(getName(), e);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            int TransactionalSetImpl::size() {
                collection::TxnSetSizeRequest *request = new collection::TxnSetSizeRequest(getName());
                boost::shared_ptr<int> result = toObject<int>(invoke(request));
                return *result;
            }
        }
    }
}