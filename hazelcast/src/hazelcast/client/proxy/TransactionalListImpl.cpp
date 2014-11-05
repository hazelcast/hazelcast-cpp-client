//
// Created by sancar koyunlu on 01/10/14.
//

#include "hazelcast/client/proxy/TransactionalListImpl.h"
#include "hazelcast/client/collection/TxnListAddRequest.h"
#include "hazelcast/client/collection/TxnListRemoveRequest.h"
#include "hazelcast/client/collection/TxnListSizeRequest.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalListImpl::TransactionalListImpl(const std::string& objectName, txn::TransactionProxy *context)
            : TransactionalObject("hz:impl:listService", objectName, context) {
            }

            bool TransactionalListImpl::add(const serialization::pimpl::Data& e) {
                collection::TxnListAddRequest *request = new collection::TxnListAddRequest(getName(), e);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            bool TransactionalListImpl::remove(const serialization::pimpl::Data& e) {
                collection::TxnListRemoveRequest *request = new collection::TxnListRemoveRequest(getName(), e);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            int TransactionalListImpl::size() {
                collection::TxnListSizeRequest *request = new collection::TxnListSizeRequest(getName());
                boost::shared_ptr<int> result = toObject<int>(invoke(request));
                return *result;
            }
        }
    }
}

